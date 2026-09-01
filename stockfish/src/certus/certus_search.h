#ifndef CERTUS_SEARCH_H_INCLUDED
#define CERTUS_SEARCH_H_INCLUDED

#include "certus_eval.h"

namespace Stockfish {

class Position;

namespace Search {
struct Stack;
class SearchManager;
class Worker;
}

namespace Certus {

void reset_search_evidence();
EvalNeed pick_eval_need(bool rootNode, bool pvNode, const Position& pos, const Search::Stack* ss);
void record_evidence_hit(Evidence::EvidenceClass c);

// Returns true when root STRONG_CONSENSUS forces bestmove (search skipped).
bool prepare_root_search(const Position& rootPos, const Tablebases::Config& tbConfig,
                         Search::SearchManager& manager);
void finish_search_evidence();

// FEAT-0002: marked-only movegen in main search (not qsearch).
std::vector<Move> consensus_marked_legal_moves(const Position& pos);
bool              allow_search_move(const Position& pos, Move move, bool inCheck, int pvIdx);

}  // namespace Certus
}  // namespace Stockfish

#endif
