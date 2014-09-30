/*
**
** File:    util_lbc.c
**
** Description: utility functions for the lbc codec
**
** Functions:
**
**  I/O functions:
**
**      Read_lbc()
**      Write_lbc()
**
**  High-pass filtering:
**
**      Rem_Dc()
**
**  Miscellaneous signal processing functions:
**
**      Vec_Norm()
**      Mem_Shift()
**      Comp_En()
**      Scale()
**
**  Bit stream packing/unpacking:
**
**      Line_Pack()
**      Line_Unpk()
**
**  Mathematical functions:
**
**      Sqrt_lbc()
**      Rand_lbc()
*/

/*
	ITU-T G.723.1 Software Package Release 2 (June 2006)
    
    ITU-T G.723.1 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/


#include <stdlib.h>
#include <stdio.h>

#include "g723_const.h"
extern Flag  UseHp;
extern  CODSTATDEF  CodStat  ;
extern DECSTATDEF  DecStat  ;
extern enum  Crate    WrkRate;

extern void    Line_Pack( LINEDEF *Line, char *Vout, Word16 Ftyp );
extern Word32  Ser2Par( Word16 **Pnt, int Count );
extern Word16 g723_add(Word16 var1, Word16 var2);     /* Short add,           1 */


extern Word32 L_g723_mult(Word16 var1, Word16 var2);  /* Long mult,           1 */
extern Word32 g723_L_mac(Word32 L_var3, Word16 var1, Word16 var2); /* Mac,    1 */
extern Word32 L_mls( Word32, Word16 ) ;        /* Wght ?? */
extern Word32 L_g723_add(Word32 L_var1, Word32 L_var2);   /* Long add,        2 */
extern Word16 round_(Word32 L_var1);              /* Round,               1 */
extern Word16 g723_mult_r(Word16 var1, Word16 var2);  /* Mult with round,     2 */
extern Word16 g723_mac_r(Word32 L_var3, Word16 var1, Word16 var2);/* Mac with rounding,*/
extern Word16 g723_msu_r(Word32 L_var3, Word16 var1, Word16 var2);/* Msu with rounding,*/
extern Word16 g723_shr(Word16 var1, Word16 var2);     /* Short shift right,   1 */
extern Word32 L_g723_shr(Word32 L_var1, Word16 var2); /* Long shift right,    2 */
extern Word16 g723_abs_s(Word16 var1);                /* Short abs,           1 */
extern Word16 g723_norm_s(Word16 var1);             /* Short norm,           15 */
//extern Word32 L_g723_shr(Word32 L_var1, Word16 var2); /* Long shift right,    2 */
extern Word16 g723_extract_l(Word32 L_var1);          /* Extract low,         1 */
extern Word16 g723_sub(Word16 var1, Word16 var2);     /* Short sub,           1 */
extern Word16* Par2Ser( Word32 Inp, Word16 *Pnt, int BitNum );
extern Word16 g723_norm_l(Word32 L_var1);           /* Long norm,            30 */
extern Word32 L_g723_shl(Word32 L_var1, Word16 var2); /* Long shift left,     2 */
extern Word16 g723_extract_h(Word32 L_var1);          /* Extract high,        1 */
extern Word16 div_l( Word32, Word16 ) ;
extern Word32 g723_L_deposit_h(Word16 var1);       /* 16 bit var1 -> MSB,     2 */
extern Word32 g723_L_msu(Word32 L_var3, Word16 var1, Word16 var2); /* Msu,    1 */
extern Word32 g723_L_deposit_l(Word16 var1);       /* 16 bit var1 -> LSB,     2 */
/*
**
** Function:        Read_lbc()
**
** Description:     Read in a file
**
** Links to text:   Sections 2.2 & 4
**
** Arguments:
**
**  Word16 *Dpnt
**  int     Len
**  FILE *Fp
**
** Outputs:
**
**  Word16 *Dpnt
**
** Return value:    None
**
*/
void  Read_lbc( Word16 *Dpnt, int Len, FILE *Fp )
{
    int   i  ;

    for ( i = 0 ; i < Len ; i ++ )
        Dpnt[i] = (Word16) 0 ;

    fread ( (char *)Dpnt, sizeof(Word16), Len, Fp ) ;

    return;
}

