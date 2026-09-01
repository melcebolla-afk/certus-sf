#include "certus_eval.h"

#include "../evaluate.h"
#include "../evidence/resolver.h"

namespace Stockfish::Certus {

namespace {

thread_local const Evidence::Manager* tls_manager = nullptr;
thread_local Tablebases::Config       tls_tb_config{};

Evidence::EvalContext make_context() {
    Evidence::EvalContext ctx;
    ctx.manager         = tls_manager;
    ctx.tb_cardinality  = Tablebases::MaxCardinality;
    ctx.syzygy_loaded   = Tablebases::MaxCardinality > 0;
    if (tls_tb_config.cardinality > 0 && tls_tb_config.cardinality < ctx.tb_cardinality)
        ctx.tb_cardinality = tls_tb_config.cardinality;
    return ctx;
}

}  // namespace

void bind_evidence(const Evidence::Manager& manager) { tls_manager = &manager; }

void bind_tb_config(const Tablebases::Config& config) { tls_tb_config = config; }

Value evaluate(const Position& pos, const Eval::NNUE::Networks& networks,
               Eval::NNUE::AccumulatorStack& accumulators, Eval::NNUE::AccumulatorCaches& caches,
               int optimism) {
    if (!tls_manager)
        return Eval::evaluate(networks, pos, accumulators, caches, optimism);

    Position& mutable_pos = const_cast<Position&>(pos);
    const Evidence::EvalContext ctx = make_context();
    const Evidence::EvalResult    ev  = Evidence::evaluate_full(mutable_pos, ctx);

    if (ev.evidence_class == Evidence::EvidenceClass::Inference)
        return Eval::evaluate(networks, pos, accumulators, caches, optimism);

    return ev.to_value();
}

}  // namespace Stockfish::Certus
