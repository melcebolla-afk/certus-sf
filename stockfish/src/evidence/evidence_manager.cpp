#include "evidence_manager.h"

#include "catalog_path.h"
#include "evidence_root.h"

#include <sstream>

namespace Stockfish::Evidence {

namespace {

EvidenceInfoMode parse_evidence_info(const std::string& s) {
    std::string t;
    for (char c : s)
        t += char(std::tolower(static_cast<unsigned char>(c)));
    if (t == "off")
        return EvidenceInfoMode::Off;
    if (t == "all")
        return EvidenceInfoMode::All;
    return EvidenceInfoMode::Root;
}

ConsensusSearchMode parse_consensus_search(const std::string& s) {
    std::string t;
    for (char c : s)
        t += char(std::tolower(static_cast<unsigned char>(c)));
    if (t == "markedonly" || t == "marked_only")
        return ConsensusSearchMode::MarkedOnly;
    return ConsensusSearchMode::Off;
}

IccfSearchMode parse_iccf_search(const std::string& s) {
    std::string t;
    for (char c : s)
        t += char(std::tolower(static_cast<unsigned char>(c)));
    if (t == "freqonly" || t == "freq_only" || t == "frequentonly")
        return IccfSearchMode::FreqOnly;
    return IccfSearchMode::Off;
}

}  // namespace

std::optional<std::string> Manager::ready_line(const char* label, bool ready,
                                               const std::string& version, size_t entries) const {
    if (!ready)
        return std::nullopt;
    std::ostringstream ss;
    ss << label << " ready version=" << version << " entries=" << entries;
    return ss.str();
}

std::optional<std::string> Manager::apply_layer_path(const char* label,
                                                     std::string (Manager::*reload)(
                                                       const std::string&),
                                                     const std::string& path) {
    const std::string err = (this->*reload)(path);
    if (!err.empty())
        return std::string("warning ") + label + ": " + err;
    if (on_reload_)
        on_reload_();
    return std::nullopt;
}

std::string Manager::reload_consensus(const std::string& path) {
    const std::string err = consensus_.load(path);
    return err;
}

std::string Manager::reload_iccf(const std::string& path) {
    return iccf_.load(path);
}

std::string Manager::reload_theoretical(const std::string& path) {
    return theoretical_.load(path);
}

std::string Manager::reload_mate(const std::string& path) {
    return mate_.load(path);
}

std::string Manager::reload_evidence_root(const std::string& path) {
    evidence_root_ = path;
    if (path.empty())
        return {};
    if (!is_directory(path))
        return "EvidencePath: not a directory: " + path;
    return {};
}

std::optional<Wdl> Manager::probe_theoretical(const Position& pos) const {
    bool hit = false;
    const Wdl r = theoretical_.probe(pos, &hit);
    return hit ? std::optional<Wdl>{r} : std::nullopt;
}

void Manager::register_options(OptionsMap& options, std::function<void()> on_reload) {
    on_reload_ = std::move(on_reload);

    auto layer_cb = [this](const char* label, std::string (Manager::*reload)(const std::string&)) {
        return [this, label, reload](const Option& o) -> std::optional<std::string> {
            const std::string path = o;
            if (auto w = apply_layer_path(label, reload, path))
                return *w;

            if (path.empty())
                return std::optional<std::string>{std::string(label) + " cleared"};

            if (label == std::string("ConsensusPath"))
                return ready_line(label, consensus_.ready(), consensus_.content_version(),
                                  consensus_.size());
            if (label == std::string("IccfPath"))
                return ready_line(label, iccf_.ready(), iccf_.content_version(), iccf_.size());
            if (label == std::string("TheoreticalPath"))
                return ready_line(label, theoretical_.ready(), theoretical_.content_version(),
                                  theoretical_.size());
            if (label == std::string("MatePath"))
                return ready_line(label, mate_.ready(), mate_.content_version(), mate_.size());
            return std::nullopt;
        };
    };

    options.add("TheoreticalPath",
                Option("", layer_cb("TheoreticalPath", &Manager::reload_theoretical)));
    options.add("ConsensusPath", Option("", layer_cb("ConsensusPath", &Manager::reload_consensus)));
    options.add("IccfPath", Option("", layer_cb("IccfPath", &Manager::reload_iccf)));
    options.add("MatePath", Option("", layer_cb("MatePath", &Manager::reload_mate)));

    options.add("EvidencePath", Option("", [this](const Option& o) -> std::optional<std::string> {
        const std::string path = o;
        const std::string err  = reload_evidence_root(path);
        if (!err.empty())
            return std::string("warning ") + err;
        if (path.empty())
            return std::optional<std::string>{"EvidencePath cleared (per-layer paths unchanged)"};

        std::ostringstream out;
        out << "EvidencePath root=" << path;

        auto load_via_root = [&](const char* layer, const char* label,
                                 std::string (Manager::*reload)(const std::string&)) {
            if (auto dir = resolve_layer(path, layer))
            {
                if (auto w = apply_layer_path(label, reload, *dir))
                    out << "\nwarning " << label << " via EvidencePath: "
                        << w->substr(w->find(':') + 2);
                else if (label == std::string("ConsensusPath") && consensus_.ready())
                    out << "\nConsensusPath ready version=" << consensus_.content_version()
                        << " entries=" << consensus_.size() << " (via EvidencePath)";
                else if (label == std::string("IccfPath") && iccf_.ready())
                    out << "\nIccfPath ready version=" << iccf_.content_version()
                        << " entries=" << iccf_.size() << " (via EvidencePath)";
                else if (label == std::string("TheoreticalPath") && theoretical_.ready())
                    out << "\nTheoreticalPath ready version=" << theoretical_.content_version()
                        << " entries=" << theoretical_.size() << " (via EvidencePath)";
                else if (label == std::string("MatePath") && mate_.ready())
                    out << "\nMatePath ready version=" << mate_.content_version()
                        << " entries=" << mate_.size() << " (via EvidencePath)";
            }
            else
                out << "\nwarning EvidencePath: no versioned " << layer << "/ with catalog.json";
        };

        load_via_root("consensus", "ConsensusPath", &Manager::reload_consensus);
        load_via_root("iccf", "IccfPath", &Manager::reload_iccf);
        load_via_root("theoretical", "TheoreticalPath", &Manager::reload_theoretical);
        load_via_root("mate", "MatePath", &Manager::reload_mate);

        return out.str();
    }));

    options.add("EvidenceInfo",
                Option("Off Root All", "Root", [this](const Option& o) -> std::optional<std::string> {
                    evidence_info_ = parse_evidence_info(std::string(o));
                    return std::nullopt;
                }));

    options.add("ConsensusSearch",
                Option("Off MarkedOnly", "MarkedOnly", [this](const Option& o) -> std::optional<std::string> {
                    consensus_search_ = parse_consensus_search(std::string(o));
                    return std::nullopt;
                }));

    options.add("IccfSearch",
                Option("Off FreqOnly", "Off", [this](const Option& o) -> std::optional<std::string> {
                    iccf_search_ = parse_iccf_search(std::string(o));
                    return std::nullopt;
                }));
}

}  // namespace Stockfish::Evidence
