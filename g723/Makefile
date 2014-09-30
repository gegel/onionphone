CROSS :=

CC = $(CROSS)cc
AS = $(CC)
AR = $(CROSS)ar

CC_OPT=-fomit-frame-pointer -ffast-math -funroll-loops -O4

CFLAGS   = -Wall -pg -g -O4


ARFLAGS  := rc


LIBG723 = libg723.a

all: $(LIBG723)

LIBG723_SRC = \
	basop.c \
	coder.c\
	lsp.c\
	tab_lbc.c\
	util_lbc.c\
	dec_cng.c\
	exc_lbc.c\
	tame.c\
	vad.c\
	cod_cng.c\
	decod.c\
	lpc.c\
	util_cng.c\
	lbccodec.c



LIBG723_OBJS = $(LIBG723_SRC:.c=.o)

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(AS) $(CC_OPT) -c -o $@ $^
%.a:
	$(AR) $(ARFLAGS) $@ $^


$(LIBG723): $(LIBG723_OBJS)

clean:
	rm -f *.o $(LIBG723) 

.PHONY: all clean
