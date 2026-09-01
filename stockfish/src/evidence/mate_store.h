#ifndef MATE_STORE_H_INCLUDED
#define MATE_STORE_H_INCLUDED

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "../types.h"
#include "certus_hash.h"

namespace Stockfish::Evidence {

struct MateEntry {
    bool     stm_wins;
    std::uint8_t plies;
};

class MateStore {
   public:
    MateStore() { clear(); }

    void  clear();
    bool  empty() const { return count_ == 0; }
    size_t size() const { return count_; }
    bool  ready() const { return !empty(); }

    const std::string& content_version() const { return content_version_; }

    std::string load(const std::string& path);
    const MateEntry* probe(const Position& pos) const;

   private:
    enum class Backend { Empty, Heap, Mapped };

    Backend                              backend_ = Backend::Empty;
    std::unordered_map<CertusKey, MateEntry> heap_;
    std::vector<std::uint8_t>            mapped_;
    std::size_t                          table_off_ = 0;
    std::size_t                          count_     = 0;
    std::string                          content_version_;

    static std::string open_mapped(const std::string& idx_path, MateStore& out);
    static std::string load_json_heap(const std::string& json_path, MateStore& out);
    const MateEntry*   mmap_probe(CertusKey key) const;
};

}  // namespace Stockfish::Evidence

#endif
