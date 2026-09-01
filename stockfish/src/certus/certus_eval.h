#ifndef CERTUS_EVAL_H_INCLUDED
#define CERTUS_EVAL_H_INCLUDED

#include "../evaluate.h"
#include "../position.h"
#include "../syzygy/tbprobe.h"
#include "../types.h"
#include "../evidence/evidence_manager.h"
#include "../evidence/resolver.h"

namespace Stockfish::Certus {

enum class EvalNeed { Full, HardOnly, SoftOnly };

void bind_evidence(const Evidence::Manager& manager);
void bind_tb_config(const Tablebases::Config& config);

const Evidence::Manager* evidence_manager();
void                     set_eval_need(EvalNeed need);
EvalNeed                 current_eval_need();
void                     set_track_hits(bool track);
bool                     tracking_hits();

void set_track_hits_impl(bool track);
bool tracking_hits_impl();

Value evaluate(const Position& pos, const Eval::NNUE::Networks& networks,
               Eval::NNUE::AccumulatorStack& accumulators, Eval::NNUE::AccumulatorCaches& caches,
               int optimism);

Evidence::EvalResult evaluate_layers(const Position& pos, EvalNeed need);

}  // namespace Stockfish::Certus

#endif
