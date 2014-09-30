#ifndef _DRIVERS_
#define _DRIVERS_

#include <std.h>

/* data structure type define for TLC32044 registers */
typedef struct 
{
	Uns	    ra;
	Uns	    rb;
	Uns	    ta;
	Uns	    tb;
	Uns	    taP;
	Uns	    raP;
	Uns	    control;	
} TLC_Regs;

/* data structure type define for AIC driver Isr data object */	
typedef struct
{
	Uns	    size;		/* length of buffers					*/
	Uns	    count;		/* number of samples in current buffer	*/
	Ptr	    present;	/* pointer to 'current' working buffer	*/
	Ptr	    next;		/* pointer to next avalible free buffer	*/
	Bool	swap;		/* buffer swap flag (signal get/put)	*/
	Uns	    *xfer_reg;	/* C3x data tx or data rx register		*/
    Uns     isr_count;  /* interrupt counter                    */
} AIC_IsrObj;

/* data structure type define for AIC driver user paramsters data object */
typedef struct
{
	Bool    	sync_isr;		/* bool for common tx and rx isr's		*/
	Bool	    tx_only;		/* bool for only using tx opperations	*/
	Bool	    rx_only;		/* bool for only using rx opperations	*/
	Uns	        chan_id;		/* which AIC channel is being used		*/
	Uns	        tx_size;		/* size of buffers to put				*/
	Uns	        rx_size;		/* size of buffers to get				*/
	TLC_Regs	aic_regs;		/* TLC32044 registers data structure	*/
} AIC_Attrs;

/* data structure type define for AIC driver data object */
typedef struct
{
	Uns	        port;			/* which C3x serail port is being used	*/
	Uns	        rx_mask;		/* rx interrupt ie mask value			*/	
	Uns	        tx_mask;		/* tx interrupt ie mask value			*/
	Bool		tx_first;		/* flag for first call to AIC_get()		*/
	Bool		rx_first;		/* flag for first call to AIC_put()		*/
	AIC_Attrs	attrs;			/* user configurable parameters			*/ 
	AIC_IsrObj	rx_obj;			/* rx Isr data structure				*/
	AIC_IsrObj	tx_obj;			/* tx Isr data structure				*/
	AIC_IsrObj	*both_obj[2];	/* sync_isr tx/rx Isr data structure	*/
} AIC_DriverObj;
typedef AIC_DriverObj *AIC_Driver;

/* data structure type define for C3x serial port registers */
typedef struct
{
	Uns	    control;	/* global control	 	 		*/
	Uns	    space1;		/* C3x reserved space			*/
	Uns	    tx_ctrl;	/* FSX/DX/CLKX port control 	*/
	Uns	    rx_ctrl;	/* FSR/DR/CLKR port control		*/
	Uns	    time_ctrl;	/* R/X timer control			*/
	Uns	    time_cnt;	/* R/X timer count				*/
	Uns	    time_perd;	/* R/X timer period				*/
	Uns	    space2;		/* C3x reserved space			*/
	Uns	    dtr;		/* data transmit register		*/
	Uns	    space3;		/* C3x reserved space			*/
	Uns	    space4;		/* C3x reserved space			*/
	Uns	    space5;		/* C3x reserved space			*/
	Uns	    drr;		/* data receive register		*/		
} SER_Regs;

/* data structure type define for serial port driver Isr data object */	
typedef struct
{
	Uns	        size;		/* length of buffers					*/
	Uns	        count;		/* number of samples in current buffer	*/
	Ptr		    present;	/* pointer to 'current' working buffer	*/
	Ptr         next;		/* pointer to next avalible free buffer	*/
	Bool		swap;		/* buffer swap flag (signal get/put)	*/
	Uns	        *xfer_reg;	/* C3x data tx or data rx register		*/
    Uns         isr_count;  /* interrupt counter                    */
    Uns         isr_size;   /* SER frame count for clock comp       */
    Uns         isr_length; /* # num of int's to check for clk cmp  */
    AIC_IsrObj  *aic_obj;   /* pointer to aic isr obj for clock cmp */
	Uns	        mask;   	/* tx/rx interrupt ie mask value		*/
	Uns	        buff_aval;	/* next tx buffer avalible flag			*/
} SER_IsrObj;

/* data structure type define for serial driver user paramsters data object */
typedef struct
{
	Bool		tx_only;		/* bool for only using tx opperations	*/
	Bool		rx_only;		/* bool for only using rx opperations	*/
	Uns	        chan_id;		/* which serial channel is being used	*/
	Uns	        tx_size;		/* size of buffers to put				*/
	Uns	        rx_size;		/* size of buffers to get				*/
   	Bool		tx_clk_comp; 	/* bool for SER tx clock compensation  	*/
	Bool		rx_clk_comp;   	/* bool for SER rx clock compensation   */
	Uns	        rx_clk_cnt;		/* clock compensation rx frame count    */
	Uns	        tx_clk_cnt;		/* clock compensation tx frame count    */
	SER_Regs	ser_regs;		/* C3x serial port data structure		*/
} SER_Attrs;

/* data structure type define for serial port driver data object */
typedef struct
{
	Uns	        port;			/* which C3x serail port is being used	*/
	Bool		rx_first;		/* flag for first call to SER_get()		*/
	Bool		tx_first;		/* flag for first call to SER_put()		*/
    SER_Attrs	attrs;			/* user configurable parameters			*/ 
	SER_IsrObj	rx_obj;			/* rx Isr data structure				*/
	SER_IsrObj	tx_obj;			/* tx Isr data structure				*/
} SER_DriverObj;
typedef SER_DriverObj *SER_Driver;	

extern Void null_isr(Void);
extern Void set_isr(Uns vector, Void isr(Void), Arg argument);
extern Void fix_buffer(Ptr buffer, Uns size);
extern Void float_buffer(Ptr buffer, Uns size);
extern Void scale_buffer(Float *input, Float *output, Uns size, Float scale);
extern Void enable_int(Uns mask);
extern Void prime_int(Uns mask);
extern Void disable_int(Uns mask);
extern Void global_enable(Bool flag);

#define AIC_getattrs(AICOBJ, AICATTRS) (*(AICATTRS) = (AICOBJ)->attrs)
#define AIC_setattrs(AICOBJ, AICATTRS) ((AICOBJ)->attrs = *(AICATTRS)) 
extern AIC_Attrs AIC_ATTRS;
extern AIC_Driver AIC_create(AIC_Attrs *attrs);
extern Void AIC_delete(AIC_Driver driver);
extern Bool AIC_get(AIC_Driver driver, Ptr *buffer);
extern Bool AIC_put(AIC_Driver driver, Ptr *buffer);
extern Void AIC_tx_isr(Void);
extern Void AIC_rx_isr(Void);
extern Void AIC_both_isr(Void);

#define SER_getattrs(SEROBJ, SERATTRS) (*(SERATTRS) = (SEROBJ)->attrs)
#define SER_setattrs(SEROBJ, SERATTRS) ((SEROBJ)->attrs = *(SERATTRS)) 
extern SER_Attrs SER_ATTRS;
extern SER_Driver SER_create(SER_Attrs *attrs);
extern Void SER_delete(SER_Driver driver);
extern Bool SER_get(SER_Driver driver, Ptr *buffer);
extern Bool SER_put(SER_Driver driver, Ptr *buffer);
extern Void SER_tx_isr(Void);
extern Void SER_rx_isr(Void);
extern Void SER_link_AIC(SER_Driver ser_drv, AIC_Driver aic_drv, SER_Attrs *attrs);

#endif /* _DRIVERS_ */
 