/*
**
** Function:        Write_lbc()
**
** Description:     Write a file
**
** Links to text:   Section
**
** Arguments:
**
**  Word16 *Dpnt
**  int     Len
**  FILE *Fp
**
** Outputs:         None
**
** Return value:    None
**
*/
void    Write_lbc( Word16 *Dpnt, int Len, FILE *Fp )
{
    fwrite( (char *)Dpnt, sizeof(Word16), Len, Fp ) ;
}

void    Line_Wr( char *Line, FILE *Fp )
{
    Word16  Info ;
    int     Size   ;

    Info = Line[0] & (Word16)0x0003 ;

    /* Check frame type and rate informations */
    switch (Info) {

        case 0x0002 : {   /* SID frame */
            Size  = 4;
            break;
        }

        case 0x0003 : {  /* untransmitted silence frame */
            Size  = 1;
            break;
        }

        case 0x0001 : {   /* active frame, low rate */
            Size  = 20;
            break;
        }

        default : {  /* active frame, high rate */
            Size  = 24;
        }
    }
    fwrite( Line, Size , 1, Fp ) ;
}

int Line_Rd( char *Line, FILE *Fp )
{
    Word16  Info ;
    int     Size   ;

    if(fread( Line, 1,1, Fp ) != 1) return(-1);

    Info = Line[0] & (Word16)0x0003 ;

    /* Check frame type and rate informations */
    switch(Info) {

        /* Active frame, high rate */
        case 0 : {
            Size  = 23;
            break;
        }

        /* Active frame, low rate */
        case 1 : {
            Size  = 19;
            break;
        }

        /* Sid Frame */
        case 2 : {
            Size  = 3;
            break;
        }

        /* untransmitted */
        default : {
            return(0);
        }
    }
    fread( &Line[1], Size , 1, Fp ) ;
    return(0);
}

/*
**
** Function:        Rem_Dc()
**
** Description:     High-pass filtering
**
** Links to text:   Section 2.3
**
** Arguments:
**
**  Word16 *Dpnt
**
** Inputs:
**
**  CodStat.HpfZdl  FIR filter memory from previous frame (1 word)
**  CodStat.HpfPdl  IIR filter memory from previous frame (1 word)
**
** Outputs:
**
**  Word16 *Dpnt
**
** Return value:    None
**
*/
void  Rem_Dc( Word16 *Dpnt )
{
    int   i  ;


    Word32   Acc0,Acc1 ;

    if ( UseHp ) {
        for ( i = 0 ; i < Frame ; i ++ ) {

            /* Do the Fir and scale by 2 */
            Acc0 = L_g723_mult( Dpnt[i], (Word16) 0x4000 ) ;
            Acc0 = g723_L_mac ( Acc0, CodStat.HpfZdl, (Word16) 0xc000 ) ;
            CodStat.HpfZdl = Dpnt[i] ;

            /* Do the Iir part */
            Acc1 = L_mls( CodStat.HpfPdl, (Word16) 0x7f00 ) ;
            Acc0 = L_g723_add( Acc0, Acc1 ) ;
            CodStat.HpfPdl = Acc0 ;
            Dpnt[i] = round_(Acc0) ;
        }
    }
    else {
        for ( i = 0 ; i < Frame ; i ++ )
            Dpnt[i] = g723_shr( Dpnt[i], (Word16) 1 ) ;
    }

    return;
}

