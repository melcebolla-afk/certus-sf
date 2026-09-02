/*
  certus-sf — FEAT-0002/0003 consensus marked + ICCF frequent search probe.
  Build: make -C stockfish/src consensus_search_probe
*/

#include "../bitboard.h"
#include "../movegen.h"
#include "../position.h"
#include "../uci.h"

#include "../certus/certus_eval.h"
#include "../certus/certus_search.h"
#include "../evidence/evidence_manager.h"

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <string>

using namespace Stockfish;
using namespace Stockfish::Evidence;

namespace {

int failures = 0;

void check(bool ok, const char* msg) {
    if (!ok)
    {
        std::cerr << "FAIL: " << msg << '\n';
        ++failures;
    }
}

}  // namespace

int main(int argc, char** argv) {
    Bitboards::init();
    Position::init();

    const std::string root = (argc > 1) ? argv[1] : "../..";
    const std::string cpath = root + "/testdata/consensus";
    const std::string ipath = root + "/testdata/iccf";
    const char*       fen   = "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2";
    const char* fen_italian =
      "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3";

    Manager mgr;
    check(mgr.reload_consensus(cpath).empty(), "load consensus");
    check(mgr.reload_iccf(ipath).empty(), "load iccf");
    Certus::bind_evidence(mgr);

    StateListPtr states(new std::deque<StateInfo>(1));
    Position     pos;
    pos.set(fen, false, &states->back());

    const Move nc3 = UCIEngine::to_move(pos, "b1c3");
    const Move nf3 = UCIEngine::to_move(pos, "g1f3");
    const Move a3  = UCIEngine::to_move(pos, "a2a3");
    check(nc3 != Move::none() && nf3 != Move::none() && a3 != Move::none(), "uci moves");

    const auto marked = Certus::consensus_marked_legal_moves(pos);
    check(marked.size() == 2, "two legal marked");
    check(std::find(marked.begin(), marked.end(), nc3) != marked.end(), "marked nc3");
    check(std::find(marked.begin(), marked.end(), nf3) != marked.end(), "marked nf3");

    mgr.set_consensus_search(ConsensusSearchMode::Off);
    check(Certus::allow_search_move(pos, a3, false, 0), "Off allows a3");

    mgr.set_consensus_search(ConsensusSearchMode::MarkedOnly);
    check(Certus::allow_search_move(pos, nc3, false, 0), "MarkedOnly allows nc3");
    check(Certus::allow_search_move(pos, nf3, false, 0), "MarkedOnly allows nf3");
    check(!Certus::allow_search_move(pos, a3, false, 0), "MarkedOnly blocks a3");
    check(Certus::allow_search_move(pos, a3, true, 0), "in_check allows a3");
    check(Certus::allow_search_move(pos, a3, false, 1), "pvIdx>0 allows a3");

    // FEAT-0003: ICCF frequent_moves filter (no root force). Consensus Off so ICCF applies.
    mgr.set_consensus_search(ConsensusSearchMode::Off);
    mgr.set_iccf_search(IccfSearchMode::Off);
    check(Certus::allow_search_move(pos, a3, false, 0), "Iccf Off allows a3 on e4e5");

    mgr.set_iccf_search(IccfSearchMode::FreqOnly);
    check(Certus::allow_search_move(pos, nf3, false, 0), "FreqOnly allows nf3");
    check(Certus::allow_search_move(pos, nc3, false, 0), "FreqOnly allows nc3");
    check(!Certus::allow_search_move(pos, a3, false, 0), "FreqOnly blocks a3 on e4e5");

    // Italian: ICCF-only position (no consensus entry).
    pos.set(fen_italian, false, &states->back());
    const Move bc4 = UCIEngine::to_move(pos, "f1c4");
    const Move d4  = UCIEngine::to_move(pos, "d2d4");
    const Move a2a3 = UCIEngine::to_move(pos, "a2a3");
    check(bc4 != Move::none() && d4 != Move::none() && a2a3 != Move::none(), "italian uci");
    const auto freq = Certus::iccf_frequent_legal_moves(pos);
    check(freq.size() == 3, "three frequent italian");
    check(std::find(freq.begin(), freq.end(), bc4) != freq.end(), "freq bc4");
    check(!Certus::allow_search_move(pos, a2a3, false, 0), "FreqOnly blocks a3 italian");
    check(Certus::allow_search_move(pos, bc4, false, 0), "FreqOnly allows bc4");

    // Precedence: MarkedOnly on e4e5 wins over ICCF (same fen has both).
    pos.set(fen, false, &states->back());
    mgr.set_consensus_search(ConsensusSearchMode::MarkedOnly);
    mgr.set_iccf_search(IccfSearchMode::FreqOnly);
    check(!Certus::allow_search_move(pos, a3, false, 0), "consensus still blocks a3");
    check(Certus::allow_search_move(pos, nf3, false, 0), "consensus allows nf3");

    if (failures == 0)
        std::cout << "consensus_search_probe: all tests passed\n";
    else
        std::cerr << "consensus_search_probe: " << failures << " failure(s)\n";

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
