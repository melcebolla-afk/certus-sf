#ifndef THEORETICAL_STORE_H_INCLUDED
#define THEORETICAL_STORE_H_INCLUDED

#include <optional>
#include <optional>
#include <string>
#include <unordered_map>

#include "../types.h"
#include "certus_hash.h"
#include "consensus_store.h"

namespace Stockfish::Evidence {

using TheoryResult = Wdl;

class TheoreticalStore {
   public:
    TheoreticalStore() { clear(); }

    void  clear();
    bool  empty() const { return map_.empty(); }
    size_t size() const { return map_.size(); }
    bool  ready() const { return !empty(); }

    const std::string& content_version() const { return content_version_; }

    std::string load(const std::string& path);
    TheoryResult probe(const Position& pos, bool* hit) const;

   private:
    std::unordered_map<CertusKey, TheoryResult> map_;
    std::string                                 content_version_;
};

TheoryResult parse_theory_result(const std::string& s);
std::optional<TheoryResult> try_parse_theory_result(const std::string& s);
std::optional<TheoryResult> try_parse_theory_result(const std::string& s);

}  // namespace Stockfish::Evidence

#endif