/*
**
** Function:        Vec_Norm()
**
** Description:     Vector normalization
**
** Links to text:
**
** Arguments:
**
**  Word16 *Vect
**  Word16 Len
**
** Outputs:
**
**  Word16 *Vect
**
** Return value:  The power of 2 by which the data vector multiplyed.
**
*/
Word16  Vec_Norm( Word16 *Vect, Word16 Len )
{
    int   i  ;

    Word16  Acc0,Acc1   ;
    Word16  Exp   ;
    Word16  Rez ;
    Word32  Temp  ;

    static   short ShiftTable[16] = {
      0x0001 ,
      0x0002 ,
      0x0004 ,
      0x0008 ,
      0x0010 ,
      0x0020 ,
      0x0040 ,
      0x0080 ,
      0x0100 ,
      0x0200 ,
      0x0400 ,
      0x0800 ,
      0x1000 ,
      0x2000 ,
      0x4000 ,
      0x7fff
   } ;

    /* Find absolute maximum */
    Acc1 = (Word16) 0 ;
    for ( i = 0 ; i < Len ; i ++ ) {
        Acc0 = g723_abs_s( Vect[i] ) ;
        if ( Acc0 > Acc1 )
            Acc1 = Acc0 ;
    }

    /* Get the shift count */
    Rez = g723_norm_s( Acc1 ) ;
    Exp = ShiftTable[Rez] ;

    /* Normalize all the vector */
    for ( i = 0 ; i < Len ; i ++ ) {
        Temp = L_g723_mult( Exp, Vect[i] ) ;
        Temp = L_g723_shr( Temp, 4 ) ;
        Vect[i] = g723_extract_l( Temp ) ;
    }

    Rez = g723_sub( Rez, (Word16) 3) ;
    return Rez ;
}

/*
**
** Function:        Mem_Shift()
**
** Description:     Memory shift, update of the high-passed input speech signal
**
** Links to text:
**
** Arguments:
**
**  Word16 *PrevDat
**  Word16 *DataBuff
**
** Outputs:
**
**  Word16 *PrevDat
**  Word16 *DataBuff
**
** Return value:    None
**
*/
void  Mem_Shift( Word16 *PrevDat, Word16 *DataBuff )
{
    int   i  ;

    Word16   Dpnt[Frame+LpcFrame-SubFrLen] ;

    /*  Form Buffer  */
    for ( i = 0 ; i < LpcFrame-SubFrLen ; i ++ )
        Dpnt[i] = PrevDat[i] ;
    for ( i = 0 ; i < Frame ; i ++ )
        Dpnt[i+LpcFrame-SubFrLen] = DataBuff[i] ;

    /* Update PrevDat */
    for ( i = 0 ; i < LpcFrame-SubFrLen ; i ++ )
        PrevDat[i] = Dpnt[Frame+i] ;

    /* Update DataBuff */
    for ( i = 0 ; i < Frame ; i ++ )
        DataBuff[i] = Dpnt[(LpcFrame-SubFrLen)/2+i] ;

    return;
}

