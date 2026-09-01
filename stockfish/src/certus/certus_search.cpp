#include "certus_search.h"

#include "../evidence/resolver.h"
#include "../misc.h"
#include "../movegen.h"
#include "../score.h"
#include "../search.h"
#include "../syzygy/tbprobe.h"
#include "../uci.h"

#include <algorithm>
#include <atomic>
#include <sstream>

namespace Stockfish::Certus {

namespace {

struct EvidenceHits {
    std::atomic<std::uint64_t> tb{0};
    std::atomic<std::uint64_t> mate{0};
    std::atomic<std::uint64_t> theory{0};
    std::atomic<std::uint64_t> consensus{0};
    std::atomic<std::uint64_t> iccf{0};
};

EvidenceHits g_hits;
bool          g_track_hits = false;

bool show_root_evidence(const Evidence::Manager& mgr) {
    return mgr.evidence_info() != Evidence::EvidenceInfoMode::Off;
}

bool track_search_hits(const Evidence::Manager& mgr) {
    return mgr.evidence_info() == Evidence::EvidenceInfoMode::All;
}

void print_info(const std::string& line) {
    sync_cout << line << sync_endl;
}

std::string format_root_evidence(const Evidence::EvalResult& ev) {
    std::ostringstream ss;
    ss << "info string evidence=" << Evidence::evidence_class_name(ev.evidence_class)
       << " confidence=" << ev.confidence;
    if (ev.evidence_class == Evidence::EvidenceClass::Inference)
    {
        const char* source = ev.sources.empty() ? "-" : ev.sources[0].c_str();
        ss << " source=" << source;
    }
    ss << " version=" << (ev.layer_version.empty() ? "-" : ev.layer_version);
    return ss.str();
}

Evidence::EvalContext make_root_context(const Evidence::Manager& mgr,
                                        const Tablebases::Config& tbConfig) {
    Evidence::EvalContext ctx;
    ctx.manager        = &mgr;
    ctx.tb_cardinality = Tablebases::MaxCardinality;
    ctx.syzygy_loaded  = Tablebases::MaxCardinality > 0;
    if (tbConfig.cardinality > 0 && tbConfig.cardinality < ctx.tb_cardinality)
        ctx.tb_cardinality = tbConfig.cardinality;
    return ctx;
}

std::vector<Move> legal_marked_moves(const Position& pos, const std::vector<std::string>& marked) {
    MoveList<LEGAL> legal(pos);
    std::vector<Move> out;
    for (const auto& uci : marked)
    {
        const Move m = UCIEngine::to_move(pos, uci);
        if (m == Move::none())
            continue;
        if (std::find(legal.begin(), legal.end(), m) == legal.end())
            continue;
        if (std::find(out.begin(), out.end(), m) == out.end())
            out.push_back(m);
    }
    return out;
}

}  // namespace

void set_track_hits_impl(bool track) { g_track_hits = track; }

bool tracking_hits_impl() { return g_track_hits; }

void reset_search_evidence() {
    g_hits.tb.store(0);
    g_hits.mate.store(0);
    g_hits.theory.store(0);
    g_hits.consensus.store(0);
    g_hits.iccf.store(0);
    g_track_hits = false;
    set_eval_need(EvalNeed::Full);
}

void record_evidence_hit(Evidence::EvidenceClass c) {
    switch (c)
    {
    case Evidence::EvidenceClass::ProvenTb:
        g_hits.tb.fetch_add(1, std::memory_order_relaxed);
        break;
    case Evidence::EvidenceClass::ProvenMate:
        g_hits.mate.fetch_add(1, std::memory_order_relaxed);
        break;
    case Evidence::EvidenceClass::Theoretical:
        g_hits.theory.fetch_add(1, std::memory_order_relaxed);
        break;
    case Evidence::EvidenceClass::StrongConsensus:
        g_hits.consensus.fetch_add(1, std::memory_order_relaxed);
        break;
    case Evidence::EvidenceClass::EmpiricalIccf:
        g_hits.iccf.fetch_add(1, std::memory_order_relaxed);
        break;
    default:
        break;
    }
}

EvalNeed pick_eval_need(bool rootNode, bool pvNode, const Position& pos, const Search::Stack* ss) {
    if (rootNode || pvNode)
        return EvalNeed::Full;
    // Interior quiet midgame: NNUE only (skip catalog probes — SF-native eval, better nps).
    if (ss && !ss->inCheck && pos.count<ALL_PIECES>() > 6)
        return EvalNeed::SoftOnly;
    return EvalNeed::Full;
}

bool prepare_root_search(const Position& rootPos, const Tablebases::Config& tbConfig,
                         Search::SearchManager& manager, Value nnueEval) {
    const Evidence::Manager* mgr = evidence_manager();
    if (!mgr)
        return false;

    reset_search_evidence();
    set_track_hits_impl(track_search_hits(*mgr));

    Evidence::EvalResult root_ev =
      Evidence::evaluate_full(const_cast<Position&>(rootPos), make_root_context(*mgr, tbConfig));

    if (show_root_evidence(*mgr))
        print_info(format_root_evidence(root_ev));

    if (root_ev.evidence_class != Evidence::EvidenceClass::StrongConsensus)
        return false;

    const Evidence::ConsensusEntry* entry = mgr->probe_consensus(rootPos);
    if (!entry)
        return false;

    const std::vector<Move> marked = legal_marked_moves(rootPos, entry->marked_moves);
    if (marked.empty())
    {
        if (show_root_evidence(*mgr) && !entry->marked_moves.empty())
            print_info("info string marked-miss (no legal marked; search fallback)");
        return false;
    }

    if (show_root_evidence(*mgr))
    {
        std::ostringstream ss;
        ss << "info string marked=";
        for (std::size_t i = 0; i < marked.size(); ++i)
        {
            if (i)
                ss << ',';
            ss << UCIEngine::move(marked[i], rootPos.is_chess960());
        }
        print_info(ss.str());
    }

    const std::string bestmove = UCIEngine::move(marked[0], rootPos.is_chess960());

    Search::InfoFull info{};
    info.depth    = 1;
    info.selDepth = 1;
    info.multiPV  = 1;
    info.score    = Score(nnueEval, rootPos);
    info.timeMs   = 0;
    info.nodes    = 0;
    info.nps      = 0;
    info.tbHits   = 0;
    info.hashfull = 0;
    info.pv       = bestmove;
    manager.updates.onUpdateFull(info);

    manager.updates.onBestmove(bestmove, "");
    return true;
}

void finish_search_evidence() {
    if (!g_track_hits)
        return;

    std::ostringstream parts;
    auto append = [&](const char* label, std::uint64_t n) {
        if (n > 0)
        {
            if (!parts.str().empty())
                parts << ' ';
            parts << label << '=' << n;
        }
    };

    append("PROVEN_TB", g_hits.tb.load());
    append("PROVEN_MATE", g_hits.mate.load());
    append("THEORETICAL", g_hits.theory.load());
    append("STRONG_CONSENSUS", g_hits.consensus.load());
    append("EMPIRICAL_ICCF", g_hits.iccf.load());

    if (!parts.str().empty())
        print_info("info string evidence_hits " + parts.str());
}

std::vector<Move> consensus_marked_legal_moves(const Position& pos) {
    const Evidence::Manager* mgr = evidence_manager();
    if (!mgr || !mgr->consensus().ready())
        return {};

    const Evidence::ConsensusEntry* entry = mgr->probe_consensus(pos);
    if (!entry || entry->marked_moves.empty())
        return {};

    return legal_marked_moves(pos, entry->marked_moves);
}

bool allow_search_move(const Position& pos, Move move, bool inCheck, int pvIdx) {
    const Evidence::Manager* mgr = evidence_manager();
    if (!mgr || mgr->consensus_search() != Evidence::ConsensusSearchMode::MarkedOnly)
        return true;
    if (inCheck || pvIdx > 0)
        return true;

    const std::vector<Move> marked = consensus_marked_legal_moves(pos);
    if (marked.empty())
        return true;

    return std::find(marked.begin(), marked.end(), move) != marked.end();
}

}  // namespace Stockfish::Certus
