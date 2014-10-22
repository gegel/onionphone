APPS = common/crp \
       libaddkey \
       libcodecs \
       libdesktop

ifdef SYSTEMROOT
LDADD = -lm -lcomctl32 -lwinmm -lws2_32
EXEADD = .exe
else
LDADD = -lm -lasound
endif

BUILDDIRS = $(APPS:%=build-%)
CLEANDIRS = $(APPS:%=clean-%)
TESTDIRS = $(APPS:%=test-%)

all: $(BUILDDIRS)
	$(CC) libaddkey/builtin.o common/crp/builtin.o -o addkey$(EXEADD)
	$(CC) libdesktop/builtin.o libcodecs/builtin.o common/crp/builtin.o $(LDADD) -o oph$(EXEADD)

$(APPS): $(BUILDDIRS)
$(BUILDDIRS):
	$(MAKE) -C $(@:build-%=%)

clean: $(CLEANDIRS)
	rm -f addkey$(EXEADD)
	rm -f oph$(EXEADD)
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
