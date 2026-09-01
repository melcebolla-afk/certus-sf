#ifndef ICCF_STORE_H_INCLUDED
#define ICCF_STORE_H_INCLUDED

#include <string>
#include <unordered_map>
#include <vector>

#include "../types.h"
#include "certus_hash.h"
#include "consensus_store.h"

namespace Stockfish::Evidence {

struct IccfEntry {
    Wdl                    wdl;
    std::uint32_t          n;
    std::uint32_t          elo;
    std::string            date;
    float                  confidence;
    std::vector<std::string> sources;
};

class IccfStore {
   public:
    IccfStore() { clear(); }

    void  clear();
    bool  empty() const { return map_.empty(); }
    size_t size() const { return map_.size(); }
    bool  ready() const { return !empty(); }

    const std::string& content_version() const { return content_version_; }

    std::string load(const std::string& path);
    const IccfEntry* probe(const Position& pos) const;

   private:
    std::unordered_map<CertusKey, IccfEntry> map_;
    std::string                              content_version_;
    std::uint32_t                            min_n_;
    std::uint32_t                            min_elo_;
    std::string                              min_date_;
    float                                    min_confidence_;
};

}  // namespace Stockfish::Evidence

#endif
