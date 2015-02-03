include Makefile-common.inc

TARGETS = addkey oph

addkey_DEPS = common/crp libaddkey
oph_DEPS = common/crp common/helpers common/libspeexdsp common/kiss_fft libcodecs libdesktop

addkey_LDADD =
oph_LDADD = -lm
ifdef SYSTEMROOT
oph_LDADD += -lcomctl32 -lwinmm -lws2_32
else
oph_LDADD += -lasound
endif

ifdef SYSTEMROOT
addkey_EXEADD = .exe
oph_EXEADD = .exe
endif

ifdef SYSTEMROOT
LDADD = -lm -lcomctl32 -lwinmm -lws2_32
EXEADD = .exe
else
LDADD = -lm -lasound
endif

%.target-build:
	$(foreach i,$($(@:%.target-build=%)_DEPS),$(MAKE) -C $(i);)
	$(CC) $(GENERIC_CFLAGS) $(foreach i,$($(@:%.target-build=%)_DEPS),$(i)/builtin.o) $($(@:%.target-build=%)_LDADD) -o $(@:%.target-build=%)$($(@:%.target-build=%)_EXEADD)

%.target-clean:
	$(foreach i,$($(@:%.target-clean=%)_DEPS),$(MAKE) -C $(i) clean;)

%.target-test:
	$(foreach i,$($(@:%.target-test=%)_DEPS),$(MAKE) -C $(i) test;)

all:
	$(foreach i,$(TARGETS),$(MAKE) $(i).target-build;)

clean:
	$(foreach i,$(TARGETS),$(MAKE) $(i).target-clean;)
	$(foreach i,$(TARGETS),rm -f $(i)$($(i)_EXEADD);)

test:
	$(foreach i,$(TARGETS),$(MAKE) $(i).target-test;)

