#include "evidence_root.h"

#include "catalog_path.h"

#include <dirent.h>
#include <sys/stat.h>

#include <algorithm>
#include <string>
#include <vector>

namespace Stockfish::Evidence {

namespace {

struct Candidate {
    double      mtime;
    std::string name;
    std::string path;
};

double file_mtime(const std::string& path) {
    struct stat st {};
    if (stat(path.c_str(), &st) != 0)
        return 0.0;
#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L
    return static_cast<double>(st.st_mtim.tv_sec)
         + static_cast<double>(st.st_mtim.tv_nsec) * 1e-9;
#else
    return static_cast<double>(st.st_mtime);
#endif
}

}  // namespace

std::optional<std::string> newest_version_dir(const std::string& layer_root) {
    if (!is_directory(layer_root))
        return std::nullopt;

    DIR* dir = opendir(layer_root.c_str());
    if (!dir)
        return std::nullopt;

    std::vector<Candidate> candidates;
    while (dirent* ent = readdir(dir))
    {
        if (ent->d_name[0] == '.')
            continue;
        std::string sub = layer_root;
        if (!sub.empty() && sub.back() != '/')
            sub += '/';
        sub += ent->d_name;
        if (!is_directory(sub))
            continue;
        const std::string catalog = resolve_catalog_json(sub);
        if (!is_regular_file(catalog))
            continue;
        candidates.push_back({file_mtime(catalog), ent->d_name, sub});
    }
    closedir(dir);

    if (candidates.empty())
        return std::nullopt;

    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
        if (a.mtime != b.mtime)
            return a.mtime > b.mtime;
        return a.name > b.name;
    });
    return candidates.front().path;
}

std::optional<std::string> resolve_layer(const std::string& evidence_root, const char* layer) {
    std::string layer_root = evidence_root;
    if (!layer_root.empty() && layer_root.back() != '/')
        layer_root += '/';
    layer_root += layer;
    return newest_version_dir(layer_root);
}

}  // namespace Stockfish::Evidence
