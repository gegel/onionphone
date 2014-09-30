/* SpeedEdit 52,63,11,0,0,10,16,10 Updated 05/07/93 15:27:30 */
#include "drivers.h"

/* params.aic_regs.control:
    b0 = d2 - 0/1 deletes/inserts the A/D HPF
	b1 = d3 - 0/1 disables/enables the loopback function
	b2 = d4 - 0/1 disables/enables the AUX IN+ and AUX IN- pins
	b3 = d5 - 0/1 asynch/synch TX and RX sections
	b4 = d6 - 0/1 gain control bits
	b5 = d7 - 0/1 gain control bits
	b6 = d8 - unused
	b7 = d9 - 0/1 delete/insert on-board second-order sinx/x correction filter
*/

void set_aic8k(params, channel, rx_length, tx_length)
AIC_Attrs	*params;
int channel, rx_length, tx_length;
  {
#ifdef TURBO		
		printf("Serial port %d used for sampling AIC at 8000 samples/sec\n",channel);
#endif
		
  params->sync_isr = FALSE;		/* need asynch operation if RX_LEN != TX_LEN */
  params->tx_only = FALSE;
  params->rx_only = FALSE;
  params->tx_size = tx_length;
  params->rx_size = rx_length;
  params->chan_id = (channel == 1 ? 1 : 0);
  params->aic_regs.ra = 18;
  params->aic_regs.rb = 36;
  params->aic_regs.ta = 18;
  params->aic_regs.tb = 36;
  params->aic_regs.taP = 1;
  params->aic_regs.raP = 1;
  params->aic_regs.control = 0x0b9/* 0x099*/;
  }
	
	
  void set_aic16k(params, channel, rx_length, tx_length)
	AIC_Attrs	*params;
	int channel, rx_length, tx_length;
  {
#ifdef TURBO		
		printf("Serial port %d used for sampling AIC at 16,000 samples/sec\n",channel);
#endif
		
		params->sync_isr = TRUE;
		params->tx_only = FALSE;
		params->rx_only = FALSE;
		params->tx_size = tx_length;
		params->rx_size = rx_length;
		params->chan_id = (channel == 1 ? 1 : 0);
		params->aic_regs.ra = 18;
		params->aic_regs.rb = 18;
		params->aic_regs.ta = 18;
		params->aic_regs.tb = 18;
		params->aic_regs.taP = 1;
		params->aic_regs.raP = 1;
		params->aic_regs.control = 0x0a9;
  }
	
