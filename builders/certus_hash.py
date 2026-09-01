"""Certus placement+STM Zobrist (paridad con stockfish/src/evidence/certus_hash.cpp)."""

from __future__ import annotations

import chess


class SplitMix64:
    def __init__(self, seed: int) -> None:
        self.state = seed & 0xFFFFFFFFFFFFFFFF

    def next(self) -> int:
        self.state = (self.state + 0x9E3779B97F4A7C15) & 0xFFFFFFFFFFFFFFFF
        z = self.state
        z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) & 0xFFFFFFFFFFFFFFFF
        z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) & 0xFFFFFFFFFFFFFFFF
        return (z ^ (z >> 31)) & 0xFFFFFFFFFFFFFFFF


def _build_keys() -> tuple[list[list[list[int]]], int]:
    rng = SplitMix64(0xC3A5CE27 ^ 0x9E3779B97F4A7C15)
    pieces = [[[0] * 64 for _ in range(6)] for _ in range(2)]
    for ci in range(2):
        for ki in range(6):
            for sq in range(64):
                pieces[ci][ki][sq] = rng.next()
    side = rng.next()
    return pieces, side


_PIECES, _SIDE = _build_keys()


def _piece_key(piece: chess.Piece, square: chess.Square) -> int:
    ci = 0 if piece.color == chess.WHITE else 1
    ki = piece.piece_type - 1
    return _PIECES[ci][ki][square]


def fen_with_defaults(fen: str) -> str:
    parts = fen.split()
    if len(parts) == 2:
        return f"{parts[0]} {parts[1]} - - 0 1"
    return fen


def hash_placement_stm_board(board: chess.Board) -> int:
    h = 0
    for square, piece in board.piece_map().items():
        h ^= _piece_key(piece, square)
    if board.turn == chess.BLACK:
        h ^= _SIDE
    return h & 0xFFFFFFFFFFFFFFFF


def hash_placement_stm_fen(fen: str) -> int:
    return hash_placement_stm_board(chess.Board(fen_with_defaults(fen)))


def fen_key(fen: str) -> str:
    parts = fen.split()
    if len(parts) < 2:
        return fen.strip()
    return f"{parts[0]} {parts[1]}"