/*
**
** Function:        Line_Pack()
**
** Description:     Packing coded parameters in bitstream of 16-bit words
**
** Links to text:   Section 4
**
** Arguments:
**
**  LINEDEF *Line     Coded parameters for a frame
**  char    *Vout     bitstream chars
**  Word16   VadBit   Voice Activity Indicator
**
** Outputs:
**
**  Word16 *Vout
**
** Return value:    None
**
*/
void    Line_Pack( LINEDEF *Line, char *Vout, Word16 Ftyp )
{
    int     i ;
    int     BitCount ;

    Word16  BitStream[192] ;
    Word16 *Bsp = BitStream ;
    Word32  Temp ;

    /* Clear the output vector */
    for ( i = 0 ; i < 24 ; i ++ )
        Vout[i] = 0 ;

 /*
  * Add the coder rate info and frame type info to the 2 msb
  * of the first word of the frame.
  * The signaling is as follows:
  *     Ftyp  WrkRate => X1X0
  *       1     Rate63     00  :   High Rate
  *       1     Rate53     01  :   Low  Rate
  *       2       x        10  :   Silence Insertion Descriptor frame
  *       0       x        11  :   Used only for simulation of
  *                                 untransmitted silence frames
  */
    switch (Ftyp) {

        case 0 : {
            Temp = 0x00000003L;
            break;
        }

        case 2 : {
            Temp = 0x00000002L;
            break;
        }

        default : {
            if ( WrkRate == Rate63 )
                Temp = 0x00000000L ;
            else
                Temp = 0x00000001L ;
            break;
        }
    }

    /* Serialize Control info */
    Bsp = Par2Ser( Temp, Bsp, 2 ) ;

    

    /* Check for Speech/NonSpeech case */
    if ( Ftyp == 1 ) {

    /* 24 bit LspId */
    Temp = (*Line).LspId ;
    Bsp = Par2Ser( Temp, Bsp, 24 ) ;

 /*
  * Do the part common to both rates
  */

        /* Adaptive code book lags */
        Temp = (Word32) (*Line).Olp[0] - (Word32) PitchMin ;
        Bsp = Par2Ser( Temp, Bsp, 7 ) ;

        Temp = (Word32) (*Line).Sfs[1].AcLg ;
        Bsp = Par2Ser( Temp, Bsp, 2 ) ;

        Temp = (Word32) (*Line).Olp[1] - (Word32) PitchMin ;
        Bsp = Par2Ser( Temp, Bsp, 7 ) ;

        Temp = (Word32) (*Line).Sfs[3].AcLg ;
        Bsp = Par2Ser( Temp, Bsp, 2 ) ;

        /* Write combined 12 bit index of all the gains */
        for ( i = 0 ; i < SubFrames ; i ++ ) {
            Temp = (*Line).Sfs[i].AcGn*NumOfGainLev + (*Line).Sfs[i].Mamp ;
            if ( WrkRate == Rate63 )
                Temp += (Word32) (*Line).Sfs[i].Tran << 11 ;
            Bsp = Par2Ser( Temp, Bsp, 12 ) ;
        }

        /* Write all the Grid indices */
        for ( i = 0 ; i < SubFrames ; i ++ )
            *Bsp ++ = (*Line).Sfs[i].Grid ;

        /* High rate only part */
        if ( WrkRate == Rate63 ) {

            /* Write the reserved bit as 0 */
            *Bsp ++ = (Word16) 0 ;

            /* Write 13 bit combined position index */
            Temp = (*Line).Sfs[0].Ppos >> 16 ;
            Temp = Temp * 9 + ( (*Line).Sfs[1].Ppos >> 14) ;
            Temp *= 90 ;
            Temp += ((*Line).Sfs[2].Ppos >> 16) * 9 + ( (*Line).Sfs[3].Ppos >> 14 ) ;
            Bsp = Par2Ser( Temp, Bsp, 13 ) ;

            /* Write all the pulse positions */
            Temp = (*Line).Sfs[0].Ppos & 0x0000ffffL ;
            Bsp = Par2Ser( Temp, Bsp, 16 ) ;

            Temp = (*Line).Sfs[1].Ppos & 0x00003fffL ;
            Bsp = Par2Ser( Temp, Bsp, 14 ) ;

            Temp = (*Line).Sfs[2].Ppos & 0x0000ffffL ;
            Bsp = Par2Ser( Temp, Bsp, 16 ) ;

            Temp = (*Line).Sfs[3].Ppos & 0x00003fffL ;
            Bsp = Par2Ser( Temp, Bsp, 14 ) ;

            /* Write pulse amplitudes */
            Temp = (Word32) (*Line).Sfs[0].Pamp ;
            Bsp = Par2Ser( Temp, Bsp, 6 ) ;

            Temp = (Word32) (*Line).Sfs[1].Pamp ;
            Bsp = Par2Ser( Temp, Bsp, 5 ) ;

            Temp = (Word32) (*Line).Sfs[2].Pamp ;
            Bsp = Par2Ser( Temp, Bsp, 6 ) ;

            Temp = (Word32) (*Line).Sfs[3].Pamp ;
            Bsp = Par2Ser( Temp, Bsp, 5 ) ;
        }

        /* Low rate only part */
        else {

            /* Write 12 bits of positions */
            for ( i = 0 ; i < SubFrames ; i ++ ) {
                Temp = (*Line).Sfs[i].Ppos ;
                Bsp = Par2Ser( Temp, Bsp, 12 ) ;
            }

            /* Write 4 bit Pamps */
            for ( i = 0 ; i < SubFrames ; i ++ ) {
                Temp = (*Line).Sfs[i].Pamp ;
                Bsp = Par2Ser( Temp, Bsp, 4 ) ;
            }
        }

    }

    else if(Ftyp == 2) {   /* SID frame */

        /* 24 bit LspId */
        Temp = (*Line).LspId ;
        Bsp = Par2Ser( Temp, Bsp, 24 ) ;

        /* Do Sid frame gain */
        Temp = (Word32)(*Line).Sfs[0].Mamp ;
        Bsp = Par2Ser( Temp, Bsp, 6 ) ;
    }

    /* Write out active frames */
    if ( Ftyp == 1 ) {
        if ( WrkRate == Rate63 )
            BitCount = 192 ;
        else
            BitCount = 160 ;
    }
    /* Non active frames */
    else if ( Ftyp == 2 )
        BitCount = 32 ;
    else
        BitCount = 2;

    for ( i = 0 ; i < BitCount ; i ++ )
        Vout[i>>3] ^= BitStream[i] << (i & 0x0007) ;

    return;
}

