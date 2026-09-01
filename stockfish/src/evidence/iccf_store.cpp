#include "iccf_store.h"

#include "catalog_path.h"
#include "consensus_store.h"
#include "json_reader.h"

namespace Stockfish::Evidence {

void IccfStore::clear() {
    map_.clear();
    content_version_.clear();
    min_n_           = 10;
    min_elo_         = 2000;
    min_date_        = "2000-01-01";
    min_confidence_  = 0.5f;
}

std::string IccfStore::load(const std::string& path) {
    if (path.empty())
    {
        clear();
        return {};
    }

    const std::string file = resolve_catalog_json(path);
    auto              json = Json::read_file(file);
    if (!json)
        return "IccfPath not a file: " + file;

    const auto schema = Json::u32_field(*json, "schema_version");
    if (!schema || *schema != 1)
        return "unsupported iccf schema_version";

    const auto layer = Json::string_field(*json, "layer");
    if (layer && !layer->empty() && *layer != "EMPIRICAL_ICCF")
        return "unexpected layer " + *layer;

    auto ver = Json::string_field(*json, "content_version");
    if (!ver)
        return "iccf catalog missing content_version";

    if (auto v = Json::u32_field(*json, "min_n"))
        min_n_ = *v;
    if (auto v = Json::u32_field(*json, "min_elo"))
        min_elo_ = *v;
    if (auto d = Json::string_field(*json, "min_date"))
        min_date_ = *d;
    if (auto v = Json::number_field(*json, "min_confidence"))
        min_confidence_ = static_cast<float>(*v);

    std::unordered_map<CertusKey, IccfEntry> map;
    Json::for_each_object_in_array(*json, "entries", [&](const std::string& obj) {
        auto fen = Json::string_field_obj(obj, "fen");
        auto wdl = Json::string_field_obj(obj, "wdl");
        auto conf = Json::number_field_obj(obj, "confidence");
        auto n = Json::u32_field_obj(obj, "n");
        if (!fen || !wdl || !conf || !n)
            return;
        auto parsed = try_parse_wdl(*wdl);
        if (!parsed)
            return;
        IccfEntry e;
        e.wdl        = *parsed;
        e.n          = *n;
        e.confidence = static_cast<float>(*conf);
        if (auto elo = Json::u32_field_obj(obj, "elo"))
            e.elo = *elo;
        if (auto date = Json::string_field_obj(obj, "date"))
            e.date = date->empty() ? "1970-01-01" : *date;
        else
            e.date = "1970-01-01";
        e.sources = Json::string_array_field_obj(obj, "sources");
        map[key_from_fen(*fen)] = std::move(e);
    });

    map_             = std::move(map);
    content_version_ = std::move(*ver);
    return {};
}

const IccfEntry* IccfStore::probe(const Position& pos) const {
    const auto it = map_.find(hash_placement_stm(pos));
    if (it == map_.end())
        return nullptr;
    const IccfEntry& e = it->second;
    if (e.n < min_n_)
        return nullptr;
    if (e.elo > 0 && e.elo < min_elo_)
        return nullptr;
    if (!e.date.empty() && e.date < min_date_)
        return nullptr;
    if (e.confidence + 1e-6f < min_confidence_)
        return nullptr;
    return &it->second;
}

}  // namespace Stockfish::Evidence
