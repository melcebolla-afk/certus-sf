#ifndef CONSENSUS_STORE_H_INCLUDED
#define CONSENSUS_STORE_H_INCLUDED

#include <optional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../types.h"
#include "certus_hash.h"

namespace Stockfish::Evidence {

enum class Wdl { Win, Draw, Loss };

struct ConsensusEntry {
    Wdl                    wdl;
    float                  confidence;
    std::vector<std::string> marked_moves;
    std::vector<std::string> sources;
};

class ConsensusStore {
   public:
    ConsensusStore() { clear(); }

    void  clear();
    bool  empty() const { return map_.empty(); }
    size_t size() const { return map_.size(); }
    bool  ready() const { return !empty(); }

    const std::string& content_version() const { return content_version_; }
    float              min_confidence() const { return min_confidence_; }

    std::string load(const std::string& path);
    const ConsensusEntry* probe(const Position& pos) const;

   private:
    std::unordered_map<CertusKey, ConsensusEntry> map_;
    std::string                                   content_version_;
    float                                         min_confidence_;
};

Wdl parse_wdl(const std::string& s);
std::optional<Wdl> try_parse_wdl(const std::string& s);
std::optional<Wdl> try_parse_wdl(const std::string& s);

}  // namespace Stockfish::Evidence

#endif
