# FIXED FLOAT
ARITHMETICS ?= FIXED

# -DHAVE_CONFIG_H -DFIXED_POINT
EXTRADEFS = -DHAVE_CONFIG_H -D$(ARITHMETICS)_POINT
INCADD ?= -I. -I../include -I../celt -I../silk_common -I../silk_fixed -I../silk_float -I../../../common/inc

include ../../../Makefile-common.inc
include ../../../Makefile-leaf.inc

