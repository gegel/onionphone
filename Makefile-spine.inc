BUILDDIRS = $(LEAFS:%=build-%)
CLEANDIRS = $(LEAFS:%=clean-%)
TESTDIRS = $(LEAFS:%=test-%)

all: $(BUILDDIRS)
	@echo Merging $(notdir $(CURDIR))
	@$(LD) -r $(LEAFS:%=%/builtin.o) -o $(BLOB)

$(LEAFS): $(BUILDDIRS)
$(BUILDDIRS):
	$(MAKE) -C $(@:build-%=%)

clean: $(CLEANDIRS)
	rm -f $(BLOB)
$(CLEANDIRS): 
	$(MAKE) -C $(@:clean-%=%) clean

test: $(TESTDIRS)
$(TESTDIRS): 
	$(MAKE) -C $(@:test-%=%) test

.PHONY: subdirs $(LEAFS)
.PHONY: subdirs $(BUILDDIRS)
.PHONY: subdirs $(CLEANDIRS)
.PHONY: subdirs $(TESTDIRS)
