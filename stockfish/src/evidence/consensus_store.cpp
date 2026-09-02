#include "consensus_store.h"

#include "catalog_path.h"
#include "json_reader.h"

namespace Stockfish::Evidence {

Wdl parse_wdl(const std::string& s) {
    if (auto w = try_parse_wdl(s))
        return *w;
    return Wdl::Draw;
}

std::optional<Wdl> try_parse_wdl(const std::string& s) {
    std::string t;
    t.reserve(s.size());
    for (char c : s)
        t += char(std::tolower(static_cast<unsigned char>(c)));
    if (t == "w" || t == "win" || t == "1-0")
        return Wdl::Win;
    if (t == "d" || t == "draw" || t == "1/2-1/2" || t == "1/2")
        return Wdl::Draw;
    if (t == "l" || t == "loss" || t == "0-1")
        return Wdl::Loss;
    return std::nullopt;
}

void ConsensusStore::clear() {
    map_.clear();
    content_version_.clear();
    min_confidence_ = 0.8f;
}

std::string ConsensusStore::load(const std::string& path) {
    if (path.empty())
    {
        clear();
        return {};
    }

    const std::string file = resolve_catalog_json(path);
    auto              json = Json::read_file(file);
    if (!json)
        return "ConsensusPath not a file: " + file;

    const auto schema = Json::u32_field(*json, "schema_version");
    if (!schema || *schema != 1)
        return "unsupported consensus schema_version";

    const auto layer = Json::string_field(*json, "layer");
    if (layer && !layer->empty() && *layer != "STRONG_CONSENSUS")
        return "unexpected layer " + *layer;

    auto ver = Json::string_field(*json, "content_version");
    if (!ver)
        return "consensus catalog missing content_version";

    float min_conf = 0.8f;
    if (auto mc = Json::number_field(*json, "min_confidence"))
        min_conf = static_cast<float>(*mc);

    std::unordered_map<CertusKey, ConsensusEntry> map;
    Json::for_each_object_in_array(*json, "entries", [&](const std::string& obj) {
        auto fen  = Json::string_field_obj(obj, "fen");
        auto conf = Json::number_field_obj(obj, "confidence");
        if (!fen || !conf)
            return;
        // wdl optional (FEAT-0002+: unused for search score; default draw if absent).
        Wdl wdl = Wdl::Draw;
        if (auto wdl_s = Json::string_field_obj(obj, "wdl"))
        {
            auto parsed = try_parse_wdl(*wdl_s);
            if (!parsed)
                return;
            wdl = *parsed;
        }
        ConsensusEntry e;
        e.wdl          = wdl;
        e.confidence   = static_cast<float>(*conf);
        e.marked_moves = Json::string_array_field_obj(obj, "marked_moves");
        e.sources      = Json::string_array_field_obj(obj, "sources");
        map[key_from_fen(*fen)] = std::move(e);
    });

    map_              = std::move(map);
    content_version_  = std::move(*ver);
    min_confidence_   = min_conf;
    return {};
}

const ConsensusEntry* ConsensusStore::probe(const Position& pos) const {
    const auto it = map_.find(hash_placement_stm(pos));
    if (it == map_.end())
        return nullptr;
    if (it->second.confidence + 1e-6f < min_confidence_)
        return nullptr;
    return &it->second;
}

}  // namespace Stockfish::Evidence
