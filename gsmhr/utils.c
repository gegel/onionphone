/*___________________________________________________________________________
 |                                                                           |
 |                                                                           |
 |                 Residual Error Insertion Device                           |
 |                                                                           |
 |                                                                           |
 |      File  :  REID.C                                                      |
 |                                                                           |
 |      Date  :  February 03, 1995                                           |
 |                                                                           |
 |      Version: 4.1                                                         |
 |                                                                           |
 |                                                                           |
 |   Description:                                                            |
 |   ------------                                                            |
 |      This routine transforms the output file format of the GSM Half       |
 |      Rate Encoder module consisting of:                                   |
 |           * 18 speech parameters (see GSM TS 06.20)                       |
 |           *  1 speech flag SP (see GSM TS 06.41)                          |
 |           *  1 voice activity flag VAD (see GSM TS 06.42)                 |
 |                                                                           |
 |      to the input file format of the GSM Half Rate Decoder module         |
 |      requiring:                                                           |
 |           * 18 speech parameters (see GSM TS 06.20)                       |
 |           *  1 channel condition flag BFI (see GSM TS 06.21, 05.05)       |
 |           *  1 channel condition flag UFI (see GSM TS 06.21, 05.05)       |
 |           *  1 SID flag (2 bits) (see GSM TS 06.41, 05.05)                |
 |           *  1 time alignment flag TAF (see GSM TS 06.41)                 |
 |                                                                           |
 |      Between SID updates the speech parameters are replaced by random     |
 |      values simulating an interrupted transmission on the air interface   |
 |                                                                           |
 |      The actual implementation only supports error free transmission (EP0)|
 |                                                                           |
 |      The shell for the future use of error patterns (residual error       |
 |      pattern insertion) is already included.  If necessary, byte swapping |
 |      is performed on the input speech parameters so that they are always  |
 |      represented internally in PC byte order (assuming that the byte      |
 |      order of the input file is compatible with the machine on which the  |
 |      program is run).  However, byte swapping is not done on the flag     |
 |      words (input: SP and VAD, output: BFI, UFI, SID, and TAF).  Thus,    |
 |      the residual error pattern insertion code may be written to handle   |
 |      the speech parameter words on a byte basis, but the flag words must  |
 |      always be handled on a word basis.                                   |
 |___________________________________________________________________________|
*/
/*___________________________________________________________________________
 |                                                                           |
 |      Creation: 19.12.94                                                   |
 |                                                                           |
 |  Changes:                                                                 |
 |      22.12.94:  Removal of BCI flag, instead: determination of SID flag   |
 |      12.01.95:  SID update period = 12 (instead of 24)                    |
 |      13.01.95:  When in CNI mode, the parameters between SID updates are  |
 |                 random values. This simulates the interrupted transmission|
 |      03.02.95:  Longword main( Longword...) replaced by int main(int ...),|
 |                 initial value of swTAFCnt set to 1                        |
 |___________________________________________________________________________|
*/

/*___________________________________________________________________________
 |                                                                           |
 |      Include-Files                                                        |
 |___________________________________________________________________________|
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef VAX
#   define OPEN_WI  "wb","mrs=512","rfm=fix","ctx=stm"
#   define OPEN_RI  "rb","mrs=512","rfm=fix","ctx=stm"
#   define OPEN_WB  "wb","mrs=512","rfm=fix","ctx=stm"
#   define OPEN_RB  "rb","mrs=2","rfm=fix","ctx=stm"
#   define OPEN_WT  "w","mrs=256","rat=cr","rfm=var"
#   define OPEN_RT  "r","mrs=256","rat=cr","rfm=var"
#else
#   define OPEN_WB  "wb"
#   define OPEN_RB  "rb"
#   define OPEN_WI  "wb"
#   define OPEN_RI  "rb"
#   define OPEN_WT  "wt"
#   define OPEN_RT  "rt"
#endif

#define LW_SIGN (long)0x80000000       /* sign bit */
#define LW_MIN (long)0x80000000
#define LW_MAX (long)0x7fffffff
#define SW_MIN (short)0x8000           /* smallest Ram */
#define SW_MAX (short)0x7fff           /* largest Ram */

typedef char  Byte;
typedef long int Longword;             /* 32 bit "accumulator" (L_*) */
typedef short int Shortword;           /* 16 bit "register"  (sw*) */

