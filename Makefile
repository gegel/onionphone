COMMONAPPS = common/crp

ADDKEYAPPS = libaddkey

CODECSAPPS = libcodecs

OPHAPPS = libdesktop

ifdef SYSTEMROOT
LDADD = -lm -lcomctl32 -lwinmm -lws2_32
EXEADD = .exe
else
LDADD = -lm -lasound
endif

COMMONBUILDDIRS = $(COMMONAPPS:%=build-%)
ADDKEYBUILDDIRS = $(ADDKEYAPPS:%=build-%)
CODECSBUILDDIRS = $(CODECSAPPS:%=build-%)
OPHBUILDDIRS = $(OPHAPPS:%=build-%)
COMMONCLEANDIRS = $(COMMONAPPS:%=clean-%)
ADDKEYCLEANDIRS = $(ADDKEYAPPS:%=clean-%)
CODECSCLEANDIRS = $(CODECSAPPS:%=clean-%)
OPHCLEANDIRS = $(OPHAPPS:%=clean-%)
COMMONTESTDIRS = $(COMMONAPPS:%=test-%)
ADDKEYTESTDIRS = $(ADDKEYAPPS:%=test-%)
CODECSTESTDIRS = $(CODECSAPPS:%=test-%)
OPHTESTDIRS = $(OPHAPPS:%=test-%)

all: $(COMMONBUILDDIRS) $(ADDKEYBUILDDIRS) $(CODECSBUILDDIRS) $(OPHBUILDDIRS)
	$(CC) $(COMMONAPPS:%=%/builtin.o) $(ADDKEYAPPS:%=%/builtin.o) -o addkey$(EXEADD)
	$(CC) $(COMMONAPPS:%=%/builtin.o) $(CODECSAPPS:%=%/builtin.o) $(OPHAPPS:%=%/builtin.o) $(LDADD) -o oph$(EXEADD)

fast: $(OPHBUILDDIRS)
	$(CC) $(COMMONAPPS:%=%/builtin.o) $(CODECSAPPS:%=%/builtin.o) $(OPHAPPS:%=%/builtin.o) $(LDADD) -o oph$(EXEADD)

$(COMMONAPPS): $(COMMONBUILDDIRS)
$(ADDKEYAPPS): $(ADDKEYBUILDDIRS)
$(CODECSAPPS): $(CODECSBUILDDIRS)
$(OPHAPPS): $(OPHBUILDDIRS)
$(COMMONBUILDDIRS):
	$(MAKE) -C $(@:build-%=%)
$(ADDKEYBUILDDIRS):
	$(MAKE) -C $(@:build-%=%)
$(CODECSBUILDDIRS):
	$(MAKE) -C $(@:build-%=%)
$(OPHBUILDDIRS):
	$(MAKE) -C $(@:build-%=%)

clean: $(COMMONCLEANDIRS) $(ADDKEYCLEANDIRS) $(CODECSCLEANDIRS) $(OPHCLEANDIRS)
$(COMMONCLEANDIRS): 
	$(MAKE) -C $(@:clean-%=%) clean
$(ADDKEYCLEANDIRS): 
	$(MAKE) -C $(@:clean-%=%) clean
	rm -f addkey
	rm -f addkey$(EXEADD)
$(CODECSCLEANDIRS): 
	$(MAKE) -C $(@:clean-%=%) clean
$(OPHCLEANDIRS): 
	$(MAKE) -C $(@:clean-%=%) clean
	rm -f oph$
	rm -f oph$(EXEADD)

test: $(COMMONTESTDIRS) $(ADDKEYTESTDIRS) $(CODECSCLEANDIRS) $(OPHTESTDIRS)
$(COMMONTESTDIRS): 
	$(MAKE) -C $(@:test-%=%) test
$(ADDKEYTESTDIRS): 
	$(MAKE) -C $(@:test-%=%) test
$(CODECSTESTDIRS): 
	$(MAKE) -C $(@:test-%=%) test
$(OPHTESTDIRS): 
	$(MAKE) -C $(@:test-%=%) test

.PHONY: subdirs $(COMMONAPPS)
.PHONY: subdirs $(ADDKEYAPPS)
.PHONY: subdirs $(CODECSAPPS)
.PHONY: subdirs $(OPHAPPS)
.PHONY: subdirs $(COMMONBUILDDIRS)
.PHONY: subdirs $(ADDKEYBUILDDIRS)
.PHONY: subdirs $(CODECSBUILDDIRS)
.PHONY: subdirs $(OPHBUILDDIRS)
.PHONY: subdirs $(COMMONCLEANDIRS)
.PHONY: subdirs $(ADDKEYCLEANDIRS)
.PHONY: subdirs $(CODECSCLEANDIRS)
.PHONY: subdirs $(OPHCLEANDIRS)
.PHONY: subdirs $(COMMONTESTDIRS)
.PHONY: subdirs $(ADDKEYTESTDIRS)
.PHONY: subdirs $(CODECSTESTDIRS)
.PHONY: subdirs $(OPHTESTDIRS)
.PHONY: all fast clean test
