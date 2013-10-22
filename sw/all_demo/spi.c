/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/
#include  <stdio.h>

#include "stm32f10x.h"
#include <stm32f10x_spi.h>
#include "hw_config.h"
#include "spi.h"


/* ------------------------------------------------------------------------------------------------- */
/* BSP SPI */
/* ------------------------------------------------------------------------------------------------- */



/* ------------------------------------------------------------------------------------------------- */
/* extern USART */
/* ------------------------------------------------------------------------------------------------- */


// SPI speed setting function
// SpeedSet:
// SPI_SPEED_2 2 frequency (SPI 36M @ sys 72M)
// SPI_SPEED_4 4 frequency (SPI 18M @ sys 72M)
// SPI_SPEED_8 8 frequency (SPI 9M @ sys 72M)
// SPI_SPEED_16 16 frequency (SPI 4.5M @ sys 72M)
// SPI_SPEED_256 256 frequency (SPI 281.25K @ sys 72M)
void bsp_set_spi1_speed (u8 speed)
{
	SPI1->CR1 &= 0XFFC7; // Fsck = Fcpu/256
	switch (speed)
	{
		case SPI_SPEED_2: // Second division
			SPI1->CR1 |= 0<<3; // Fsck = Fpclk / 2 = 36Mhz
			break;
		case SPI_SPEED_4: // four-band
			SPI1-> CR1 |= 1<<3; // Fsck = Fpclk / 4 = 18Mhz
			break;
		case SPI_SPEED_8: // eighth of the frequency
			SPI1-> CR1 |= 2<<3; // Fsck = Fpclk / 8 = 9Mhz
			break;
		case SPI_SPEED_16: // sixteen frequency
			SPI1-> CR1 |= 3<<3; // Fsck = Fpclk/16 = 4.5Mhz
			break;
		case SPI_SPEED_256: // 256 frequency division
			SPI1-> CR1 |= 7<<3; // Fsck = Fpclk/16 = 281.25Khz
			break;
	}
	
	SPI1->CR1 |= 1<<6; // SPI devices enable
	
}

/*******************************************************************************
* Function Name: bsp_readwritebyte_spi1
* Description: SPI read and write a single byte (to return after sending the data read in this Newsletter)
* Input: u8 TxData the number to be sent
* Output: None
* Return: u8 RxData the number of received
*******************************************************************************/
u8 bsp_readwritebyte_spi1 (u8 tx_data)
{
	u8 retry=0;
	
	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus (SPI1, SPI_I2S_FLAG_TXE) == RESET)
	{
		retry++;
		if(retry>200)
			return 0;
	}

	/* Send byte through the SPI1 peripheral */
	SPI_I2S_SendData (SPI1, tx_data);

	retry=0;

	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus (SPI1, SPI_I2S_FLAG_RXNE) == RESET)
	{
		retry++;
		
		if(retry>200)
			return 0;
	}		

	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData (SPI1);
}