/*___________________________________________________________________________
 |                                                                           |
 |      local Functions                                                      |
 |___________________________________________________________________________|
*/
static Longword error_free( FILE *infile, FILE *outfile);
static void SwapBytes( Shortword buffer[], Longword len );
static Longword ByteOrder( void );
static size_t ReadInputFile( Shortword buffer[], FILE *fp );
static size_t WriteOutputFile( Shortword buffer[], FILE *fp );
static Longword EncoderInterface( FILE *infile, Shortword swInPara[] );
static Shortword swSidDetection(Shortword pswParameters[], 
                                                  Shortword pswErrorFlag[]);
static void RandomParameters(Shortword pswParameters[]);
static Shortword getPnBits(Shortword iBits, Longword *pL_PNSeed);
FILE *OpenBinfile( char *name, char *mode );
Longword Strincmp( const char *s, const char *t, size_t max );
Longword Stricmp( const char *s, const char *t );

Longword L_shr(Longword L_var1, Shortword var2);   /* 2 ops */
Longword L_shl(Longword L_var1, Shortword var2);   /* 2 ops */
Shortword shr(Shortword var1, Shortword var2);     /* 1 ops */
Shortword shl(Shortword var1, Shortword var2);     /* 1 ops */

/*___________________________________________________________________________
 |                                                                           |
 |      Subroutines                                                          |
 |___________________________________________________________________________|
*/
static Longword error_free( FILE *infile, FILE *outfile)
{

#define SPEECH      1
#define CNIFIRSTSID 2
#define CNICONT     3
#define VALIDSID    11
#define GOODSPEECH  33

    static Shortword swDecoMode = {SPEECH};
    static Shortword swTAFCnt = {1};
    Shortword swInPara[20], i, swFrameType;
    Shortword swOutPara[22],pswErrorFlag[3];
 
    if( EncoderInterface( infile, swInPara )) return( 1 );

    /* Copy input parameters to output parameters (error free transmission) */
    /* -------------------------------------------------------------------- */
    for (i=0;i<18;i++)
       swOutPara[i] = swInPara[i];

    /* Set channel status flags (error free transmission) */
    /* -------------------------------------------------- */
    swOutPara[18] = 0;     /* BFI flag */
    swOutPara[19] = 0;     /* UFI flag */

    /* Evaluate SID flag */
    /* ----------------- */
    pswErrorFlag[0] = 0;   /* BFI flag */
    pswErrorFlag[1] = 0;   /* UFI flag */
    pswErrorFlag[2] = 0;   /* BCI flag */
    swOutPara[20] = swSidDetection(swOutPara, pswErrorFlag);


    /* Evaluate TAF flag */
    /* ----------------- */
    if (swTAFCnt == 0) swOutPara[21] = 1;
    else               swOutPara[21] = 0;
    swTAFCnt = (swTAFCnt + 1) % 12;

   
    /* Frame classification:                                                */
    /* Since the transmission is error free, the received frames are either */
    /* valid speech or valid SID frames                                     */
    /* -------------------------------------------------------------------- */
    if      ( swOutPara[20] == 2) swFrameType = VALIDSID;
    else if ( swOutPara[20] == 0) swFrameType = GOODSPEECH;
    else {
            printf( "Error in SID detection\n" );
            return( 1 );
    }

    /* Update of decoder state */
    /* ----------------------- */
    if (swDecoMode == SPEECH) {
       if      (swFrameType == VALIDSID)   swDecoMode = CNIFIRSTSID;
       else if (swFrameType == GOODSPEECH) swDecoMode = SPEECH;
    }
    else {  /* comfort noise insertion mode */
       if      (swFrameType == VALIDSID)   swDecoMode = CNICONT;
       else if (swFrameType == GOODSPEECH) swDecoMode = SPEECH;
    }


    /* Replace parameters by random data if in CNICONT-mode and TAF=0 */
    /* -------------------------------------------------------------- */
    if ((swDecoMode == CNICONT) && (swOutPara[21] == 0)){
       RandomParameters(swOutPara);
       /* Set flags such, that an "unusable frame" is produced */
       swOutPara[18] = 1;     /* BFI flag */
       swOutPara[19] = 1;     /* UFI flag */
       swOutPara[20] = 0;     /* SID flag */
    }    



    if( outfile ) {
        if( WriteOutputFile( swOutPara, outfile )) {
            printf( "Error writing File\n" );
            return( 1 );
        }
    }
    return( 0 );
}

