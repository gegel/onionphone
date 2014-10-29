BLOB = builtin.o

APPS = celt silk_common src

# FIXED FLOAT
export ARITHMETICS=FLOAT

ifeq ($(ARITHMETICS),FIXED)
	APPS += silk_fixed
else
	APPS += silk_float
endif

LD ?= ld

BUILDDIRS = $(APPS:%=build-%)
CLEANDIRS = $(APPS:%=clean-%)
TESTDIRS = $(APPS:%=test-%)

all: $(BUILDDIRS)
	$(LD) -r $(APPS:%=%/builtin.o) -o $(BLOB)

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
