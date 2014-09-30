/* SpeedEdit 184,196,24,0,0,10,16,10 Updated 08/10/92 09:10:50 */
#include <stdio.h>
#include "lpcdefs.h"
#include "filter.h"

#ifdef NN_VOICE

#define NFEATURE	3		/* number of features per voicing frame */
#define NICLASS 	3*NFEATURE	/* number of inputs to the neural net classifier */
#define LPF_LENG	3		/* length of low pass filter used to estimate average v & uv energy */
#define MAXDB		72.25		/* scale factor to normalize speech level */
					/* 20.0*log10(4096), where 4096 = max signal amplitude */
#define V_NOMINAL	0.6		/* nominal value of Elow_v_hat */
#define UV_NOMINAL	0.3		/* nominal value of Elow_uv_hat */
#define NFRAMES_ELOW	440		/* number of (half) frames of Elow history to save */
#define BUF_FRACT	.1		/* fraction of Elow_buf used to determine El and Eh */
#define THRESH		.2		/* threshhold for Eh - El to declare speech present in Elow_buf */
#define NHIDDEN 	2		/* number of hidden units used in the neural net classifier */

/*     ----------------------------------------  CLASSIFY  ----------------------------------------
*/

static float classify(float x[])
{
   int i;
   float out[NHIDDEN+1];
   static float bias[] =    {	 4.7605f-01, -4.4143f-01, -2.5532f+01	};
   static float weight0[] = {	 0.0000f+00,  9.9600f+00,  1.3399f+00	};	/* weight0[0] is not used */
   static float weight1[] = {	 2.5327f-01,  5.1214f-02,  5.1845f-02,
				 8.7389f-01,  6.6049f-02,  5.6143f-02,
				 2.1496f-02,  6.4269f-02,  7.5030f-03	};
   static float weight2[] = {	 9.1907f-01,  1.4349f+00, -5.3130f-03,
				-2.1947f+00,  2.5551f+01,  4.0398f-01,
				-1.9285f-01, -9.4621f-01,  7.7245f-01	};

   /*	hidden unit 1	*/
   out[1] = bias[1];
   for (i=0; i<NICLASS; i++) out[1] += weight1[i] * x[i];
   out[1] = (float) tanh((double) out[1]);

   /*	hidden unit 2	*/
   out[2] = bias[2];
   for (i=0; i<NICLASS; i++) out[2] += weight2[i] * x[i];
   out[2] = (float) tanh((double) out[2]);

   /*	output unit   */
   out[0] = bias[0];
   for (i=1; i<=NHIDDEN; i++) out[0] += weight0[i] * out[i];

   return (float) tanh((double) out[0]);
}

/*     ----------------------------------------  AUTOCOR  ----------------------------------------
*/

static float autocor(float signal[], int n, int lag)
{
   int i, p1, p2;
   float cor1, cor2, cor3;

   /*p1 = lag / 2;				   to be centered in signal window, go back half of lag */
   p1 = lag >> 1;				/* to be centered in signal window, go back half of lag */
   p2 = lag - p1;				/* and forward the difference */
   cor1 = cor2 = cor3 = 0.0;
   for (i=0; i<n;  i++)
   {
      cor1 += signal[i-p1] * signal[i+p2];
      cor2 += signal[i-p1] * signal[i-p1];
      cor3 += signal[i+p2] * signal[i+p2];
   }
   if (cor2 == 0.0 || cor3 == 0.0) return 0.0;
   else 			   return (cor1 * cor1) / (cor2 * cor3);	/* range:  0 to +1 */
}

/*     ----------------------------------------  SINSERT  ----------------------------------------

     This routine re-sorts a buffer which has a single (new) entry (perhaps) out of place.
'sbuf' is the array sorted in ascending order; 'obuf' is another array to be re-ordered in the same manner;
'k' is the index of the newly inserted entry.
*/

