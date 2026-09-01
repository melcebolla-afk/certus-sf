#ifndef EVIDENCE_CATALOG_PATH_H_INCLUDED
#define EVIDENCE_CATALOG_PATH_H_INCLUDED

#include <string>

namespace Stockfish::Evidence {

std::string resolve_catalog_json(const std::string& path);
bool        is_directory(const std::string& path);
bool        is_regular_file(const std::string& path);

}  // namespace Stockfish::Evidence

#endif