static Longword EncoderInterface( FILE *infile, Shortword swInPara[] )
{
    size_t i = 0;    

    i = ReadInputFile( swInPara, infile );

    return(( i == 0 ) ? 1 : 0 );
}

static size_t ReadInputFile( Shortword buffer[], FILE *fp )
{
    size_t i;

    i = fread( buffer, sizeof( Shortword ), 20, fp );
    SwapBytes( buffer, 18 );
    return( i );
}

static size_t WriteOutputFile( Shortword buffer[], FILE *fp )
{
    size_t i;

    SwapBytes( buffer, 18 );
    i = fwrite( buffer, sizeof( Shortword ), 22, fp );
    return( ( i == 22 ) ? 0 : 1 );
}


static void SwapBytes( Shortword buffer[], Longword len )
{
    Byte *pc, tmp;
    Longword i;
    
    if( !ByteOrder())
        return;
    pc = (Byte *)buffer;
    for( i = 0; i < len; i++ ) {
        tmp   = pc[0];
        pc[0] = pc[1];
        pc[1] = tmp;
        pc += 2;
    }
}

static Longword ByteOrder( void )
{
    Shortword si;
    Byte *pc;

    si = 0x1234;
    pc = (Byte *)&si;
    if (pc[1] == 0x12 && pc[0] == 0x34 )
            return( 0 );
    if (pc[0] == 0x12 && pc[1] == 0x34 )
            return( 1 );
    printf( "Error in ByteOrder: %X, %X\n", (int)pc[0], (int)pc[1] );
    exit( 1 );
    return( 2 );
}

FILE *OpenBinfile( char *name, char *mode )
{
    FILE *fp;

    if( toupper( *mode ) == 'W' ) { /* Write access */
        if(( fp = fopen( name, OPEN_WB )) == NULL ) { 
            printf( "Can't open output file '%s'\n", name );
            exit( 1 );
        }
    } else { /* Read access */
        if(( fp = fopen( name, OPEN_RB )) == NULL ) { 
            printf( "Can't open file '%s'\n", name );
            exit( 1 );
        } 
    }
    return( fp );
}

Longword Strincmp( const char *s, const char *t, size_t max )
{
    for( ; max > 1; ++s, ++t, --max ) {
        if( toupper( *s ) != toupper( *t ))
            break;
        if( *s == '\0' )
            return( 0 );
    }
    return( toupper( *s ) - toupper( *t ));
}

Longword Stricmp( const char *s, const char *t )
{
    for(; toupper( *s ) == toupper( *t ); ++s, ++t ) {
        if( *s == '\0' )
            return( 0 );
    }
    return( toupper( *s ) - toupper( *t ));
}

/*************************************************************************
 *
 *   FUNCTION NAME: getPnBits
 *
 *   PURPOSE:
 *     
 *     Generate iBits pseudo-random bits using *pL_PNSeed as the
 *     pn-generators seed.
 *
 *   INPUTS:
 *
 *     iBits - integer indicating how many random bits to return.
 *     range [0,15], 0 yields 1 bit output
 *     
 *     *pL_PNSeed - 32 bit seed (changed by function)
 *     
 *   OUTPUTS:
 *     
 *     *pL_PNSeed - 32 bit seed, modified.
 *     
 *   RETURN VALUE:
 *     
 *    random bits in iBits LSB's.  
 *     
 *     
 *   IMPLEMENTATION:
 *     
 *    implementation of x**31 + x**3 + 1 == PN_XOR_REG | PN_XOR_ADD a
 *    PN sequence generator using Longwords generating a 2**31 -1
 *    length pn-sequence.
 *
 *************************************************************************/

static Shortword getPnBits(Shortword iBits, Longword *pL_PNSeed){

#define PN_XOR_REG (Longword)0x00000005L
#define PN_XOR_ADD (Longword)0x40000000L

  Shortword swPnBits=0;
  Longword L_Taps,L_FeedBack;
  Shortword i;

  for (i=0; i < iBits; i++){
    
    /* update the state */                     
    /********************/
    
    L_Taps = *pL_PNSeed & PN_XOR_REG;  
    L_FeedBack = L_Taps; /* Xor tap bits to yield feedback bit */
    L_Taps = L_shr(L_Taps,1);

    while(L_Taps){
      L_FeedBack = L_FeedBack ^ L_Taps;
      L_Taps = L_shr(L_Taps,1);
    }

    /* LSB of L_FeedBack is next MSB of PN register */

    *pL_PNSeed = L_shr(*pL_PNSeed,1);
    if (L_FeedBack & 1)
       *pL_PNSeed = *pL_PNSeed | PN_XOR_ADD;
    
    /* State update complete.  
       Get the output bit from the state, add/or it into output */

    swPnBits = shl(swPnBits,1);
    swPnBits = swPnBits | (*pL_PNSeed & 1);
    
  }
  return(swPnBits);
}


