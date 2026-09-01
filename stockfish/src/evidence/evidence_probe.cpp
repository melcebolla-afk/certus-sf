/*
  certus-sf — probe hit/miss tests against testdata/ (Phase 1).
  Build: make -C stockfish/src evidence_probe
  Run:   ./stockfish/src/evidence_probe [repo_root]
*/

#include "../position.h"
#include "../bitboard.h"

#include "consensus_store.h"
#include "iccf_store.h"
#include "mate_store.h"
#include "theoretical_store.h"

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

void probe_consensus(ConsensusStore& store, StateListPtr& states, const std::string& fen,
                     const ConsensusEntry** out) {
    Position pos;
    pos.set(fen, false, &states->back());
    *out = store.probe(pos);
}

std::string td(const std::string& root, const char* layer) {
    return root + "/testdata/" + layer;
}

}  // namespace

int main(int argc, char** argv) {
    Bitboards::init();
    Position::init();

    const std::string root = (argc > 1) ? argv[1] : "../..";

    {
        StateListPtr states(new std::deque<StateInfo>(1));
        ConsensusStore store;
        check(store.load(td(root, "consensus")).empty(), "load consensus");
        check(store.ready(), "consensus ready");
        const ConsensusEntry* hit = nullptr;
        probe_consensus(store, states,
                        "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2", &hit);
        check(hit != nullptr, "consensus hit");
        check(hit && hit->wdl == Wdl::Draw, "consensus wdl draw");
        check(hit && !hit->marked_moves.empty(), "consensus marked_moves");
    }

    {
        StateListPtr states(new std::deque<StateInfo>(1));
        IccfStore store;
        check(store.load(td(root, "iccf")).empty(), "load iccf");
        Position pos;
        pos.set("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2", false,
                &states->back());
        check(store.probe(pos) != nullptr, "iccf hit");
        pos.set("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1", false,
                &states->back());
        check(store.probe(pos) == nullptr, "iccf gate miss low n");
    }

    {
        StateListPtr states(new std::deque<StateInfo>(1));
        TheoreticalStore store;
        check(store.load(td(root, "theoretical")).empty(), "load theoretical");
        Position pos;
        pos.set("8/8/8/8/8/8/8/4K2k w - - 0 1", false, &states->back());
        bool hit = false;
        check(store.probe(pos, &hit) == Wdl::Draw && hit, "theory hit draw");
    }

    {
        StateListPtr states(new std::deque<StateInfo>(1));
        MateStore store;
        check(store.load(td(root, "mate")).empty(), "load mate");
        Position pos;
        pos.set("7k/6Q1/6K1/8/8/8/8/8 w - - 0 1", false, &states->back());
        const auto* hit = store.probe(pos);
        check(hit != nullptr, "mate hit");
        check(hit && hit->stm_wins && hit->plies <= 5, "mate entry");
    }

    if (failures == 0)
        std::cout << "evidence_probe: all tests passed\n";
    else
        std::cerr << "evidence_probe: " << failures << " failure(s)\n";

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
