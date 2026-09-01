# certus-sf Makefile overlay — included from upstream Makefile via:
#   -include certus/certus.mk
# Do not scatter certus build rules in the upstream Makefile.

ifeq ($(target_windows),yes)
EXE := certus-sf.exe
else
EXE := certus-sf
endif

VPATH := $(VPATH):evidence:certus

CERTUS_SRCS = certus_engine.cpp certus_eval.cpp certus_search.cpp \
	certus_hash.cpp json_reader.cpp catalog_path.cpp evidence_root.cpp \
	consensus_store.cpp iccf_store.cpp theoretical_store.cpp mate_store.cpp \
	evidence_manager.cpp resolver.cpp

SRCS += $(CERTUS_SRCS)

CXXFLAGS += -DCERTUS_SF

CERTUS_CLEAN = certus-sf certus-sf.exe evidence_probe golden_probe consensus_search_probe

CONSENSUS_SEARCH_PROBE_SRCS = certus_hash.cpp json_reader.cpp catalog_path.cpp evidence_root.cpp \
	consensus_store.cpp iccf_store.cpp theoretical_store.cpp mate_store.cpp evidence_manager.cpp \
	certus_eval.cpp certus_search.cpp \
	certus/consensus_search_probe.cpp

CONSENSUS_SEARCH_PROBE_OBJS = $(CONSENSUS_SEARCH_PROBE_SRCS:.cpp=.o) \
	bitboard.o movegen.o position.o misc.o memory.o tune.o uci.o \
	evaluate.o score.o nnue_accumulator.o nnue_misc.o network.o half_ka_v2_hm.o full_threats.o

EVIDENCE_PROBE_SRCS = certus_hash.cpp json_reader.cpp catalog_path.cpp evidence_root.cpp \
	consensus_store.cpp iccf_store.cpp theoretical_store.cpp mate_store.cpp \
	evidence/evidence_probe.cpp

GOLDEN_PROBE_SRCS = certus_hash.cpp json_reader.cpp catalog_path.cpp evidence_root.cpp \
	consensus_store.cpp iccf_store.cpp theoretical_store.cpp mate_store.cpp \
	evidence_manager.cpp resolver.cpp \
	evidence/golden_probe.cpp

EVIDENCE_PROBE_OBJS = $(EVIDENCE_PROBE_SRCS:.cpp=.o) \
	bitboard.o movegen.o position.o misc.o memory.o tune.o

GOLDEN_PROBE_OBJS = $(GOLDEN_PROBE_SRCS:.cpp=.o) \
	bitboard.o movegen.o position.o misc.o memory.o tune.o tbprobe.o uci.o

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
