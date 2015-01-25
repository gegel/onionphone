/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*---------------------------------------------------------------------------*\
                                                                             
  FILE........: dump.h
  AUTHOR......: David Rowe                                                          
  DATE CREATED: 25/8/09                                                       
                                                                             
  Routines to dump data to text files for Octave analysis.

\*---------------------------------------------------------------------------*/

/*
  All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 2.1, as
  published by the Free Software Foundation.  This program is
  distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __DUMP__
#define __DUMP__

#include "defines.h"
#include "comp.h"
#include "kiss_fft.h"
#include "codec2_internal.h"

/* amplitude modelling */

void dump_model(MODEL * m);
void dump_quantised_model(MODEL * m);
void dump_Pwn(COMP Pw[]);
void dump_Pw(COMP Pw[]);
void dump_Rw(float Rw[]);
void dump_lsp_(float lsp_[]);
void dump_ak_(float ak[], int order);

/* NLP states */

void dump_sq(float sq[]);
void dump_dec(COMP Fw[]);
void dump_Fw(COMP Fw[]);
void dump_e(float e_hz[]);

/* post filter */

void dump_bg(float e, float bg_est, float percent_uv);
void dump_Pwb(COMP Pwb[]);

#endif