/***************************************************************************
 *
 *   FUNCTION NAME: L_shl
 *
 *   PURPOSE:
 *
 *     Arithmetic shift left (or right).
 *     Arithmetically shift the input left by var2.   If var2 is
 *     negative then an arithmetic shift right (L_shr) of L_var1 by
 *     -var2 is performed.
 *
 *   INPUTS:
 *
 *     var2
 *                     16 bit short signed integer (Shortword) whose value
 *                     falls in the range 0xffff 8000 <= var2 <= 0x0000 7fff.
 *     L_var1
 *                     32 bit long signed integer (Longword) whose value
 *                     falls in the range
 *                     0x8000 0000 <= L_var1 <= 0x7fff ffff.
 *   OUTPUTS:
 *
 *     none
 *
 *   RETURN VALUE:
 *
 *     L_Out
 *                     32 bit long signed integer (Longword) whose value
 *                     falls in the range
 *                     0x8000 0000 <= L_var1 <= 0x7fff ffff.
 *
 *
 *   IMPLEMENTATION:
 *
 *     Arithmetically shift the 32 bit input left by var2.  This
 *     operation maintains the sign of the input number. If var2 is
 *     negative then an arithmetic shift right (L_shr) of L_var1 by
 *     -var2 is performed.  See description of L_shr for details.
 *
 *     Equivalent to the Full-Rate GSM ">> n" operation.  Note that
 *     ANSI-C does not guarantee operation of the C ">>" or "<<"
 *     operator for negative numbers.
 *
 *   KEYWORDS: shift, arithmetic shift left,
 *
 *************************************************************************/

Longword L_shl(Longword L_var1, Shortword var2)
{

  Longword L_Mask,
         L_Out;
  int    i,
         iOverflow = 0;

  if (var2 == 0 || L_var1 == 0)
  {
    L_Out = L_var1;
  }
  else if (var2 < 0)
  {
    if (var2 <= -31)
    {
      if (L_var1 > 0)
        L_Out = 0;
      else
        L_Out = 0xffffffffL;
    }
    else
      L_Out = L_shr(L_var1, -var2);
  }
  else
  {

    if (var2 >= 31)
      iOverflow = 1;

    else
    {

      if (L_var1 < 0)
        L_Mask = LW_SIGN;              /* sign bit mask */
      else
        L_Mask = 0x0;
      L_Out = L_var1;
      for (i = 0; i < var2 && !iOverflow; i++)
      {
        /* check the sign bit */
        L_Out = (L_Out & 0x7fffffffL) << 1;
        if ((L_Mask ^ L_Out) & LW_SIGN)
          iOverflow = 1;
      }
    }

    if (iOverflow)
    {
      /* saturate */
      if (L_var1 > 0)
        L_Out = LW_MAX;
      else
        L_Out = LW_MIN;
    }
  }

  return (L_Out);
}

/***************************************************************************
 *
 *   FUNCTION NAME: L_shr
 *
 *   PURPOSE:
 *
 *     Arithmetic shift right (or left).
 *     Arithmetically shift the input right by var2.   If var2 is
 *     negative then an arithmetic shift left (shl) of var1 by
 *     -var2 is performed.
 *
 *   INPUTS:
 *
 *     var2
 *                     16 bit short signed integer (Shortword) whose value
 *                     falls in the range 0xffff 8000 <= var2 <= 0x0000 7fff.
 *     L_var1
 *                     32 bit long signed integer (Longword) whose value
 *                     falls in the range
 *                     0x8000 0000 <= L_var1 <= 0x7fff ffff.
 *   OUTPUTS:
 *
 *     none
 *
 *   RETURN VALUE:
 *
 *     L_Out
 *                     32 bit long signed integer (Longword) whose value
 *                     falls in the range
 *                     0x8000 0000 <= L_var1 <= 0x7fff ffff.
 *
 *
 *   IMPLEMENTATION:
 *
 *     Arithmetically shift the input right by var2.  This
 *     operation maintains the sign of the input number. If var2 is
 *     negative then an arithmetic shift left (shl) of L_var1 by
 *     -var2 is performed.  See description of L_shl for details.
 *
 *     The input is a 32 bit number, as is the output.
 *
 *     Equivalent to the Full-Rate GSM ">> n" operation.  Note that
 *     ANSI-C does not guarantee operation of the C ">>" or "<<"
 *     operator for negative numbers.
 *
 *   KEYWORDS: shift, arithmetic shift right,
 *
 *************************************************************************/

