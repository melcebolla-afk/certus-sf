#include "catalog_path.h"

#include <sys/stat.h>

namespace Stockfish::Evidence {

bool is_directory(const std::string& path) {
    struct stat st {};
    if (stat(path.c_str(), &st) != 0)
        return false;
    return S_ISDIR(st.st_mode);
}

bool is_regular_file(const std::string& path) {
    struct stat st {};
    if (stat(path.c_str(), &st) != 0)
        return false;
    return S_ISREG(st.st_mode);
}

std::string resolve_catalog_json(const std::string& path) {
    if (path.empty())
        return {};
    if (is_directory(path))
    {
        std::string catalog = path;
        if (!catalog.empty() && catalog.back() != '/')
            catalog += '/';
        catalog += "catalog.json";
        return catalog;
    }
    return path;
}

}  // namespace Stockfish::Evidence
