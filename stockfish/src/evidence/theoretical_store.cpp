#include "theoretical_store.h"

#include "catalog_path.h"
#include "json_reader.h"

namespace Stockfish::Evidence {

TheoryResult parse_theory_result(const std::string& s) {
    if (auto r = try_parse_theory_result(s))
        return *r;
    return Wdl::Draw;
}

std::optional<TheoryResult> try_parse_theory_result(const std::string& s) {
    std::string t;
    for (char c : s)
        t += char(std::toupper(static_cast<unsigned char>(c)));
    if (t == "W" || t == "WIN" || t == "1-0")
        return Wdl::Win;
    if (t == "D" || t == "DRAW" || t == "1/2-1/2" || t == "1/2")
        return Wdl::Draw;
    if (t == "L" || t == "LOSS" || t == "0-1")
        return Wdl::Loss;
    return std::nullopt;
}

void TheoreticalStore::clear() {
    map_.clear();
    content_version_.clear();
}

std::string TheoreticalStore::load(const std::string& path) {
    if (path.empty())
    {
        clear();
        return {};
    }

    const std::string file = resolve_catalog_json(path);
    auto              json = Json::read_file(file);
    if (!json)
        return "TheoreticalPath not a file: " + file;

    const auto schema = Json::u32_field(*json, "schema_version");
    if (!schema || *schema != 1)
        return "unsupported theory schema_version";

    auto ver = Json::string_field(*json, "content_version");
    if (!ver)
        return "theory catalog missing content_version";

    std::unordered_map<CertusKey, TheoryResult> map;
    Json::for_each_object_in_array(*json, "entries", [&](const std::string& obj) {
        auto fen = Json::string_field_obj(obj, "fen");
        auto res = Json::string_field_obj(obj, "result");
        if (!fen || !res)
            return;
        auto parsed = try_parse_theory_result(*res);
        if (!parsed)
            return;
        map[key_from_fen(*fen)] = *parsed;
    });

    map_             = std::move(map);
    content_version_ = std::move(*ver);
    return {};
}

TheoryResult TheoreticalStore::probe(const Position& pos, bool* hit) const {
    const auto it = map_.find(hash_placement_stm(pos));
    if (it == map_.end())
    {
        if (hit)
            *hit = false;
        return Wdl::Draw;
    }
    if (hit)
        *hit = true;
    return it->second;
}

}  // namespace Stockfish::Evidence