Longword L_shr(Longword L_var1, Shortword var2)
{

  Longword L_Mask,
         L_Out;

  if (var2 == 0 || L_var1 == 0)
  {
    L_Out = L_var1;
  }
  else if (var2 < 0)
  {
    /* perform a left shift */
    /*----------------------*/
    if (var2 <= -31)
    {
      /* saturate */
      if (L_var1 > 0)
        L_Out = LW_MAX;
      else
        L_Out = LW_MIN;
    }
    else
      L_Out = L_shl(L_var1, -var2);
  }
  else
  {

    if (var2 >= 31)
    {
      if (L_var1 > 0)
        L_Out = 0;
      else
        L_Out = 0xffffffffL;
    }
    else
    {
      L_Mask = 0;

      if (L_var1 < 0)
      {
        L_Mask = ~L_Mask << (32 - var2);
      }

      L_var1 >>= var2;
      L_Out = L_Mask | L_var1;
    }
  }
  return (L_Out);
}


/***************************************************************************
 *
 *   FUNCTION NAME: shl
 *
 *   PURPOSE:
 *
 *     Arithmetically shift the input left by var2.
 *
 *
 *   INPUTS:
 *
 *     var1
 *                     16 bit short signed integer (Shortword) whose value
 *                     falls in the range 0xffff 8000 <= var1 <= 0x0000 7fff.
 *     var2
 *                     16 bit short signed integer (Shortword) whose value
 *                     falls in the range 0xffff 8000 <= var2 <= 0x0000 7fff.
 *
 *   OUTPUTS:
 *
 *     none
 *
 *   RETURN VALUE:
 *
 *     swOut
 *                     16 bit short signed integer (Shortword) whose value
 *                     falls in the range
 *                     0xffff 8000 <= swOut <= 0x0000 7fff.
 *
 *   IMPLEMENTATION:
 *
 *     If Arithmetically shift the input left by var2.  If var2 is
 *     negative then an arithmetic shift right (shr) of var1 by
 *     -var2 is performed.  See description of shr for details.
 *     When an arithmetic shift left is performed the var2 LS bits
 *     are zero filled.
 *
 *     The only exception is if the left shift causes an overflow
 *     or underflow.  In this case the LS bits are not modified.
 *     The number returned is 0x8000 in the case of an underflow or
 *     0x7fff in the case of an overflow.
 *
 *     The shl is equivalent to the Full-Rate GSM "<< n" operation.
 *     Note that ANSI-C does not guarantee operation of the C ">>"
 *     or "<<" operator for negative numbers - it is not specified
 *     whether this shift is an arithmetic or logical shift.
 *
 *   KEYWORDS: asl, arithmetic shift left, shift
 *
 *************************************************************************/

Shortword shl(Shortword var1, Shortword var2)
{
  Shortword swOut;
  Longword L_Out;

  if (var2 == 0 || var1 == 0)
  {
    swOut = var1;
  }
  else if (var2 < 0)
  {

    /* perform a right shift */
    /*-----------------------*/

    if (var2 <= -15)
    {
      if (var1 < 0)
        swOut = (Shortword) 0xffff;
      else
        swOut = 0x0;
    }
    else
      swOut = shr(var1, -var2);

  }
  else
  {
    /* var2 > 0 */
    if (var2 >= 15)
    {
      /* saturate */
      if (var1 > 0)
        swOut = SW_MAX;
      else
        swOut = SW_MIN;
    }
    else
    {

      L_Out = (Longword) var1 *(1 << var2);

      swOut = (Shortword) L_Out;       /* copy low portion to swOut,
                                        * overflow could have hpnd */
      if (swOut != L_Out)
      {
        /* overflow  */
        if (var1 > 0)
          swOut = SW_MAX;              /* saturate */
        else
          swOut = SW_MIN;              /* saturate */
      }
    }
  }
  return (swOut);
}

