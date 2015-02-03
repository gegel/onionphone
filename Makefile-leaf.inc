# NORMAL DEBUG
MODE ?= NORMAL
MODE_NORMAL ?= -O3 -ffast-math -funroll-loops
MODE_DEBUG ?= -O0 -g -ggdb

CFLAGS = $(GENERIC_CFLAGS) $(MODE_$(MODE)) $(EXTRADEFS)

SRCS = $(wildcard *.c)
OBJ = $(SRCS:%.c=%.o)

all: build

build: $(BLOB)

$(BLOB): $(OBJ)
	@echo Merging $(notdir $(CURDIR))
	@$(LD) -r $(OBJ) -o $(BLOB)

%.o: %.c
	@echo Building $@
	@$(CC) $(CFLAGS) $(INCADD) -c $<

clean:
	rm -f $(OBJ) $(BLOB)

test:
	cppcheck --std=c99 --enable=all --force -q .
	scan-build $(MAKE)

.PHONY: build

