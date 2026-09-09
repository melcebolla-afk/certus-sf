#ifndef CERTUS_SEARCH_H_INCLUDED
#define CERTUS_SEARCH_H_INCLUDED

#include "certus_eval.h"

#include "../types.h"

#include <algorithm>
#include <functional>
#include <vector>

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

// Returns true when root forces bestmove (Strict only: consensus / ICCF singleton).
bool prepare_root_search(const Position& rootPos, const Tablebases::Config& tbConfig,
                         Search::SearchManager& manager, std::function<Value()> getNnueEval,
                         int displayDepth, bool showWdl);
void finish_search_evidence();

// FEAT-0004: preferred marked/frequent once per node (not once per candidate).
struct SearchMoveFilter {
    bool              restrict_moves  = false;  // Strict + catalog hit
    bool              boost_preferred = false;  // Mixed or Strict with preferred set
    bool              interior_depth  = false;  // Strict: LMR/ext bias in tree
    std::vector<Move> preferred;

    bool allows(Move m) const {
        if (!restrict_moves)
            return true;
        return std::find(preferred.begin(), preferred.end(), m) != preferred.end();
    }

    bool is_preferred(Move m) const {
        if (!boost_preferred || preferred.empty())
            return false;
        return std::find(preferred.begin(), preferred.end(), m) != preferred.end();
    }
};

SearchMoveFilter  make_search_move_filter(const Position& pos, bool inCheck, int pvIdx);
std::vector<Move> consensus_marked_legal_moves(const Position& pos);
std::vector<Move> iccf_frequent_legal_moves(const Position& pos);
bool              allow_search_move(const Position& pos, Move move, bool inCheck, int pvIdx);

// reductionUnits are SF LMR units (/1024). Preferred: less LMR (Mixed+Strict);
// Strict interiors also get a mild extension.
void apply_style_depth_bias(const SearchMoveFilter& filt, Move move, bool rootNode, Depth& extension,
                            Depth& reductionUnits);

}  // namespace Certus
}  // namespace Stockfish

#endif
