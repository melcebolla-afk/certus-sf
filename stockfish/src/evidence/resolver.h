#ifndef EVIDENCE_RESOLVER_H_INCLUDED
#define EVIDENCE_RESOLVER_H_INCLUDED

#include <optional>
#include <string>
#include <vector>

#include "../types.h"
#include "evidence_manager.h"

namespace Stockfish::Evidence {

enum class EvidenceClass {
    ProvenTb,
    ProvenMate,
    Theoretical,
    StrongConsensus,
    EmpiricalIccf,
    Inference,
};

struct EvalResult {
    std::optional<std::tuple<std::uint16_t, std::uint16_t, std::uint16_t>> wdl;
    int                  utility     = 0;
    std::optional<int>   mate_plies;
    bool                 stm_wins_mate = false;
    EvidenceClass        evidence_class = EvidenceClass::Inference;
    float                confidence  = 0.0f;
    std::string          layer_version;
    std::vector<std::string> sources;

    bool hard_evidence() const;
    Value to_value() const;
};

struct EvalContext {
    const Manager* manager     = nullptr;
    int            tb_cardinality = 0;
    bool           syzygy_loaded  = false;
};

const char* evidence_class_name(EvidenceClass c);
EvidenceClass evidence_class_from_name(const std::string& s);

EvalResult evaluate_layers(Position& pos, const EvalContext& ctx, bool hard_layers,
                           bool soft_layers);

EvalResult evaluate_full(Position& pos, const EvalContext& ctx);
EvalResult evaluate_hard_only(Position& pos, const EvalContext& ctx);

}  // namespace Stockfish::Evidence

#endif
