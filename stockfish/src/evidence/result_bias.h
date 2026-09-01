#ifndef EVIDENCE_RESULT_BIAS_H_INCLUDED
#define EVIDENCE_RESULT_BIAS_H_INCLUDED

#include "evidence_manager.h"
#include "resolver.h"

namespace Stockfish::Evidence {

class ResultBias {
   public:
    static int  remap(ResultBiasMode mode, int utility);
    static void apply(ResultBiasMode mode, EvalResult& ev);
};

}  // namespace Stockfish::Evidence

#endif
