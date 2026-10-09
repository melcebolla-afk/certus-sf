/*
  certus-sf — FEAT-0002/0003/0004/0006/0007 consensus + ICCF + CertusStyle + MixedEffort probe.
  Build: make -C stockfish/src consensus_search_probe
*/

#include "../attacks.h"
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
    Attacks::init();
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

    // FEAT-0006: Mixed High (default) never hard-filters; stronger effort.
    mgr.set_certus_style(CertusStyleMode::Mixed);
    mgr.set_mixed_effort(MixedEffortMode::High);
    mgr.set_consensus_search(ConsensusSearchMode::MarkedOnly);
    check(Certus::allow_search_move(pos, a3, false, 0), "Mixed High allows a3");
    {
        const auto f = Certus::make_search_move_filter(pos, false, 0);
        check(f.boost_preferred && f.is_preferred(nc3) && !f.restrict_moves, "Mixed High boosts marked");
        check(f.interior_depth && f.lmr_relief == 2048, "Mixed High interior + LMR-2");
        Depth ext = 0;
        Depth red = 2048;
        Certus::apply_style_depth_bias(f, nc3, false, ext, red);
        check(red == 0 && ext == 1, "Mixed High interior: LMR-2 + extension");
        ext = 0;
        red = 2048;
        Certus::apply_style_depth_bias(f, a3, false, ext, red);
        check(red == 2048 && ext == 0, "Mixed High non-preferred untouched");
        const auto f1 = Certus::make_search_move_filter(pos, false, 1);
        check(f1.boost_preferred && f1.is_preferred(nc3) && !f1.restrict_moves,
              "Mixed High boosts pvIdx>0");
        check(Certus::allow_search_move(pos, a3, false, 1), "Mixed High pvIdx>0 still allows a3");
    }

    // FEAT-0007: Mixed Max — LMR≈0, inherits High MultiPV/ext, picker bonus TLS.
    mgr.set_mixed_effort(MixedEffortMode::Max);
    check(Certus::allow_search_move(pos, a3, false, 0), "Mixed Max allows a3");
    {
        const auto f = Certus::make_search_move_filter(pos, false, 0);
        check(f.boost_preferred && f.is_preferred(nc3) && !f.restrict_moves, "Mixed Max boosts marked");
        check(f.interior_depth && f.zero_lmr, "Mixed Max interior + zero LMR");
        Depth ext = 0;
        Depth red = 3072;
        Certus::apply_style_depth_bias(f, nc3, false, ext, red);
        check(red == 0 && ext == 1, "Mixed Max interior: LMR 0 + extension");
        ext = 0;
        red = 3072;
        Certus::apply_style_depth_bias(f, a3, false, ext, red);
        check(red == 3072 && ext == 0, "Mixed Max non-preferred untouched");
        check(Certus::make_search_move_filter(pos, false, 1).boost_preferred,
              "Mixed Max boosts pvIdx>0");
        Certus::set_move_picker_boost(&f);
        check(Certus::move_picker_preferred_bonus(nc3) == 8000, "Max picker bonus preferred");
        check(Certus::move_picker_preferred_bonus(a3) == 0, "Max picker bonus non-preferred");
        Certus::set_move_picker_boost(nullptr);
        check(Certus::move_picker_preferred_bonus(nc3) == 0, "picker boost cleared");
    }

    // FEAT-0006: Mixed Low = FEAT-0004 (LMR-1, no ext, no MultiPV boost).
    mgr.set_mixed_effort(MixedEffortMode::Low);
    check(Certus::allow_search_move(pos, a3, false, 0), "Mixed Low allows a3");
    {
        const auto f = Certus::make_search_move_filter(pos, false, 0);
        check(f.boost_preferred && f.is_preferred(nc3) && !f.restrict_moves, "Mixed Low boosts marked");
        check(!f.interior_depth && f.lmr_relief == 1024, "Mixed Low has no interior extension flag");
        Depth ext = 0;
        Depth red = 2048;
        Certus::apply_style_depth_bias(f, nc3, false, ext, red);
        check(red == 1024 && ext == 0, "Mixed Low interior: LMR-1 no extension");
        ext = 0;
        red = 2048;
        Certus::apply_style_depth_bias(f, a3, false, ext, red);
        check(red == 2048 && ext == 0, "Mixed Low non-preferred untouched");
        check(!Certus::make_search_move_filter(pos, false, 1).boost_preferred,
              "Mixed Low no boost pvIdx>0");
    }

    mgr.set_certus_style(CertusStyleMode::Strict);
    mgr.set_mixed_effort(MixedEffortMode::Max);
    mgr.set_consensus_search(ConsensusSearchMode::MarkedOnly);
    {
        const auto f = Certus::make_search_move_filter(pos, false, 0);
        check(f.interior_depth && f.restrict_moves, "Strict filters + interior depth");
        check(!f.zero_lmr && f.lmr_relief == 1024, "Strict ignores MixedEffort Max LMR");
        Depth ext = 0;
        Depth red = 2048;
        Certus::apply_style_depth_bias(f, nc3, false, ext, red);
        check(red == 1024 && ext == 1, "Strict interior: LMR-1 + extension");
        check(!Certus::make_search_move_filter(pos, false, 1).restrict_moves,
              "Strict pvIdx>0 no filter");
    }
    mgr.set_certus_style(CertusStyleMode::Mixed);
    mgr.set_mixed_effort(MixedEffortMode::High);
    mgr.set_consensus_search(ConsensusSearchMode::MarkedOnly);

    mgr.set_certus_style(CertusStyleMode::Off);
    check(Certus::allow_search_move(pos, a3, false, 0), "Style Off allows a3");
    check(!Certus::make_search_move_filter(pos, false, 0).boost_preferred, "Style Off no boost");

    mgr.set_certus_style(CertusStyleMode::Strict);
    mgr.set_consensus_search(ConsensusSearchMode::Off);
    mgr.set_iccf_search(IccfSearchMode::Off);  // isolate; default FreqOnly would still filter
    check(Certus::allow_search_move(pos, a3, false, 0), "Strict+Consensus Off allows a3");

    mgr.set_consensus_search(ConsensusSearchMode::MarkedOnly);
    check(Certus::allow_search_move(pos, nc3, false, 0), "Strict MarkedOnly allows nc3");
    check(Certus::allow_search_move(pos, nf3, false, 0), "Strict MarkedOnly allows nf3");
    check(!Certus::allow_search_move(pos, a3, false, 0), "Strict MarkedOnly blocks a3");
    check(Certus::allow_search_move(pos, a3, true, 0), "in_check allows a3");
    check(Certus::allow_search_move(pos, a3, false, 1), "pvIdx>0 allows a3");

    // FEAT-0003: ICCF frequent_moves filter under Strict.
    mgr.set_consensus_search(ConsensusSearchMode::Off);
    mgr.set_iccf_search(IccfSearchMode::Off);
    check(Certus::allow_search_move(pos, a3, false, 0), "Iccf Off allows a3 on e4e5");

    mgr.set_iccf_search(IccfSearchMode::FreqOnly);
    check(Certus::allow_search_move(pos, nf3, false, 0), "FreqOnly allows nf3");
    check(Certus::allow_search_move(pos, nc3, false, 0), "FreqOnly allows nc3");
    check(!Certus::allow_search_move(pos, a3, false, 0), "FreqOnly blocks a3 on e4e5");

    pos.set(fen_italian, false, &states->back());
    const Move bc4 = UCIEngine::to_move(pos, "f1c4");
    const Move d4  = UCIEngine::to_move(pos, "d2d4");
    const Move nc3i = UCIEngine::to_move(pos, "b1c3");
    const Move a2a3 = UCIEngine::to_move(pos, "a2a3");
    check(bc4 != Move::none() && d4 != Move::none() && nc3i != Move::none() && a2a3 != Move::none(),
          "italian uci");
    const auto freq = Certus::iccf_frequent_legal_moves(pos);
    check(freq.size() == 3, "three frequent italian");
    check(std::find(freq.begin(), freq.end(), bc4) != freq.end(), "freq bc4");
    check(!Certus::allow_search_move(pos, a2a3, false, 0), "FreqOnly blocks a3 italian");
    check(Certus::allow_search_move(pos, bc4, false, 0), "FreqOnly allows bc4");

    // FEAT-0007: Max uses same preferred set as High (marked else frequent), more effort only.
    mgr.set_certus_style(CertusStyleMode::Mixed);
    mgr.set_mixed_effort(MixedEffortMode::High);
    mgr.set_consensus_search(ConsensusSearchMode::MarkedOnly);
    mgr.set_iccf_search(IccfSearchMode::FreqOnly);
    {
        const auto fh = Certus::make_search_move_filter(pos, false, 0);
        check(fh.is_preferred(d4) && !fh.is_preferred(bc4), "High italian preferred = marked only");
    }
    mgr.set_mixed_effort(MixedEffortMode::Max);
    {
        const auto fm = Certus::make_search_move_filter(pos, false, 0);
        check(fm.is_preferred(d4) && !fm.is_preferred(bc4) && !fm.is_preferred(nc3i),
              "Max italian preferred = marked else frequent (same as High)");
        check(!fm.restrict_moves && fm.zero_lmr, "Max italian no filter + zero LMR");
        check(Certus::allow_search_move(pos, a2a3, false, 0), "Max italian still allows a3");
    }

    // FEAT-0008: Strict Priority vs Union on italian (marked + frequent both present).
    mgr.set_certus_style(CertusStyleMode::Strict);
    mgr.set_consensus_search(ConsensusSearchMode::MarkedOnly);
    mgr.set_iccf_search(IccfSearchMode::FreqOnly);
    mgr.set_strict_preferred(StrictPreferredMode::Priority);
    {
        const auto fp = Certus::make_search_move_filter(pos, false, 0);
        check(fp.restrict_moves && fp.is_preferred(d4) && !fp.is_preferred(bc4),
              "Strict Priority = marked only when marked present");
        check(!Certus::allow_search_move(pos, bc4, false, 0), "Strict Priority blocks frequent-only bc4");
    }
    mgr.set_strict_preferred(StrictPreferredMode::Union);
    {
        const auto fu = Certus::make_search_move_filter(pos, false, 0);
        check(fu.restrict_moves && fu.is_preferred(d4) && fu.is_preferred(bc4),
              "Strict Union includes marked and frequent");
        check(Certus::allow_search_move(pos, bc4, false, 0), "Strict Union allows bc4");
        check(!Certus::allow_search_move(pos, a2a3, false, 0), "Strict Union still blocks a3");
        const auto P = Certus::strict_preferred_moves(pos);
        check(P.size() >= 3, "Strict Union italian |P|>=3");
    }
    mgr.set_strict_preferred(StrictPreferredMode::Priority);

    pos.set(fen, false, &states->back());
    mgr.set_certus_style(CertusStyleMode::Strict);
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
