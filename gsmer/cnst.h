/*--------------------------------------------------------------------------*
*       Codec constant parameters (coder, decoder, and postfilter)          *
*---------------------------------------------------------------------------*/

#define  L_TOTAL      320       /* Total size of w_speech buffer.             */
#define  L_WINDOW     240       /* Window size in LP analysis               */
#define  L_FRAME      160       /* Frame size                               */
#define  L_FRAME_BY2  80        /* Frame size divided by 2                  */
#define  L_SUBFR      40        /* Subframe size                            */
#define  M            10        /* Order of LP filter                       */
#define  MP1          (M+1)     /* Order of LP filter + 1                   */
#define  AZ_SIZE      (4*M+4)   /* Size of array of LP filters in 4 w_subfr.s */
#define  PIT_MIN      18        /* Minimum pitch lag                        */
#define  PIT_MAX      143       /* Maximum pitch lag                        */
#define  L_INTERPOL   (10+1)    /* Length of filter for interpolation       */

#define  PRM_SIZE     57        /* Size of vector of analysis parameters    */
#define  SERIAL_SIZE  (244+1)   /* bits per frame + bfi                     */

#define  MU       26214         /* Factor for tilt compensation filter 0.8  */
#define  AGC_FAC  29491         /* Factor for automatic gain control 0.9    */
