/*
  certus-sf — UCI product identity (Stockfish version in string).
*/

#include "certus_engine.h"

#include "../misc.h"

namespace Stockfish::Certus {

std::string engine_identity(bool to_uci) {
    if (to_uci)
        return std::string("certus-sf dev\nid author certus-sf (") + engine_version_info()
             + " fork)";
    return std::string("certus-sf dev (") + engine_version_info() + " fork)";
}

}  // namespace Stockfish::Certus
