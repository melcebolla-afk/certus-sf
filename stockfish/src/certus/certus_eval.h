#ifndef CERTUS_EVAL_H_INCLUDED
#define CERTUS_EVAL_H_INCLUDED

#include "../evaluate.h"
#include "../position.h"
#include "../syzygy/tbprobe.h"
#include "../types.h"
#include "../evidence/evidence_manager.h"

namespace Stockfish::Certus {

void bind_evidence(const Evidence::Manager& manager);
void bind_tb_config(const Tablebases::Config& config);

Value evaluate(const Position& pos, const Eval::NNUE::Networks& networks,
               Eval::NNUE::AccumulatorStack& accumulators, Eval::NNUE::AccumulatorCaches& caches,
               int optimism);

}  // namespace Stockfish::Certus

#endif
