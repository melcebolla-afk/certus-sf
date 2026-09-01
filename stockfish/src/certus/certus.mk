# certus-sf Makefile overlay — included from upstream Makefile via:
#   -include certus/certus.mk
# Do not scatter certus build rules in the upstream Makefile.

ifeq ($(target_windows),yes)
EXE := certus-sf.exe
else
EXE := certus-sf
endif

VPATH := $(VPATH):evidence:certus

CERTUS_SRCS = certus_engine.cpp \
	certus_hash.cpp json_reader.cpp catalog_path.cpp evidence_root.cpp \
	consensus_store.cpp iccf_store.cpp theoretical_store.cpp mate_store.cpp \
	evidence_manager.cpp

SRCS += $(CERTUS_SRCS)

CXXFLAGS += -DCERTUS_SF

CERTUS_CLEAN = certus-sf certus-sf.exe evidence_probe

EVIDENCE_PROBE_SRCS = certus_hash.cpp json_reader.cpp catalog_path.cpp evidence_root.cpp \
	consensus_store.cpp iccf_store.cpp theoretical_store.cpp mate_store.cpp \
	evidence/evidence_probe.cpp

EVIDENCE_PROBE_OBJS = $(EVIDENCE_PROBE_SRCS:.cpp=.o) \
	bitboard.o movegen.o position.o misc.o memory.o tune.o

.PHONY: evidence_probe

evidence_probe: $(EVIDENCE_PROBE_OBJS)
	+$(CXX) -o $@ $(EVIDENCE_PROBE_OBJS) $(LDFLAGS)

evidence/evidence_probe.o: evidence/evidence_probe.cpp
	+$(CXX) $(CXXFLAGS) -c -o $@ $<
