/*
  certus-sf — Certus-compatible Zobrist (placement + STM only).
*/

#include "certus_hash.h"

#include "../position.h"

#include <deque>
#include <sstream>
#include <string>

namespace Stockfish::Evidence {

namespace {

struct SplitMix64 {
    std::uint64_t state;

    explicit SplitMix64(std::uint64_t seed) : state(seed) {}

    std::uint64_t next() {
        state += 0x9E3779B97F4A7C15ULL;
        std::uint64_t z = state;
        z             = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z             = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }
};

struct CertusZobrist {
    std::uint64_t pieces[2][6][64];
    std::uint64_t side;

    CertusZobrist() {
        SplitMix64 rng(0xC3A5CE27ULL ^ 0x9E3779B97F4A7C15ULL);
        for (auto& color : pieces)
            for (auto& kind : color)
                for (auto& sq : kind)
                    sq = rng.next();
        side = rng.next();
    }

    std::uint64_t piece(Piece pc, Square s) const {
        const Color  c  = color_of(pc);
        const int    ci = c == WHITE ? 0 : 1;
        const int    ki = type_of(pc) - PAWN;
        return pieces[ci][ki][s];
    }
};

const CertusZobrist& keys() {
    static const CertusZobrist k;
    return k;
}

std::string fen_with_defaults(const std::string& fen) {
    std::istringstream iss(fen);
    std::string        board, stm;
    iss >> board >> stm;
    if (board.empty() || stm.empty())
        return fen;
    return board + " " + stm + " - - 0 1";
}

}  // namespace

CertusKey hash_placement_stm(const Position& pos) {
    CertusKey h = 0;
    for (Square s = SQ_A1; s <= SQ_H8; ++s)
    {
        const Piece pc = pos.piece_on(s);
        if (pc != NO_PIECE)
            h ^= keys().piece(pc, s);
    }
    if (pos.side_to_move() == BLACK)
        h ^= keys().side;
    return h;
}

CertusKey key_from_fen(const std::string& fen) {
    StateListPtr states(new std::deque<StateInfo>(1));
    Position     pos;

    std::istringstream iss(fen);
    std::string        board, stm, third;
    iss >> board >> stm >> third;
    const std::string full =
      (!board.empty() && !stm.empty() && third.empty()) ? fen_with_defaults(fen) : fen;
    pos.set(full, false, &states->back());
    return hash_placement_stm(pos);
}

}  // namespace Stockfish::Evidence
