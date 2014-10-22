BLOB = builtin.o
APPS = celt silk_common silk_fixed silk_float src

LD ?= ld

BUILDDIRS = $(APPS:%=build-%)
CLEANDIRS = $(APPS:%=clean-%)
TESTDIRS = $(APPS:%=test-%)

all: $(BUILDDIRS)
	$(LD) -r */builtin.o -o $(BLOB)

$(APPS): $(BUILDDIRS)
$(BUILDDIRS):
	$(MAKE) -C $(@:build-%=%)

clean: $(CLEANDIRS)
	rm -f $(BLOB)
$(CLEANDIRS): 
	$(MAKE) -C $(@:clean-%=%) clean

test: $(TESTDIRS)
$(TESTDIRS): 
	$(MAKE) -C $(@:test-%=%) test

.PHONY: subdirs $(APPS)
.PHONY: subdirs $(BUILDDIRS)
.PHONY: subdirs $(CLEANDIRS)
.PHONY: subdirs $(TESTDIRS)
.PHONY: all clean test
