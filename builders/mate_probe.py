"""Exhaustive forced-mate probe (paridad evidence-engine/src/mate.rs)."""

from __future__ import annotations

from dataclasses import dataclass

import chess


@dataclass(frozen=True)
class MateResult:
    stm_wins: bool
    plies: int


def probe_mate_ungated(board: chess.Board, max_plies: int) -> MateResult | None:
    if board.is_checkmate():
        return MateResult(stm_wins=False, plies=0)

    legal = list(board.legal_moves)
    if not legal:
        return None

    win = _forced_win(board, max_plies)
    if win is not None:
        return MateResult(stm_wins=True, plies=win)

    loss = _forced_loss(board, max_plies)
    if loss is not None:
        return MateResult(stm_wins=False, plies=loss)

    return None


def _forced_win(board: chess.Board, max_depth: int) -> int | None:
    if max_depth <= 0:
        return None
    legal = list(board.legal_moves)
    if not legal:
        return None

    best: int | None = None
    for move in legal:
        board.push(move)
        if board.is_checkmate():
            board.pop()
            best = 1 if best is None else min(best, 1)
            continue

        opp = list(board.legal_moves)
        if not opp:
            board.pop()
            continue

        if max_depth < 2:
            board.pop()
            continue

        ok = True
        worst = 0
        for om in opp:
            board.push(om)
            rest = _forced_win(board, max_depth - 2)
            board.pop()
            if rest is None:
                ok = False
                break
            worst = max(worst, 2 + rest)

        board.pop()
        if ok:
            best = worst if best is None else min(best, worst)

    return best


def _forced_loss(board: chess.Board, max_depth: int) -> int | None:
    legal = list(board.legal_moves)
    if not legal:
        return 0 if board.is_checkmate() else None
    if max_depth <= 0:
        return None

    worst = 0
    for move in legal:
        board.push(move)
        opp_win = _forced_win(board, max_depth - 1)
        board.pop()
        if opp_win is None:
            return None
        worst = max(worst, 1 + opp_win)
    return worst
