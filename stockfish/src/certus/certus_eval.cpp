#include "certus_eval.h"

#include "../evaluate.h"
#include "../evidence/resolver.h"

#include "certus_search.h"

namespace Stockfish::Certus {

namespace {

const Evidence::Manager* g_manager            = nullptr;
thread_local Tablebases::Config tls_tb_config{};
thread_local EvalNeed           tls_eval_need = EvalNeed::Full;

Evidence::EvalContext make_context() {
    Evidence::EvalContext ctx;
    ctx.manager        = g_manager;
    ctx.tb_cardinality = Tablebases::MaxCardinality;
    ctx.syzygy_loaded  = Tablebases::MaxCardinality > 0;
    if (tls_tb_config.cardinality > 0 && tls_tb_config.cardinality < ctx.tb_cardinality)
        ctx.tb_cardinality = tls_tb_config.cardinality;
    return ctx;
}

std::pair<bool, bool> layer_flags(EvalNeed need) {
    switch (need)
    {
    case EvalNeed::HardOnly:
        return {true, false};
    case EvalNeed::SoftOnly:
        return {false, true};
    default:
        return {true, true};
    }
}

}  // namespace

void bind_evidence(const Evidence::Manager& manager) { g_manager = &manager; }

void bind_tb_config(const Tablebases::Config& config) { tls_tb_config = config; }

const Evidence::Manager* evidence_manager() { return g_manager; }

void set_eval_need(EvalNeed need) { tls_eval_need = need; }

EvalNeed current_eval_need() { return tls_eval_need; }

void set_track_hits(bool track) { Certus::set_track_hits_impl(track); }

bool tracking_hits() { return Certus::tracking_hits_impl(); }

Evidence::EvalResult evaluate_layers(const Position& pos, EvalNeed need) {
    if (!g_manager)
        return Evidence::EvalResult{};

    Position&                   mutable_pos = const_cast<Position&>(pos);
    const Evidence::EvalContext ctx         = make_context();
    const auto [hard, soft]                 = layer_flags(need);
    Evidence::EvalResult ev = Evidence::evaluate_layers(mutable_pos, ctx, hard, soft);
    return ev;
}

Value evaluate(const Position& pos, const Eval::NNUE::Networks& networks,
               Eval::NNUE::AccumulatorStack& accumulators, Eval::NNUE::AccumulatorCaches& caches,
               int optimism) {
    if (!g_manager)
        return Eval::evaluate(networks, pos, accumulators, caches, optimism);

    const EvalNeed need = tls_eval_need;

    // SoftOnly: skip evidence resolver — Stockfish NNUE (interior quiet midgame).
    if (need == EvalNeed::SoftOnly)
        return Eval::evaluate(networks, pos, accumulators, caches, optimism);

    const Evidence::EvalResult ev = evaluate_layers(pos, need);

    if (tracking_hits())
        record_evidence_hit(ev.evidence_class);

    if (ev.evidence_class == Evidence::EvidenceClass::Inference)
        return Eval::evaluate(networks, pos, accumulators, caches, optimism);

    return ev.to_value();
}

}  // namespace Stockfish::Certus
