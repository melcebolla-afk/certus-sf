#include "evidence_root.h"

#include "catalog_path.h"
#include "json_reader.h"

#include <dirent.h>
#include <sys/stat.h>

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace Stockfish::Evidence {

namespace {

struct Candidate {
    int         version_ymd = 0;  // YYYYMMDD from dir name / content_version; 0 = unknown
    double      mtime       = 0;
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

// Extract YYYYMMDD from strings like v2026.09.02, viccf-2026.08.29, iccf-2026.08.29, 2026-09-02.
int parse_version_ymd(const std::string& s) {
    const std::size_t n = s.size();
    for (std::size_t i = 0; i + 9 < n; ++i)
    {
        if (!std::isdigit(static_cast<unsigned char>(s[i]))
            || !std::isdigit(static_cast<unsigned char>(s[i + 1]))
            || !std::isdigit(static_cast<unsigned char>(s[i + 2]))
            || !std::isdigit(static_cast<unsigned char>(s[i + 3])))
            continue;
        const char sep1 = s[i + 4];
        if (sep1 != '.' && sep1 != '-')
            continue;
        if (!std::isdigit(static_cast<unsigned char>(s[i + 5]))
            || !std::isdigit(static_cast<unsigned char>(s[i + 6])))
            continue;
        const char sep2 = s[i + 7];
        if (sep2 != '.' && sep2 != '-')
            continue;
        if (!std::isdigit(static_cast<unsigned char>(s[i + 8]))
            || !std::isdigit(static_cast<unsigned char>(s[i + 9])))
            continue;
        // Prefer match not followed by another digit (avoid eating longer numbers).
        if (i + 10 < n && std::isdigit(static_cast<unsigned char>(s[i + 10])))
            continue;

        const int y = (s[i] - '0') * 1000 + (s[i + 1] - '0') * 100 + (s[i + 2] - '0') * 10
                      + (s[i + 3] - '0');
        const int m = (s[i + 5] - '0') * 10 + (s[i + 6] - '0');
        const int d = (s[i + 8] - '0') * 10 + (s[i + 9] - '0');
        if (y < 1990 || y > 2100 || m < 1 || m > 12 || d < 1 || d > 31)
            continue;
        return y * 10000 + m * 100 + d;
    }
    return 0;
}

int version_key_for_candidate(const std::string& dir_name, const std::string& catalog_path) {
    if (const int from_dir = parse_version_ymd(dir_name))
        return from_dir;
    // Fallback: content_version inside catalog.json (git pull may reset mtimes).
    if (auto json = Json::read_file(catalog_path))
    {
        if (auto ver = Json::string_field(*json, "content_version"))
            return parse_version_ymd(*ver);
    }
    return 0;
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
        Candidate c;
        c.name        = ent->d_name;
        c.path        = sub;
        c.mtime       = file_mtime(catalog);
        c.version_ymd = version_key_for_candidate(c.name, catalog);
        candidates.push_back(std::move(c));
    }
    closedir(dir);

    if (candidates.empty())
        return std::nullopt;

    // Prefer calendar version (stable across git clone/pull); mtime only as last resort.
    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
        if (a.version_ymd != b.version_ymd)
            return a.version_ymd > b.version_ymd;
        if (a.name != b.name)
            return a.name > b.name;
        return a.mtime > b.mtime;
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
