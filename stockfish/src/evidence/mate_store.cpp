#include "mate_store.h"

#include "catalog_path.h"
#include "json_reader.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sys/stat.h>
#include <sys/stat.h>

namespace Stockfish::Evidence {

namespace {

constexpr char     IDX_MAGIC[4] = {'C', 'M', 'T', 'E'};
constexpr std::uint32_t IDX_SCHEMA = 1;
constexpr std::size_t   ENTRY_SIZE = 16;

std::uint64_t read_u64_le(const std::uint8_t* p) {
    std::uint64_t v = 0;
    for (int i = 0; i < 8; ++i)
        v |= std::uint64_t(p[i]) << (8 * i);
    return v;
}

std::uint32_t read_u32_le(const std::uint8_t* p) {
    std::uint32_t v = 0;
    for (int i = 0; i < 4; ++i)
        v |= std::uint32_t(p[i]) << (8 * i);
    return v;
}

bool idx_is_fresh(const std::string& idx, const std::string& json) {
    if (!is_regular_file(idx))
        return false;
    if (!is_regular_file(json))
        return true;
    struct stat si {}, sj {};
    if (stat(idx.c_str(), &si) != 0 || stat(json.c_str(), &sj) != 0)
        return true;
#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L
    const double ti = si.st_mtim.tv_sec + si.st_mtim.tv_nsec * 1e-9;
    const double tj = sj.st_mtim.tv_sec + sj.st_mtim.tv_nsec * 1e-9;
#else
    const double ti = si.st_mtime;
    const double tj = sj.st_mtime;
#endif
    return ti >= tj;
}

std::string idx_paths(const std::string& path, std::string& json_file, std::string& idx_file) {
    if (is_directory(path))
    {
        json_file = resolve_catalog_json(path);
        idx_file  = path;
        if (!idx_file.empty() && idx_file.back() != '/')
            idx_file += '/';
        idx_file += "catalog.idx";
    }
    else
    {
        json_file = path;
        idx_file  = path;
        const auto dot = idx_file.rfind('.');
        if (dot != std::string::npos)
            idx_file = idx_file.substr(0, dot);
        idx_file += ".idx";
    }
    return {};
}

}  // namespace

void MateStore::clear() {
    backend_         = Backend::Empty;
    heap_.clear();
    mapped_.clear();
    table_off_       = 0;
    count_           = 0;
    content_version_.clear();
}

const MateEntry* MateStore::mmap_probe(CertusKey key) const {
    if (count_ == 0)
        return nullptr;
    std::size_t lo = 0, hi = count_;
    while (lo < hi)
    {
        const std::size_t mid = lo + (hi - lo) / 2;
        const std::size_t off = table_off_ + mid * ENTRY_SIZE;
        const CertusKey   k   = read_u64_le(mapped_.data() + off);
        if (k < key)
            lo = mid + 1;
        else if (k > key)
            hi = mid;
        else
        {
            static thread_local MateEntry e;
            e.plies    = mapped_[off + 8];
            e.stm_wins = mapped_[off + 9] != 0;
            return &e;
        }
    }
    return nullptr;
}

std::string MateStore::open_mapped(const std::string& idx_path, MateStore& out) {
    std::ifstream in(idx_path, std::ios::binary);
    if (!in)
        return "open idx: " + idx_path;
    in.seekg(0, std::ios::end);
    const auto sz = static_cast<std::size_t>(in.tellg());
    in.seekg(0, std::ios::beg);
    if (sz < 20)
        return "idx too small";
    out.mapped_.resize(sz);
    in.read(reinterpret_cast<char*>(out.mapped_.data()), sz);
    if (!in)
        return "read idx failed";

    if (std::memcmp(out.mapped_.data(), IDX_MAGIC, 4) != 0)
        return "bad idx magic";
    if (read_u32_le(out.mapped_.data() + 4) != IDX_SCHEMA)
        return "unsupported idx schema";

    const std::size_t n = static_cast<std::size_t>(read_u64_le(out.mapped_.data() + 8));
    const std::size_t ver_len = read_u32_le(out.mapped_.data() + 16);
    const std::size_t ver_end = 20 + ver_len;
    if (ver_end > sz)
        return "idx ver truncated";

    out.content_version_ = std::string(reinterpret_cast<const char*>(out.mapped_.data() + 20),
                                       ver_len);
    out.table_off_ = ver_end;
    out.count_     = n;
    const std::size_t need = ver_end + n * ENTRY_SIZE;
    if (need > sz)
        return "idx truncated";
    out.backend_ = Backend::Mapped;
    out.heap_.clear();
    return {};
}

std::string MateStore::load_json_heap(const std::string& json_path, MateStore& out) {
    auto json = Json::read_file(json_path);
    if (!json)
        return "read mate: " + json_path;

    const auto schema = Json::u32_field(*json, "schema_version");
    if (!schema || *schema != 1)
        return "unsupported mate schema_version";

    const auto layer = Json::string_field(*json, "layer");
    if (layer && !layer->empty() && *layer != "PROVEN_MATE")
        return "unexpected layer " + *layer;

    auto ver = Json::string_field(*json, "content_version");
    if (!ver)
        return "mate catalog missing content_version";

    std::unordered_map<CertusKey, MateEntry> map;
    Json::for_each_object_in_array(*json, "entries", [&](const std::string& obj) {
        auto fen = Json::string_field_obj(obj, "fen");
        auto plies = Json::u32_field_obj(obj, "plies");
        if (!fen || !plies)
            return;
        MateEntry e;
        e.plies = static_cast<std::uint8_t>(*plies);
        if (auto b = Json::bool_field_obj(obj, "stm_wins"))
            e.stm_wins = *b;
        else if (auto n = Json::number_field_obj(obj, "stm_wins"))
            e.stm_wins = *n != 0.0;
        map[key_from_fen(*fen)] = e;
    });

    out.heap_            = std::move(map);
    out.count_           = out.heap_.size();
    out.content_version_ = std::move(*ver);
    out.backend_         = Backend::Heap;
    out.mapped_.clear();
    return {};
}

std::string MateStore::load(const std::string& path) {
    if (path.empty())
    {
        clear();
        return {};
    }

    std::string json_file, idx_file;
    idx_paths(path, json_file, idx_file);

    if (idx_is_fresh(idx_file, json_file))
    {
        MateStore tmp;
        const std::string err = open_mapped(idx_file, tmp);
        if (err.empty())
        {
            *this = std::move(tmp);
            return {};
        }
        if (!is_regular_file(json_file))
            return "MatePath idx unusable (" + err + ") and no catalog.json";
    }

    if (!is_regular_file(json_file))
        return "MatePath: need catalog.idx or catalog.json";

    MateStore tmp;
    const std::string err = load_json_heap(json_file, tmp);
    if (!err.empty())
        return err;

    // Prefer idx if present and fresh after JSON load; else heap fallback.
    if (idx_is_fresh(idx_file, json_file))
    {
        MateStore mapped;
        if (open_mapped(idx_file, mapped).empty())
        {
            *this = std::move(mapped);
            return {};
        }
    }

    *this = std::move(tmp);
    return {};
}

const MateEntry* MateStore::probe(const Position& pos) const {
    const CertusKey key = hash_placement_stm(pos);
    if (backend_ == Backend::Mapped)
        return mmap_probe(key);
    if (backend_ == Backend::Heap)
    {
        const auto it = heap_.find(key);
        return it == heap_.end() ? nullptr : &it->second;
    }
    return nullptr;
}

}  // namespace Stockfish::Evidence