Word16* Par2Ser( Word32 Inp, Word16 *Pnt, int BitNum )
{
    int i   ;
    Word16  Temp ;

    for ( i = 0 ; i < BitNum ; i ++ ) {
        Temp = (Word16) Inp & (Word16)0x0001 ;
        Inp >>= 1 ;
        *Pnt ++ = Temp ;
    }

    return Pnt ;
}

/*
**
** Function:        Line_Unpk()
**
** Description:     unpacking of bitstream, gets coding parameters for a frame
**
** Links to text:   Section 4
**
** Arguments:
**
**  char   *Vinp        bitstream chars
**  Word16 *VadType
**
** Outputs:
**
**  Word16 *VadType
**
** Return value:
**
**  LINEDEF             coded parameters
**     Word16   Crc
**     Word32   LspId
**     Word16   Olp[SubFrames/2]
**     SFSDEF   Sfs[SubFrames]
**
*/
LINEDEF  Line_Unpk( char *Vinp, Word16 *Ftyp, Word16 Crc )
{
    int   i  ;
    Word16  BitStream[192] ;
    Word16 *Bsp = BitStream ;
    LINEDEF Line ;
    Word32  Temp ;
    Word16  Info;
    Word16 Bound_AcGn;

    Line.Crc = Crc;
    if(Crc != 0)
        {
           Line.LspId = 0L;    /* Dummy : to avoid gcc warning */ 
           return Line;
        }

    /* Unpack the byte info to BitStream vector */
    for ( i = 0 ; i < 192 ; i ++ )
        BitStream[i] = ( Vinp[i>>3] >> (i & (Word16)0x0007) ) & (Word16)1 ;

    /* Decode the frame type and rate info */
    Info = (Word16)Ser2Par( &Bsp, 2 ) ;

    if ( Info == 3 ) {
        *Ftyp = 0;
        Line.LspId = 0L;    /* Dummy : to avoid Borland C3.1 warning */
        return Line;
    }

    /* Decode the LspId */
    Line.LspId = Ser2Par( &Bsp, 24 ) ;

    if ( Info == 2 ) {
        /* Decode the Noise Gain */
        Line.Sfs[0].Mamp = (Word16)Ser2Par( &Bsp, 6);
        *Ftyp = 2;
        return Line ;
    }

 /*
  * Decode the common information to both rates
  */
    *Ftyp = 1;

    /* Decode the bit-rate */
    WrkRate = (Info == 0) ? Rate63 : Rate53;

    /* Decode the adaptive codebook lags */
    Temp = Ser2Par( &Bsp, 7 ) ;
    /* Test if forbidden code */
    if( Temp <= 123) {
        Line.Olp[0] = (Word16) Temp + (Word16)PitchMin ;
    }
    else {
        /* transmission error */
        Line.Crc = 1;
        return Line ;
    }

    Line.Sfs[1].AcLg = (Word16) Ser2Par( &Bsp, 2 ) ;

    Temp = Ser2Par( &Bsp, 7 ) ;
    /* Test if forbidden code */
    if( Temp <= 123) {
        Line.Olp[1] = (Word16) Temp + (Word16)PitchMin ;
    }

    else {
        /* transmission error */
        Line.Crc = 1;
        return Line ;
    }

    Line.Sfs[3].AcLg = (Word16) Ser2Par( &Bsp, 2 ) ;

    Line.Sfs[0].AcLg = 1 ;
    Line.Sfs[2].AcLg = 1 ;

    /* Decode the combined gains accordingly to the rate */
    for ( i = 0 ; i < SubFrames ; i ++ ) {

        Temp = Ser2Par( &Bsp, 12 ) ;

        Line.Sfs[i].Tran = 0 ;
        Bound_AcGn = NbFilt170 ;
        if ( (WrkRate == Rate63) && (Line.Olp[i>>1] < (SubFrLen-2) ) ) {
            Line.Sfs[i].Tran = (Word16)(Temp >> 11) ;
            Temp &= 0x000007ffL ;
            Bound_AcGn = NbFilt085 ;
        }
        Line.Sfs[i].AcGn = (Word16)(Temp / (Word16)NumOfGainLev) ;
        if(Line.Sfs[i].AcGn < Bound_AcGn ) {
            Line.Sfs[i].Mamp = (Word16)(Temp % (Word16)NumOfGainLev) ;
        }
        else {
            /* error detected */
            Line.Crc = 1;
            return Line ;
        }
    }

    /* Decode the grids */
    for ( i = 0 ; i < SubFrames ; i ++ )
        Line.Sfs[i].Grid = *Bsp ++ ;

    if (Info == 0) {

        /* Skip the reserved bit */
        Bsp ++ ;

        /* Decode 13 bit combined position index */
        Temp = Ser2Par( &Bsp, 13 ) ;
        Line.Sfs[0].Ppos = ( Temp/90 ) / 9 ;
        Line.Sfs[1].Ppos = ( Temp/90 ) % 9 ;
        Line.Sfs[2].Ppos = ( Temp%90 ) / 9 ;
        Line.Sfs[3].Ppos = ( Temp%90 ) % 9 ;

        /* Decode all the pulse positions */
        Line.Sfs[0].Ppos = ( Line.Sfs[0].Ppos << 16 ) + Ser2Par( &Bsp, 16 ) ;
        Line.Sfs[1].Ppos = ( Line.Sfs[1].Ppos << 14 ) + Ser2Par( &Bsp, 14 ) ;
        Line.Sfs[2].Ppos = ( Line.Sfs[2].Ppos << 16 ) + Ser2Par( &Bsp, 16 ) ;
        Line.Sfs[3].Ppos = ( Line.Sfs[3].Ppos << 14 ) + Ser2Par( &Bsp, 14 ) ;

        /* Decode pulse amplitudes */
        Line.Sfs[0].Pamp = (Word16)Ser2Par( &Bsp, 6 ) ;
        Line.Sfs[1].Pamp = (Word16)Ser2Par( &Bsp, 5 ) ;
        Line.Sfs[2].Pamp = (Word16)Ser2Par( &Bsp, 6 ) ;
        Line.Sfs[3].Pamp = (Word16)Ser2Par( &Bsp, 5 ) ;
    }

    else {

        /* Decode the positions */
        for ( i = 0 ; i < SubFrames ; i ++ )
            Line.Sfs[i].Ppos = Ser2Par( &Bsp, 12 ) ;

        /* Decode the amplitudes */
        for ( i = 0 ; i < SubFrames ; i ++ )
            Line.Sfs[i].Pamp = (Word16)Ser2Par( &Bsp, 4 ) ;
    }
   return Line ;
}

