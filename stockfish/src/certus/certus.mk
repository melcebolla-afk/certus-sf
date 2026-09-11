# certus-sf Makefile overlay — included from upstream Makefile via:
#   -include certus/certus.mk
# Do not scatter certus build rules in the upstream Makefile.
#
# Source paths MUST be directory-prefixed (certus/…, evidence/…).
# Universal builds symlink TRACKED_TOP = first path component of SRCS/HEADERS
# into temp_builds/<arch>/; flat names would miss the overlay dirs and break
# #include "certus/…" / -include certus/certus.mk under profile-build ARCH=*-universal.

ifeq ($(target_windows),yes)
EXE := certus-sf.exe
else
EXE := certus-sf
endif

VPATH := $(VPATH):evidence:certus

CERTUS_SRCS = certus/certus_engine.cpp certus/certus_eval.cpp certus/certus_search.cpp \
	evidence/certus_hash.cpp evidence/json_reader.cpp evidence/catalog_path.cpp \
	evidence/evidence_root.cpp evidence/consensus_store.cpp evidence/iccf_store.cpp \
	evidence/theoretical_store.cpp evidence/mate_store.cpp evidence/evidence_manager.cpp \
	evidence/resolver.cpp

SRCS += $(CERTUS_SRCS)

CXXFLAGS += -DCERTUS_SF

CERTUS_CLEAN = certus-sf certus-sf.exe evidence_probe golden_probe consensus_search_probe \
	./certus/*.o

CONSENSUS_SEARCH_PROBE_SRCS = evidence/certus_hash.cpp evidence/json_reader.cpp \
	evidence/catalog_path.cpp evidence/evidence_root.cpp \
	evidence/consensus_store.cpp evidence/iccf_store.cpp evidence/theoretical_store.cpp \
	evidence/mate_store.cpp evidence/evidence_manager.cpp \
	certus/certus_eval.cpp certus/certus_search.cpp \
	certus/consensus_search_probe.cpp

CONSENSUS_SEARCH_PROBE_OBJS = $(notdir $(CONSENSUS_SEARCH_PROBE_SRCS:.cpp=.o)) \
	bitboard.o movegen.o position.o misc.o memory.o tune.o uci.o \
	evaluate.o score.o nnue_accumulator.o nnue_misc.o network.o \
	half_ka_v2_hm.o full_threats.o pp_3wide.o attacks.o

EVIDENCE_PROBE_SRCS = evidence/certus_hash.cpp evidence/json_reader.cpp \
	evidence/catalog_path.cpp evidence/evidence_root.cpp \
	evidence/consensus_store.cpp evidence/iccf_store.cpp evidence/theoretical_store.cpp \
	evidence/mate_store.cpp \
	evidence/evidence_probe.cpp

GOLDEN_PROBE_SRCS = evidence/certus_hash.cpp evidence/json_reader.cpp \
	evidence/catalog_path.cpp evidence/evidence_root.cpp \
	evidence/consensus_store.cpp evidence/iccf_store.cpp evidence/theoretical_store.cpp \
	evidence/mate_store.cpp evidence/evidence_manager.cpp evidence/resolver.cpp \
	evidence/golden_probe.cpp

EVIDENCE_PROBE_OBJS = $(notdir $(EVIDENCE_PROBE_SRCS:.cpp=.o)) \
	bitboard.o movegen.o position.o misc.o memory.o tune.o attacks.o

GOLDEN_PROBE_OBJS = $(notdir $(GOLDEN_PROBE_SRCS:.cpp=.o)) \
	bitboard.o movegen.o position.o misc.o memory.o tune.o tbprobe.o uci.o attacks.o

.PHONY: evidence_probe golden_probe consensus_search_probe

consensus_search_probe: $(CONSENSUS_SEARCH_PROBE_OBJS)
	+$(CXX) -o $@ $(CONSENSUS_SEARCH_PROBE_OBJS) $(LDFLAGS)

certus/consensus_search_probe.o: certus/consensus_search_probe.cpp
	+$(CXX) $(CXXFLAGS) -c -o $@ $<

evidence_probe: $(EVIDENCE_PROBE_OBJS)
	+$(CXX) -o $@ $(EVIDENCE_PROBE_OBJS) $(LDFLAGS)

golden_probe: $(GOLDEN_PROBE_OBJS)
	+$(CXX) -o $@ $(GOLDEN_PROBE_OBJS) $(LDFLAGS)

evidence/evidence_probe.o: evidence/evidence_probe.cpp
	+$(CXX) $(CXXFLAGS) -c -o $@ $<

evidence/golden_probe.o: evidence/golden_probe.cpp
	+$(CXX) $(CXXFLAGS) -c -o $@ $<
