#ifndef EVIDENCE_UTILITY_H_INCLUDED
#define EVIDENCE_UTILITY_H_INCLUDED

#include <algorithm>

#include "../syzygy/tbprobe.h"
#include "../types.h"
#include "consensus_store.h"

namespace Stockfish::Evidence {

constexpr int TB_WIN_CP  = 20000;
constexpr int TB_DRAW_CP = 0;
constexpr int TB_LOSS_CP = -20000;
constexpr int MATE_SCORE = 30000;

inline int wdl_utility(Wdl w, int win_divisor) {
    switch (w)
    {
    case Wdl::Win:
        return TB_WIN_CP / win_divisor;
    case Wdl::Draw:
        return TB_DRAW_CP;
    case Wdl::Loss:
        return TB_LOSS_CP / win_divisor;
    }
    return 0;
}

inline int tb_wdl_utility(Tablebases::WDLScore wdl) {
    switch (wdl)
    {
    case Tablebases::WDLWin:
        return TB_WIN_CP;
    case Tablebases::WDLCursedWin:
        return TB_WIN_CP / 2;
    case Tablebases::WDLDraw:
    case Tablebases::WDLBlessedLoss:
        return TB_DRAW_CP;
    case Tablebases::WDLLoss:
        return TB_LOSS_CP;
    }
    return 0;
}

inline int mate_utility(bool stm_wins, int plies) {
    const int base = MATE_SCORE - plies;
    return stm_wins ? base : -base;
}

inline Value mate_to_value(bool stm_wins, int plies) { return stm_wins ? mate_in(plies) : mated_in(plies); }

inline Value utility_to_value(int utility) {
    return std::clamp(utility, VALUE_TB_LOSS_IN_MAX_PLY + 1, VALUE_TB_WIN_IN_MAX_PLY - 1);
}

}  // namespace Stockfish::Evidence

#endif