static void sinsert(float sbuf[], int obuf[], int n, int k)
/*  sbuf[0:n-1]  */
/*  obuf[0:n-1]  */
{
   int i, j, tempi;
   float temp;

   if (k < n-1 && sbuf[k] > sbuf[k+1])				/* is new datum inserted too low in buffer? */
   {
      for (i=k+2; i<n; i++) if (sbuf[k] <= sbuf[i]) break;	/* i points to value to follow sbuf[k] */
      temp  = sbuf[k];
      tempi = obuf[k];
      for (j=k; j<i-1; j++)
      {
	 sbuf[j] = sbuf[j+1];
	 obuf[j] = obuf[j+1];
      }
      sbuf[i-1] = temp;
      obuf[i-1] = tempi;
   }
   else if (k > 0 && sbuf[k] < sbuf[k-1])			/* is new datum inserted too high in buffer? */
   {
      for (i=k-2; i>=0; i--) if (sbuf[k] >= sbuf[i]) break;	/* i points to value to precede sbuf[k] */
      temp  = sbuf[k];
      tempi = obuf[k];
      for (j=k; j>i+1; j--)
      {
	 sbuf[j] = sbuf[j-1];
	 obuf[j] = obuf[j-1];
      }
      sbuf[i+1] = temp;
      obuf[i+1] = tempi;
   }
}

/*     ----------------------------------------  VFEATURES  ----------------------------------------
*/