/***************************************************************************
 *
 *   FUNCTION NAME: shr
 *
 *   PURPOSE:
 *
 *     Arithmetic shift right (or left).
 *     Arithmetically shift the input right by var2.   If var2 is
 *     negative then an arithmetic shift left (shl) of var1 by
 *     -var2 is performed.
 *
 *   INPUTS:
 *
 *     var1
 *                     16 bit short signed integer (Shortword) whose value
 *                     falls in the range 0xffff 8000 <= var1 <= 0x0000 7fff.
 *     var2
 *                     16 bit short signed integer (Shortword) whose value
 *                     falls in the range 0xffff 8000 <= var2 <= 0x0000 7fff.
 *
 *   OUTPUTS:
 *
 *     none
 *
 *   RETURN VALUE:
 *
 *     swOut
 *                     16 bit short signed integer (Shortword) whose value
 *                     falls in the range
 *                     0xffff 8000 <= swOut <= 0x0000 7fff.
 *
 *   IMPLEMENTATION:
 *
 *     Arithmetically shift the input right by var2.  This
 *     operation maintains the sign of the input number. If var2 is
 *     negative then an arithmetic shift left (shl) of var1 by
 *     -var2 is performed.  See description of shl for details.
 *
 *     Equivalent to the Full-Rate GSM ">> n" operation.  Note that
 *     ANSI-C does not guarantee operation of the C ">>" or "<<"
 *     operator for negative numbers.
 *
 *   KEYWORDS: shift, arithmetic shift right,
 *
 *************************************************************************/

Shortword shr(Shortword var1, Shortword var2)
{

  Shortword swMask,
         swOut;

  if (var2 == 0 || var1 == 0)
    swOut = var1;

  else if (var2 < 0)
  {
    /* perform an arithmetic left shift */
    /*----------------------------------*/
    if (var2 <= -15)
    {
      /* saturate */
      if (var1 > 0)
        swOut = SW_MAX;
      else
        swOut = SW_MIN;
    }
    else
      swOut = shl(var1, -var2);
  }

  else
  {

    /* positive shift count */
    /*----------------------*/

    if (var2 >= 15)
    {
      if (var1 < 0)
        swOut = (Shortword) 0xffff;
      else
        swOut = 0x0;
    }
    else
    {
      /* take care of sign extension */
      /*-----------------------------*/

      swMask = 0;
      if (var1 < 0)
      {
        swMask = ~swMask << (16 - var2);
      }

      var1 >>= var2;
      swOut = swMask | var1;

    }
  }
  return (swOut);
}

/*___________________________________________________________________________
 |                                                                           |
 |     This subroutine calculates the 'SID flag'                             |
 |                                                                           |
 |     Input:     pswParameters[18]                                          |
 |                           input parameters of the speech decoder          |
 |                                                                           |
 |                pswErrorFlag[3]                                            |
 |                           error flags, generated by channel decoder       |
 |                                                                           |
 |     Return Value:                                                         |
 |                0:         speech frame detected                           |
 |                1:         most likely SID frame received                  |
 |                2:         SID frame detected                              |
 |                                                                           |
 |___________________________________________________________________________|
 |                                                                           |
 |     History:                                                              |
 |                                                                           |
 |     12-Oct-1994: Bug removed: error corrected in case of a mode (unvoiced/|
 |                  voiced) mismatch, if a SID frame was received as an      |
 |                  unvoiced frame                                           |
 |___________________________________________________________________________|
*/

