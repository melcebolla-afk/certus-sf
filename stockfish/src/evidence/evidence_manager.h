#ifndef EVIDENCE_MANAGER_H_INCLUDED
#define EVIDENCE_MANAGER_H_INCLUDED

#include <functional>
#include <optional>
#include <string>

#include "../position.h"
#include "../ucioption.h"
#include "consensus_store.h"
#include "iccf_store.h"
#include "mate_store.h"
#include "theoretical_store.h"

namespace Stockfish::Evidence {

enum class EvidenceInfoMode { Off, Root, All };

enum class ConsensusSearchMode { Off, MarkedOnly };

enum class IccfSearchMode { Off, FreqOnly };

class Manager {
   public:
    Manager() = default;

    void register_options(OptionsMap& options, std::function<void()> on_reload);

    std::string reload_consensus(const std::string& path);
    std::string reload_iccf(const std::string& path);
    std::string reload_theoretical(const std::string& path);
    std::string reload_mate(const std::string& path);
    std::string reload_evidence_root(const std::string& path);

    const ConsensusStore&   consensus() const { return consensus_; }
    const IccfStore&        iccf() const { return iccf_; }
    const TheoreticalStore& theoretical() const { return theoretical_; }
    const MateStore&        mate() const { return mate_; }

    EvidenceInfoMode evidence_info() const { return evidence_info_; }
    ConsensusSearchMode consensus_search() const { return consensus_search_; }
    void                  set_consensus_search(ConsensusSearchMode mode) { consensus_search_ = mode; }
    IccfSearchMode        iccf_search() const { return iccf_search_; }
    void                  set_iccf_search(IccfSearchMode mode) { iccf_search_ = mode; }

    const ConsensusEntry* probe_consensus(const Position& pos) const {
        return consensus_.probe(pos);
    }
    const IccfEntry* probe_iccf(const Position& pos) const { return iccf_.probe(pos); }
    std::optional<Wdl> probe_theoretical(const Position& pos) const;
    const MateEntry*   probe_mate(const Position& pos) const { return mate_.probe(pos); }

   private:
    std::function<void()> on_reload_;
    ConsensusStore        consensus_;
    IccfStore             iccf_;
    TheoreticalStore      theoretical_;
    MateStore             mate_;
    std::string           evidence_root_;
    EvidenceInfoMode      evidence_info_ = EvidenceInfoMode::Root;
    ConsensusSearchMode   consensus_search_ = ConsensusSearchMode::MarkedOnly;
    IccfSearchMode        iccf_search_      = IccfSearchMode::Off;

    std::optional<std::string> ready_line(const char* label, bool ready,
                                          const std::string& version, size_t entries) const;
    std::optional<std::string> apply_layer_path(const char* label,
                                                std::string (Manager::*reload)(const std::string&),
                                                const std::string& path);
};

}  // namespace Stockfish::Evidence

#endif