Word32  Ser2Par( Word16 **Pnt, int Count )
{
    int     i ;
    Word32  Rez = 0L ;

    for ( i = 0 ; i < Count ; i ++ ) {
        Rez += (Word32) **Pnt << i ;
        (*Pnt) ++ ;
    }
    return Rez ;
}

/*
**
** Function:        Comp_En()
**
** Description:     Compute energy of a subframe vector
**
** Links to text:
**
** Arguments:
**
**  Word16 *Dpnt
**
** Outputs:         None
**
** Return value:
**
**      Word32 energy
**
*/
Word32   Comp_En( Word16 *Dpnt )
{
    int   i ;
    Word32   Rez ;
    Word16   Temp[SubFrLen] ;

    for ( i = 0 ; i < SubFrLen ; i ++ )
        Temp[i] = g723_shr( Dpnt[i], (Word16) 2 ) ;

    Rez = (Word32) 0 ;
    for ( i = 0 ; i < SubFrLen ; i ++ )
        Rez = g723_L_mac( Rez, Temp[i], Temp[i] ) ;

    return Rez ;
}

/*
**
** Function:        Sqrt_lbc()
**
** Description:     Square root computation
**
** Links to text:
**
** Arguments:
**
**  Word32 Num
**
** Outputs:     None
**
** Return value:
**
**  Word16 square root of num
**
*/
Word16   Sqrt_lbc( Word32 Num )
{
    int   i  ;

    Word16   Rez = (Word16) 0 ;
    Word16   Exp = (Word16) 0x4000 ;

    Word32   Acc ;

    for ( i = 0 ; i < 14 ; i ++ ) {

        Acc = L_g723_mult( g723_add(Rez, Exp), g723_add(Rez, Exp) ) ;
        if ( Num >= Acc )
            Rez = g723_add( Rez, Exp ) ;

        Exp = g723_shr( Exp, (Word16) 1 ) ;
    }
    return Rez ;
}

