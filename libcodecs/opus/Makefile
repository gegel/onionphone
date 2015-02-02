LEAFS = celt silk_common src

# FIXED FLOAT
export ARITHMETICS=FIXED

ifeq ($(ARITHMETICS),FIXED)
	LEAFS += silk_fixed
else
	LEAFS += silk_float
endif

include ../../Makefile-common.inc
include ../../Makefile-spine.inc

