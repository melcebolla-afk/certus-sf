#ifndef EVIDENCE_ROOT_H_INCLUDED
#define EVIDENCE_ROOT_H_INCLUDED

#include <optional>
#include <string>

namespace Stockfish::Evidence {

std::optional<std::string> newest_version_dir(const std::string& layer_root);
std::optional<std::string> resolve_layer(const std::string& evidence_root, const char* layer);

}  // namespace Stockfish::Evidence

#endif