static Shortword swSidDetection(Shortword pswParameters[],
                                Shortword pswErrorFlag[])
{
  static Shortword ppswIBit[2][18] = {
        5, 11,9,8, 1, 2, 7,7,5, 7,7,5, 7,7,5, 7,7,5,  /* unvoiced */
        5, 11,9,8, 1, 2, 8,9,5, 4,9,5, 4,9,5, 4,9,5}; /* voiced */

  static Shortword ppswCL1pCL2[2][18] = {
                              0x0001, /* R0      */  /* unvoiced */
                              0x00ef, /* LPC1    */
                              0x003e, /* LPC2    */
                              0x007f, /* LPC3    */
                              0x0001, /* INT LPC */
                              0x0003, /* Mode    */
                              0x001f, /* Code1_1 */
                              0x0072, /* Code2_1 */
                              0x0012, /* GSP0_1  */
                              0x003f, /* Code1_2 */
                              0x007f, /* Code2_2 */
                              0x0008, /* GSP0_2  */
                              0x007f, /* Code1_3 */
                              0x007f, /* Code2_3 */
                              0x0008, /* GSP0_3  */
                              0x007f, /* Code1_4 */
                              0x007f, /* Code2_4 */
                              0x000c, /* GSP0_4  */
                              
                              0x0000, /* R0      */  /* voiced */
                              0x0000, /* LPC1    */
                              0x0000, /* LPC2    */
                              0x0000, /* LPC3    */
                              0x0001, /* INT LPC */
                              0x0003, /* Mode    */
                              0x00ff, /* Lag_1   */
                              0x01ff, /* Code_1  */
                              0x001f, /* GSP0_1  */
                              0x000f, /* Lag_2   */
                              0x01ff, /* Code_2  */
                              0x001f, /* GSP0_2  */
                              0x000f, /* Lag_3   */
                              0x01ff, /* Code_3  */
                              0x001f, /* GSP0_3  */
                              0x000f, /* Lag_4   */
                              0x01ff, /* Code_4  */
                              0x001f}; /* GSP0_4 */

  static Shortword ppswCL2[2][18] = {
                              0x0000, /* R0      */ /* unvoiced */
                              0x0000, /* LPC1    */
                              0x0000, /* LPC2    */
                              0x0000, /* LPC3    */
                              0x0000, /* INT LPC */
                              0x0000, /* Mode    */ 
                              0x0000, /* Code1_1 */
                              0x0000, /* Code2_1 */
                              0x0000, /* GSP0_1  */
                              0x0000, /* Code1_2 */
                              0x0000, /* Code2_2 */
                              0x0000, /* GSP0_2  */
                              0x0000, /* Code1_3 */
                              0x0007, /* Code2_3 */  /* 3 bits */
                              0x0000, /* GSP0_3  */
                              0x007f, /* Code1_4 */  /* 7 bits */
                              0x007f, /* Code2_4 */  /* 7 bits */
                              0x0000, /* GSP0_4  */

                              0x0000, /* R0      */  /* voiced */
                              0x0000, /* LPC1    */
                              0x0000, /* LPC2    */
                              0x0000, /* LPC3    */
                              0x0000, /* INT LPC */
                              0x0000, /* Mode    */
                              0x0000, /* Lag_1   */
                              0x0000, /* Code_1  */
                              0x0000, /* GSP0_1  */
                              0x0000, /* Lag_2   */
                              0x0000, /* Code_2  */
                              0x0000, /* GSP0_2  */
                              0x0000, /* Lag_3   */
                              0x00ff, /* Code_3  */  /* 8 bits */
                              0x0000, /* GSP0_3  */
                              0x0000, /* Lag_4   */
                              0x01ff, /* Code_4  */  /* 9 bits */
                              0x0000}; /* GSP0_4 */

  static int first = 1;

  Shortword swMode, swBitMask;
  Shortword swSidN1, swSidN2, swSidN1pN2;
  Shortword swSid ;

  short siI, siII;


  if (first)
  {
    /* Force Sid codewords to be represented */
    /* internally in PC byte order           */
    /* ------------------------------------- */

    SwapBytes(ppswCL1pCL2[0], 18);
    SwapBytes(ppswCL1pCL2[1], 18);
    SwapBytes(ppswCL2[0], 18);
    SwapBytes(ppswCL2[1], 18);

    first = 0;
  }


  /* count transmission errors within the SID codeword      */
  /* count number of bits equal '0' within the SID codeword */
  /* ------------------------------------------------------ */

  if (pswParameters[5] == 0)
    swMode = 0;
  else 
    swMode = 1;
  

  swSidN1pN2 = 0;         /* N1 + N2 */
  swSidN2    = 0;
  swSidN1    = 0;
  
  for (siI = 0; siI < 18; siI++) {
      swBitMask = 0x0001;
      SwapBytes(&swBitMask, 1);  /* force swBitMask to PC byte order */
      for (siII = 0; siII < ppswIBit[swMode][siI]; siII++) {
        if ( (pswParameters[siI] & swBitMask) == 0 ) {
          if ( (ppswCL1pCL2[swMode][siI] & swBitMask) != 0 ) swSidN1pN2++;
          if ( (ppswCL2[swMode][siI] & swBitMask)     != 0 ) swSidN2++;
        }
        SwapBytes(&swBitMask, 1);  /* return swBitMask to native byte order */
        swBitMask = swBitMask << 1;
        SwapBytes(&swBitMask, 1);  /* force swBitMask to PC byte order */
      }
  }

  swSidN1 = swSidN1pN2 - swSidN2;


  /* frame classification */
  /* -------------------- */

  if (pswErrorFlag[2]) {

    if (swSidN1 < 3)
       swSid = 2;
    else if (swSidN1pN2 < 16)
       swSid = 1;
    else
       swSid = 0;
     
    if ( (swSidN1pN2 >= 16) && (swSidN1pN2 <= 25) ) {
      pswErrorFlag[0] = 1;     
    }

  }
  else {

    if (swSidN1 < 3)
       swSid = 2;
    else if (swSidN1pN2 < 11)
       swSid = 1;
    else
       swSid = 0;

  }


  /* in case of a mode mismatch */
  /*----------------------------*/
  
  if ( (swSid == 2) && (swMode == 0) ) swSid = 1;

  return(swSid);

}


