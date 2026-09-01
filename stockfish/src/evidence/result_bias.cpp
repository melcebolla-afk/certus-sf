#include "result_bias.h"

namespace Stockfish::Evidence {

int ResultBias::remap(ResultBiasMode mode, int utility) {
    switch (mode)
    {
    case ResultBiasMode::Neutral:
        return utility;
    case ResultBiasMode::NeedWin:
        if (utility == 0)
            return -80;
        if (utility > 0 && utility < 120)
            return utility - 40;
        if (utility < 0 && utility > -200)
            return utility - 25;
        return utility;
    case ResultBiasMode::PreferDraw:
        if (utility == 0)
            return 50;
        if (utility > -250 && utility < 250)
            return utility / 2 + (utility > 0 ? 15 : -15);
        return utility / 2;
    }
    return utility;
}

void ResultBias::apply(ResultBiasMode mode, EvalResult& ev) {
    ev.utility = remap(mode, ev.utility);
}

}  // namespace Stockfish::Evidence