static void vfeatures(float speech[], float lpspeech[], int length, int pitch, float features[])
{
   int i, nframes;
   float Elow, Elown, Flow, Cpch;
   float ss_low, ss_full, temp, El, Eh, Ethresh;

   static int first = 1;				/* flag to indicate first execution of this routine */
   static int indx = 0;
   static float Elow_uv_hat;				/* estimate of average Elow when speech is absent */
   static float Elow_v_hat;				/* estimate of average Elow when speech is present  */
   static float avg_num[] = { 1.60283f-4, 3.20566f-4, 1.60283f-4 };
   static float avg_den[] = { 1.0f,	  -1.949359f,	0.95f };
   static float Elow_buf[NFRAMES_ELOW]; 		/* the past NFRAMES_ELOW of Elow values */
   static int Elow_order[NFRAMES_ELOW];
   static FILTER *uvnf, *vnf;
   static float uvnf_state[] = { 468.053f, 468.053f };
   static float  vnf_state[] = { 936.107f, 936.107f };

   if (first)							/* initialize on first pass */
   {
      first = 0;

      /*   create the two filters used to estimate Elow when speech absent and present	 */
      uvnf = filter_create(avg_num, LPF_LENG, avg_den, LPF_LENG);
      vnf  = filter_create(avg_num, LPF_LENG, avg_den, LPF_LENG);

      /*   get the averaging filter outputs to nominal values	*/
      filter_state_set(uvnf, 2, uvnf_state);
      filter_state_set( vnf, 2,  vnf_state);
      Elow_v_hat  = filter(vnf,   V_NOMINAL);
      Elow_uv_hat = filter(uvnf, UV_NOMINAL);

      /*   initialize Elow_buf to nominal values   */
      for (i=0; i<NFRAMES_ELOW; i++)
      {
	 if (i < .5 * NFRAMES_ELOW) Elow_buf[i] = (float) UV_NOMINAL;
	 else			    Elow_buf[i] = (float) V_NOMINAL;
	 Elow_order[i] = i;
      }
   }

   /*	Elow:  low band energy	 */
   ss_low = 0.0;
   for (i=0; i<length;	i++) ss_low += lpspeech[i] * lpspeech[i];
   Elow = ss_low / length;
   if (Elow > 0.0) Elow = (float) (10.0 * log10((double) Elow) - MAXDB);	/* convert to dB; fully loaded = 0 dB */
   else 	   Elow = -100.0;				/* -100 dB if zero input */
   if (Elow < -100.0) Elow = -100.0;				/* clamp negative end of range */
   /*Elow = (Elow / 50.0) + 1.0;			    	   -100 dB to 0 dB is scaled -1.0 to +1.0 */
   Elow = (float) ((Elow * 20e-3) + 1.0);			/* -100 dB to 0 dB is scaled -1.0 to +1.0 */

   /*	Determine the average speaking level   */
   for (i=0; i<NFRAMES_ELOW; i++) if (Elow_order[i] == indx) break;	/* i = index of oldest Elow_buf sample */
   Elow_buf[i] = Elow;						/* replace oldest sample with newest */
   sinsert(Elow_buf, Elow_order, NFRAMES_ELOW, i);		/* re-sort buffers after insertion at index i */
/**   indx = ++indx % NFRAMES_ELOW;				   increment indx modulo NFRAMES_ELOW */
   indx++;
   if(indx == NFRAMES_ELOW) indx = 0;
   El = Eh = 0.0;
   nframes = (int) (BUF_FRACT * NFRAMES_ELOW);
   for (i=0; i<nframes; i++)
   {
      El += Elow_buf[i];					/* sum the lowest  BUF_FRACT of Elow_buf */
      Eh += Elow_buf[NFRAMES_ELOW-1 - i];			/* sum the highest BUF_FRACT of Elow_buf */
   }
   /*El /= nframes;*/
   El *= 0.02272727272f;
   /*Eh /= nframes;*/
   Eh *= 0.02272727272f;
   if (Eh - El > THRESH) Ethresh = (float) (.5 * (El + Eh));    /* if it's likely that Elow_buf has speech */
   else 		 Ethresh = (float) (.5 * (Elow_v_hat + Elow_uv_hat));
   if (Elow > Ethresh) Elow_v_hat  = filter( vnf, Elow);	/* speech present this half-frame */
   else 	       Elow_uv_hat = filter(uvnf, Elow);	/* speech absent  this half-frame */

   /*	Elown:	Elow normalized   */
   Elown = (float) (Elow - Elow_v_hat + V_NOMINAL);

   /*	Flow:  fraction of full band energy in low band   */
   ss_full = 0.0;
   for (i=0; i<length;	i++) ss_full += speech[i] * speech[i];
   if (ss_full > 0.0) Flow = ss_low / ss_full;
   else 	      Flow = 0.0;
   if (Flow > 1.0) Flow = 1.0;					/* this happens quite often! */
   Flow = (float) (2.0 * Flow - 1.0);			/* range: -1 to +1 */

   /*	Cpch:  autocorrelation of LPF(speech) @ one pitch period   */
   Cpch = -2.0;
   for (i=-1; i<=1; i++)
   {
      temp = (float) (2.0 * autocor(lpspeech, length, pitch+i) - 1.0);	/* range: -1 to +1 */
      if (temp > Cpch) Cpch = temp;
   }

   features[0] = Elown;
   features[1] = Flow;
   features[2] = Cpch;
}

/*     ----------------------------------------  VOICING  ----------------------------------------

Determines two half-frame voicing decisions for the frame speech[start:end].
*/

void voicing(float speech[], float lpspeech[], int start, int end, int pitch, int *v1, int *v2)
{
   int i, vfleng;
   float vclass;
   static float fbuf[4*NFEATURE] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

   /* shift the feature buffer by two voicing frames */
   for (i=0; i<2*NFEATURE; i++) fbuf[i] = fbuf[i + 2*NFEATURE];

   /*vfleng = (end - start + 1) / 2;				   voicing frame length */
   vfleng = (end - start + 1) >> 1;				/* voicing frame length */

   vfeatures(speech+start+vfleng,   lpspeech+start+vfleng,   vfleng, pitch, fbuf+6);
   vfeatures(speech+start+2*vfleng, lpspeech+start+2*vfleng, vfleng, pitch, fbuf+9);

   /* make the voicing classification for the first half frame */
   *v1 = *v2 = 0;
   vclass = classify(fbuf);
   if (vclass >= 0.0) *v1 = 1;

   /* make the voicing classification for the second half frame */
   vclass = classify(fbuf+NFEATURE);
   if (vclass >= 0.0) *v2 = 1;
}
#endif
