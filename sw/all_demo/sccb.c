
#include "sccb.h"
#include "ov7670.h"


void DelaySCCB(void)
{
	volatile int i;
	for(i=0;i<1500;i++);	
}

void InitSCCB(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//TXZZSCCB_DDR|=(1<<SCCB_SIO_C)|(1<<SCCB_SIO_D);
	//TXZZSCCB_PORT|=(1<<SCCB_SIO_C)|(1<<SCCB_SIO_D);
	GPIO_InitStructure.GPIO_Pin = SCCB_SCL_PIN | SCCB_SDA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIOE->BSRR =((1<<SCCB_SIO_D) | (1<<SCCB_SIO_C));


	// XCLK핀은 받드시 PA8을 사용해야 합니다.
	// PA8에만 Alternate function으로 MCO기능이됨.
	GPIO_InitStructure.GPIO_Pin = CAMERA_XCLK_PIN; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
	GPIO_Init(CAMERA_XCLK_PORT, &GPIO_InitStructure);

#if 0
#ifdef STM32F10X_CL
	RCC_MCOConfig(RCC_MCO_HSI  );//hsi
#else
	RCC_MCOConfig(RCC_MCO_HSE  );//hsi
#endif
#endif

	RCC_MCOConfig(RCC_MCO_HSE  );//HSE=25000000Hz

                        
}

/*
-----------------------------------------------
   Function: start command, SCCB the start signal
   Parameters: None
 Return Value: None
-----------------------------------------------
*/

void startSCCB(void)
{
    SIO_D_SET;     // data line high
    DelaySCCB();

    SIO_C_SET;    // When the clock line high data line high to low
    DelaySCCB();

    SIO_D_CLR;
    DelaySCCB();

    SIO_C_CLR;	 // data line restoration of low, single action function is necessary
    DelaySCCB();

}
/*
-----------------------------------------------
	 Function: stop command, SCCB the stop signal
	 Parameters: None
	 Return Value: None
-----------------------------------------------
*/
void stopSCCB(void)
{
    SIO_D_CLR;
    DelaySCCB();

    SIO_C_SET;
    DelaySCCB();

    SIO_D_SET;
    DelaySCCB();

}

/*
-----------------------------------------------
	Function: noAck, for continuous reading of the final end of the cycle
	Parameters: None
	Return Value: None
-----------------------------------------------
*/
void noAck(void)
{
	DelaySCCB();
	SIO_D_SET;
	DelaySCCB();
	
	SIO_C_SET;
	DelaySCCB();
	
	SIO_C_CLR;
	DelaySCCB();
	
	SIO_D_CLR;
	DelaySCCB();
}

/*
-----------------------------------------------
	Function: write one byte of data to the SCCB
	Parameters: write data
	Returns: send successful return 1 return 0 send failed
-----------------------------------------------
*/
u8 SCCBwriteByte(u8 m_data)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	unsigned char j,tem;
	for(j=0;j<8;j++) // loop 8 times to send data
	{
		if((m_data<<j)&0x80)
		{
			SIO_D_SET;
		}
		else
		{
			SIO_D_CLR;
		}
		DelaySCCB();
		SIO_C_SET;
		DelaySCCB();
		SIO_C_CLR;
		DelaySCCB();
	}
	DelaySCCB();
	
	SIO_D_IN;/* Set SDA as input */
	DelaySCCB();
	SIO_C_SET;
	DelaySCCB();
	DelaySCCB();
	DelaySCCB();
	
	if(SIO_D_STATE)
	{
		tem=0;   // SDA = 1 this fails, return 0
	}
	else
	{
		tem=1;   // SDA = 0 sent successfully, return 1
	}
	SIO_C_CLR;
	DelaySCCB();
	
	SIO_D_OUT;/* Set SDA as output */
	return(tem);  
}

/*
-----------------------------------------------
	Function: read a byte of data and returns
	Parameters: None
	Returns: the data read
-----------------------------------------------
*/
u8 SCCBreadByte(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	unsigned char read,j;
	read=0x00;
	
	SIO_D_IN;/* Set SDA as input */
	
	for(j=8;j>0;j--) // loop 8 times receiving data
	{		     
		DelaySCCB();
		SIO_C_SET;
		DelaySCCB();
		DelaySCCB();
		DelaySCCB();// data has been created
		read=read<<1;
		if(SIO_D_STATE) 
		{
			read=read+1;
		}
		SIO_C_CLR;
	}
	SIO_D_OUT;/* Set SDA as output */
	
	return(read);
}