/*___________________________________________________________________________
 |                                                                           |
 |     This subroutine sets the 18 speech parameters to random values        |
 |                                                                           |
 |     Input:     pswParameters[18]                                          |
 |                           input parameters of the speech decoder          |
 |                                                                           |
 |___________________________________________________________________________|
*/

static void RandomParameters(Shortword pswParameters[])
{
  static Shortword ppswIBit[2][18] = {
             5, 11,9,8, 1, 2, 7,7,5, 7,7,5, 7,7,5, 7,7,5,  /* unvoiced */
             5, 11,9,8, 1, 2, 8,9,5, 4,9,5, 4,9,5, 4,9,5}; /* voiced */

  static Longword L_PNSeed=(Longword)0x1091988L;
  Shortword  i,ind;

  /* Determine mode bit */
  /* ------------------ */
  pswParameters[5] = getPnBits(2, &L_PNSeed);

  /* Switch bit allocation accordingly */
  /* --------------------------------- */
  ind = 0;
  if (pswParameters[5] > 0) ind = 1;

  for (i=0; i < 5; i++){
     pswParameters[i] = getPnBits(ppswIBit[ind][i], &L_PNSeed);
  }
  for (i=6; i < 18; i++){
     pswParameters[i] = getPnBits(ppswIBit[ind][i], &L_PNSeed);
  }

  /* force random parameters to PC byte order */
  /* ---------------------------------------- */

  SwapBytes(pswParameters, 18);  
}

/*___________________________________________________________________________
 |                                                                           |
 |     Main - Program                                                        |
 |                                                                           |
 |___________________________________________________________________________|
*/
int main( int argc, char *argv[] )
{
    FILE *infile, *outfile;
    Shortword errpat, i = 0;

    if( argc < 4 || argc > 4 ) {
        fprintf( stderr, "\tUsage: REID input output EPx \n" );
        fprintf( stderr, "\tEPx: EP0\n" );
        fprintf( stderr, "\t     EP1 (not implemented)\n" );
        fprintf( stderr, "\t     EP2 (not implemented)\n" );
        fprintf( stderr, "\t     EP3 (not implemented)\n" );
        return( 1 );
    }


    if( !Strincmp( argv[3], "ep", 2 )) 
        errpat  = atoi( &argv[3][2] );

    printf( "  _____________________________________________\n" );
    printf( " |                                             |\n" );
    printf( " |       Residual Error Insertion Device       |\n" );
    printf( " |                 for                         |\n" );
    printf( " |       GSM Half-Rate Codec Simulation        |\n" );
    printf( " |                                             |\n" );
    printf( " |_____________________________________________|\n\n" );

    printf( "    Input File       : %s\n", argv[1] );
    printf( "    Output File      : %s\n", argv[2] );
    if( errpat ){
        printf( "    Error Pattern    : EP%d (not implemented)\n", errpat);
        return (1);
    }
    else
        printf( "    Error Pattern    : EP%d (error free)\n", errpat );
    printf( "\n" );

    infile  = OpenBinfile( argv[1], "r" );
    outfile = OpenBinfile( argv[2], "w" );


    if (errpat == 0) {
       for (i=0;i<6000;i++)
           if( error_free( infile, outfile)) break;
    }
    /*else
       for (i=0;i<6000;i++)
           if( residual_error_pattern( infile, outfile)) break;
        EP1-3 not implemented */

    fclose( infile );
    fclose( outfile );

    printf( " %d Frame%s processed      \n\n", i,( i != 1 ) ? "s" : "" ); 
    return( 0 );
}
