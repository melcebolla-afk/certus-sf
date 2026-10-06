#ifndef CERTUS_SEARCH_H_INCLUDED
#define CERTUS_SEARCH_H_INCLUDED

#include "certus_eval.h"

#include "../types.h"

#include <algorithm>
#include <functional>
#include <optional>
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

// When root hits TB/mate/theory, UCI PV score must match hard evidence (not child NNUE).
void                 clear_root_hard_score();
void                 set_root_hard_score(Value v);
std::optional<Value> root_hard_score();

// Returns true when root forces bestmove (Strict only: consensus / ICCF singleton).
bool prepare_root_search(const Position& rootPos, const Tablebases::Config& tbConfig,
                         Search::SearchManager& manager, std::function<Value()> getNnueEval,
                         int displayDepth, bool showWdl);
void finish_search_evidence();

// FEAT-0004/0006/0007: preferred marked/frequent once per node (not once per candidate).
struct SearchMoveFilter {
    bool              restrict_moves  = false;  // Strict + catalog hit
    bool              boost_preferred = false;  // Mixed or Strict with preferred set
    bool              interior_depth  = false;  // Strict / Mixed High|Max: ext in tree
    bool              zero_lmr        = false;  // Mixed Max: no LMR on preferred
    Depth             lmr_relief      = 1024;   // LMR units subtracted on preferred
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

// FEAT-0007 Max: MovePicker order bonus without mutating history tables.
void set_move_picker_boost(const SearchMoveFilter* filt);
int  move_picker_preferred_bonus(Move m);

// reductionUnits are SF LMR units (/1024). Preferred: less LMR (Mixed+Strict);
// Mixed Max zeros LMR. Mixed High|Max / Strict interiors also get a mild extension.
void apply_style_depth_bias(const SearchMoveFilter& filt, Move move, bool rootNode, Depth& extension,
                            Depth& reductionUnits);

}  // namespace Certus
}  // namespace Stockfish

#endif
