#include "resolver.h"

#include "../syzygy/tbprobe.h"
#include "utility.h"

#include <algorithm>
#include <cctype>

namespace Stockfish::Evidence {

namespace {

std::tuple<std::uint16_t, std::uint16_t, std::uint16_t> wdl_triplet(Wdl w) {
    switch (w)
    {
    case Wdl::Win:
        return {1000, 0, 0};
    case Wdl::Draw:
        return {0, 1000, 0};
    case Wdl::Loss:
        return {0, 0, 1000};
    }
    return {0, 1000, 0};
}

EvalResult mate_eval_indexed(const MateEntry& e, const std::string& version) {
    EvalResult r;
    r.wdl           = wdl_triplet(e.stm_wins ? Wdl::Win : Wdl::Loss);
    r.utility       = mate_utility(e.stm_wins, e.plies);
    r.mate_plies    = e.plies;
    r.stm_wins_mate = e.stm_wins;
    r.evidence_class = EvidenceClass::ProvenMate;
    r.confidence    = 1.0f;
    r.layer_version = version;
    r.sources       = {"mate-index"};
    return r;
}

EvalResult inference_zero() {
    EvalResult r;
    r.evidence_class = EvidenceClass::Inference;
    r.confidence     = 0.0f;
    return r;
}

bool probe_tb(Position& pos, const EvalContext& ctx, EvalResult& out) {
    if (!ctx.syzygy_loaded || ctx.tb_cardinality <= 0)
        return false;

    const int pieces = pos.count<ALL_PIECES>();
    if (pieces > ctx.tb_cardinality)
        return false;

    Tablebases::ProbeState err;
    const Tablebases::WDLScore wdl = Tablebases::probe_wdl(pos, &err);
    if (err == Tablebases::ProbeState::FAIL)
        return false;

    out.wdl            = std::nullopt;
    out.utility        = tb_wdl_utility(wdl);
    out.evidence_class = EvidenceClass::ProvenTb;
    out.confidence     = 1.0f;
    out.layer_version  = "syzygy-max" + std::to_string(ctx.tb_cardinality);
    out.sources        = {"syzygy-wdl"};
    return true;
}

EvalResult layer_theoretical(const Position& pos, const TheoreticalStore& store) {
    bool hit = false;
    const Wdl w = store.probe(pos, &hit);
    if (!hit)
        return inference_zero();

    EvalResult r;
    r.wdl            = wdl_triplet(w);
    r.utility        = wdl_utility(w, 2);
    r.evidence_class = EvidenceClass::Theoretical;
    r.confidence     = 0.95f;
    r.layer_version  = store.content_version();
    r.sources        = {"theoretical"};
    return r;
}

EvalResult layer_consensus(const Position& pos, const ConsensusStore& store) {
    const ConsensusEntry* e = store.probe(pos);
    if (!e)
        return inference_zero();

    EvalResult r;
    r.wdl            = wdl_triplet(e->wdl);
    r.utility        = wdl_utility(e->wdl, 2);
    r.evidence_class = EvidenceClass::StrongConsensus;
    r.confidence     = e->confidence;
    r.layer_version  = store.content_version();
    r.sources        = e->sources;
    if (r.sources.empty())
        r.sources.push_back("strong-consensus");
    return r;
}

EvalResult layer_iccf(const Position& pos, const IccfStore& store) {
    const IccfEntry* e = store.probe(pos);
    if (!e)
        return inference_zero();

    EvalResult r;
    r.wdl            = wdl_triplet(e->wdl);
    r.utility        = wdl_utility(e->wdl, 3);
    r.evidence_class = EvidenceClass::EmpiricalIccf;
    r.confidence     = e->confidence;
    r.layer_version  = store.content_version();
    r.sources        = e->sources;
    if (r.sources.empty())
        r.sources.push_back("iccf");
    return r;
}

}  // namespace

bool EvalResult::hard_evidence() const {
    switch (evidence_class)
    {
    case EvidenceClass::ProvenTb:
    case EvidenceClass::ProvenMate:
    case EvidenceClass::Theoretical:
    case EvidenceClass::StrongConsensus:
        return true;
    default:
        return false;
    }
}

Value EvalResult::to_value() const {
    if (mate_plies)
        return mate_to_value(stm_wins_mate, *mate_plies);
    if (evidence_class == EvidenceClass::ProvenTb)
    {
        if (utility >= TB_WIN_CP / 2)
            return VALUE_TB_WIN_IN_MAX_PLY;
        if (utility <= TB_LOSS_CP / 2)
            return VALUE_TB_LOSS_IN_MAX_PLY;
        return VALUE_DRAW;
    }
    return utility_to_value(utility);
}

const char* evidence_class_name(EvidenceClass c) {
    switch (c)
    {
    case EvidenceClass::ProvenTb:
        return "PROVEN_TB";
    case EvidenceClass::ProvenMate:
        return "PROVEN_MATE";
    case EvidenceClass::Theoretical:
        return "THEORETICAL";
    case EvidenceClass::StrongConsensus:
        return "STRONG_CONSENSUS";
    case EvidenceClass::EmpiricalIccf:
        return "EMPIRICAL_ICCF";
    case EvidenceClass::Inference:
        return "INFERENCE";
    }
    return "INFERENCE";
}

EvidenceClass evidence_class_from_name(const std::string& s) {
    std::string t;
    t.reserve(s.size());
    for (char c : s)
        t += char(std::toupper(static_cast<unsigned char>(c)));
    if (t == "PROVEN_TB")
        return EvidenceClass::ProvenTb;
    if (t == "PROVEN_MATE")
        return EvidenceClass::ProvenMate;
    if (t == "THEORETICAL")
        return EvidenceClass::Theoretical;
    if (t == "STRONG_CONSENSUS")
        return EvidenceClass::StrongConsensus;
    if (t == "EMPIRICAL_ICCF")
        return EvidenceClass::EmpiricalIccf;
    return EvidenceClass::Inference;
}

EvalResult evaluate_layers(Position& pos, const EvalContext& ctx, bool hard_layers,
                           bool soft_layers) {
    if (!ctx.manager)
        return inference_zero();

    const Manager& mgr = *ctx.manager;

    if (hard_layers)
    {
        const bool thin_hard =
          !soft_layers && pos.count<ALL_PIECES>() > 6 && !pos.checkers();

        EvalResult tb_hit;
        if (probe_tb(pos, ctx, tb_hit))
            return tb_hit;

        if (thin_hard)
            return inference_zero();

        if (mgr.mate().ready())
        {
            const bool probe_index =
              soft_layers || pos.count<ALL_PIECES>() <= 6 || pos.checkers();
            if (probe_index)
            {
                if (const MateEntry* e = mgr.probe_mate(pos))
                    return mate_eval_indexed(*e, mgr.mate().content_version());
            }
        }

        if (mgr.theoretical().ready())
        {
            EvalResult th = layer_theoretical(pos, mgr.theoretical());
            if (th.evidence_class == EvidenceClass::Theoretical)
                return th;
        }

        if (mgr.consensus().ready())
        {
            EvalResult cons = layer_consensus(pos, mgr.consensus());
            if (cons.evidence_class == EvidenceClass::StrongConsensus)
                return cons;
        }

        if (!soft_layers)
            return inference_zero();
    }

    if (soft_layers)
    {
        if (!hard_layers)
            return inference_zero();

        if (mgr.iccf().ready())
        {
            EvalResult iccf = layer_iccf(pos, mgr.iccf());
            if (iccf.evidence_class == EvidenceClass::EmpiricalIccf)
                return iccf;
        }

        return inference_zero();
    }

    return inference_zero();
}

EvalResult evaluate_full(Position& pos, const EvalContext& ctx) {
    return evaluate_layers(pos, ctx, true, true);
}

EvalResult evaluate_hard_only(Position& pos, const EvalContext& ctx) {
    return evaluate_layers(pos, ctx, true, false);
}

}  // namespace Stockfish::Evidence
