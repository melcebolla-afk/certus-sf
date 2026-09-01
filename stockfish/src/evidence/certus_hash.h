/*
  certus-sf — Certus-compatible Zobrist (placement + STM only).
  Must match evidence/crates/board/src/zobrist.rs for catalog keys.
*/

#ifndef CERTUS_HASH_H_INCLUDED
#define CERTUS_HASH_H_INCLUDED

#include <cstdint>
#include <optional>
#include <string>

#include "../position.h"

namespace Stockfish::Evidence {

using CertusKey = std::uint64_t;

CertusKey hash_placement_stm(const Position& pos);
CertusKey key_from_fen(const std::string& fen);

}  // namespace Stockfish::Evidence

#endif
