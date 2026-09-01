/*
  certus-sf — GOLDEN-FIXTURES.jsonl resolver regression (Phase 2).
  Build: make -C stockfish/src golden_probe
  Run:   ./stockfish/src/golden_probe [repo_root]
*/

#include "../bitboard.h"
#include "../position.h"
#include "../syzygy/tbprobe.h"

#include "consensus_store.h"
#include "evidence_manager.h"
#include "iccf_store.h"
#include "mate_store.h"
#include "resolver.h"
#include "theoretical_store.h"

#include <cstdlib>
#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace Stockfish;
using namespace Stockfish::Evidence;

namespace {

int failures = 0;
int skipped  = 0;

void check(bool ok, const std::string& msg) {
    if (!ok)
    {
        std::cerr << "FAIL: " << msg << '\n';
        ++failures;
    }
}

void skip(const std::string& msg) {
    std::cerr << "SKIP: " << msg << '\n';
    ++skipped;
}

std::string repo_path(const std::string& root, const std::string& rel) {
    if (rel.empty())
        return {};
    if (rel[0] == '/')
        return rel;
    return root + "/" + rel;
}

std::optional<std::string> extract_quoted(const std::string& line, const std::string& key) {
    const std::string needle = "\"" + key + "\":\"";
    const auto        p      = line.find(needle);
    if (p == std::string::npos)
        return std::nullopt;
    const auto start = p + needle.size();
    const auto end   = line.find('"', start);
    if (end == std::string::npos)
        return std::nullopt;
    return line.substr(start, end - start);
}

std::optional<bool> extract_bool(const std::string& line, const std::string& key) {
    const std::string needle = "\"" + key + "\":";
    const auto        p      = line.find(needle);
    if (p == std::string::npos)
        return std::nullopt;
    const auto tail = line.substr(p + needle.size());
    if (tail.rfind("true", 0) == 0)
        return true;
    if (tail.rfind("false", 0) == 0)
        return false;
    return std::nullopt;
}

std::string layer_path(const std::string& line, const char* layer) {
    if (auto v = extract_quoted(line, layer))
        return *v;
    return {};
}

struct FixtureStores {
    Manager manager;
    std::string syzygy_path;
};

FixtureStores load_layers(const std::string& root, const std::string& line) {
    FixtureStores fs;
    const auto    join = [&](const std::string& rel) {
        return rel.empty() ? std::string{} : repo_path(root, rel);
    };

    if (const std::string p = join(layer_path(line, "consensus")); !p.empty())
        fs.manager.reload_consensus(p);
    if (const std::string p = join(layer_path(line, "iccf")); !p.empty())
        fs.manager.reload_iccf(p);
    if (const std::string p = join(layer_path(line, "theoretical")); !p.empty())
        fs.manager.reload_theoretical(p);
    if (const std::string p = join(layer_path(line, "mate")); !p.empty())
        fs.manager.reload_mate(p);

    fs.syzygy_path = join(layer_path(line, "syzygy"));
    return fs;
}

void run_fixture(const std::string& root, const std::string& line) {
    const auto id    = extract_quoted(line, "id");
    const auto fen   = extract_quoted(line, "fen");
    const auto probe = extract_quoted(line, "probe");
    const auto exp   = extract_quoted(line, "expected_class");
    const auto notes = extract_quoted(line, "notes");

    if (!id || !fen || !probe || !exp)
        return;

    if (*probe == "root_only")
    {
        skip(*id + " (root_only — Fase 3)");
        return;
    }

    if (*id == "tb_krk_win")
    {
        const std::string tb = repo_path(root, layer_path(line, "syzygy"));
        Tablebases::init(tb);
        if (Tablebases::MaxCardinality <= 0)
        {
            skip(*id + ": syzygy3 missing");
            return;
        }
    }
    else
    {
        Tablebases::init("");
    }

    FixtureStores fs = load_layers(root, line);

    StateListPtr states(new std::deque<StateInfo>(1));
    Position       pos;
    pos.set(*fen, false, &states->back());

    EvalContext ctx;
    ctx.manager        = &fs.manager;
    ctx.syzygy_loaded  = Tablebases::MaxCardinality > 0;
    ctx.tb_cardinality = Tablebases::MaxCardinality;

    const bool hard = true;
    const bool soft = (*probe != "hard_only_thin");
    EvalResult result = evaluate_layers(pos, ctx, hard, soft);

    const EvidenceClass expected = evidence_class_from_name(*exp);
    check(result.evidence_class == expected,
          *id + " class expected " + *exp + " got " + evidence_class_name(result.evidence_class));

    if (auto hard_exp = extract_bool(line, "expected_hard_evidence"))
        check(result.hard_evidence() == *hard_exp,
              *id + " hard_evidence expected " + (*hard_exp ? "true" : "false"));

    if (notes && notes->find("mate fixture absent") != std::string::npos && !fs.manager.mate().ready())
        skip(*id + ": mate fixture absent");
}

}  // namespace

int main(int argc, char** argv) {
    Bitboards::init();
    Position::init();

    const std::string root = (argc > 1) ? argv[1] : "../..";
    const std::string file = root + "/docs/bootstrap/GOLDEN-FIXTURES.jsonl";

    std::ifstream in(file);
    if (!in)
    {
        std::cerr << "golden_probe: cannot open " << file << '\n';
        return EXIT_FAILURE;
    }

    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        run_fixture(root, line);
    }

    if (failures == 0)
        std::cout << "golden_probe: all tests passed (" << skipped << " skipped)\n";
    else
        std::cerr << "golden_probe: " << failures << " failure(s), " << skipped << " skipped\n";

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