/*
**
** Function:        Rand_lbc()
**
** Description:     Generator of random numbers
**
** Links to text:   Section 3.10.2
**
** Arguments:
**
**  Word16 *p
**
** Outputs:
**
**  Word16 *p
**
** Return value:
**
**  Word16 random number
**
*/
Word16   Rand_lbc( Word16 *p )
{
    Word32   Temp ;

    Temp = g723_L_deposit_l( *p ) ;
    Temp &= (Word32) 0x0000ffff ;
    Temp = Temp*(Word32)521 + (Word32) 259 ;
    *p = g723_extract_l( Temp ) ;
    return g723_extract_l( Temp ) ;
}

/*
**
** Function:        Scale()
**
** Description:     Postfilter gain scaling
**
** Links to text:   Section 3.9
**
** Arguments:
**
**  Word16 *Tv
**  Word32 Sen
**
**  Inputs:
**
**  Word16 DecStat.Gain
**
** Outputs:
**
**  Word16 *Tv
**
** Return value:    None
**
*/
void  Scale( Word16 *Tv, Word32 Sen )
{
    int   i ;

    Word32   Acc0,Acc1   ;
    Word16   Exp,SfGain  ;


    Acc0 = Sen ;
    Acc1 = Comp_En( Tv ) ;

    /* Normalize both */
    if ( (Acc1 != (Word32) 0) && (Acc0 != (Word32) 0 ) ) {

        Exp = g723_norm_l( Acc1 ) ;
        Acc1 = L_g723_shl( Acc1, Exp ) ;

        SfGain = g723_norm_l( Acc0 ) ;
        Acc0 = L_g723_shl( Acc0, SfGain ) ;
        Acc0 = L_g723_shr( Acc0, (Word16) 1 ) ;
        Exp = g723_sub( Exp, SfGain ) ;
        Exp = g723_add( Exp, (Word16) 1 ) ;
        Exp = g723_sub( (Word16) 6, Exp ) ;
        if ( Exp < (Word16) 0 )
            Exp = (Word16) 0 ;

        SfGain = g723_extract_h( Acc1 ) ;

        SfGain = div_l( Acc0, SfGain ) ;

        Acc0 = g723_L_deposit_h( SfGain ) ;

        Acc0 = L_g723_shr( Acc0, Exp ) ;

        SfGain = Sqrt_lbc( Acc0 ) ;
    }
    else
        SfGain = 0x1000 ;

    /* Filter the data */
    for ( i = 0 ; i < SubFrLen ; i ++ ) {

        /* Update gain */
        Acc0 = g723_L_deposit_h( DecStat.Gain ) ;
        Acc0 = g723_L_msu( Acc0, DecStat.Gain, (Word16) 0x0800 ) ;
        Acc0 = g723_L_mac( Acc0, SfGain, (Word16) 0x0800 ) ;
        DecStat.Gain = round_( Acc0 ) ;

        Exp = g723_add( DecStat.Gain, g723_shr( DecStat.Gain, (Word16) 4) ) ;

        Acc0 = L_g723_mult( Tv[i], Exp ) ;
        Acc0 = L_g723_shl( Acc0, (Word16) 4 ) ;
        Tv[i] = round_( Acc0 ) ;
    }

    return;
}



