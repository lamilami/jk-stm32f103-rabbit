
#include  <stm32f10x.h>
#include <stdlib.h>
#include <math.h>

#include "hw_config.h"
#include "touch.h" 
#include "lcd.h"
#include "usart.h"


#ifdef ADJ_SAVE_ENABLE	
#include "24cxx.h"	 
#endif

Pen_Holder Pen_Point;

// SPI write data
// Write 1byte to the 7843 data
void ADS_Write_Byte(u8 num)    
{  
	u8 count=0;   
	for(count=0;count<8;count++)  
	{ 	  
		if(num&0x80)TDIN=1;  
		else TDIN=0;   
		num<<=1;    
		TCLK=0;
		TCLK=1;      
	} 			    
}

// SPI read data
// Read adc value from the 7846/7843/XPT2046/UH7843/UH7846	   
u16 ADS_Read_AD(u8 CMD)	  
{ 	 
	u8 count=0; 	  
	u16 Num=0; 
	TCLK=0; // first down the clock
	TCS=0;  // select ADS7843	 
	ADS_Write_Byte(CMD); // send the command word
	
	delay_us(6); //ADS7846 conversion period of up to 6us
	TCLK=1; // clear the BUSY
	TCLK=0;
	
	for(count=0;count<16;count++)  
	{ 				  
		Num<<=1; 	 
		TCLK=0; // falling edge
		TCLK=1;
		if(DOUT)Num++; 		 
	}  	
	Num>>=4;   // only the high 12 bits effective.
	TCS=1; // release the ADS7843
	return(Num);   
	
}

// Read the value of a coordinate
// Sequential read READ_TIMES time data, the data in ascending order,
// Then remove the minimum and maximum LOST_VAL number of averagedµ 
#define READ_TIMES 15 // reads
#define LOST_VAL 5	  // discard value
u16 ADS_Read_XY(u8 xy)
{
	u16 i, j;
	u16 buf[READ_TIMES];
	u16 sum=0;
	u16 temp;
	for(i=0;i<READ_TIMES;i++)
	{				 
		buf[i]=ADS_Read_AD(xy);	    
	}				    
	for(i=0;i<READ_TIMES-1; i++) // sort
	{
		for(j=i+1;j<READ_TIMES;j++)
		{
			if(buf[i]>buf[j]) // ascending order
			{
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}	  
	sum=0;
	for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum+=buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	return temp;   
} 

// Read the coordinates with Filter
// Minimum value of not less than 100.
u8 Read_ADS(u16 *x,u16 *y)
{
	u16 xtemp,ytemp;			 	 		  
	xtemp=ADS_Read_XY(CMD_RDX);
	ytemp=ADS_Read_XY(CMD_RDY);	  												   
	if(xtemp<100||ytemp<100)return 0; // read failure
	*x=xtemp;
	*y=ytemp;
	return 1; // reading success
}	
// 2 times read ADS7846, sequential read 2 times the value of effective AD, and this deviation can not exceed two
// 50, to meet the conditions, then that reading is correct, otherwise the reading error.
// This function can greatly increase the accuracy
#define ERR_RANGE 50 // error range 
u8 Read_ADS2(u16 *x,u16 *y) 
{
	u16 x1,y1;
 	u16 x2,y2;
 	u8 flag;    
    flag=Read_ADS(&x1,&y1);   
    if(flag==0)return(0);
    flag=Read_ADS(&x2,&y2);	   
    if(flag==0)return(0);   
    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE)) // before and after the two samples within the +-50
    &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }else return 0;	  
} 

// Read the first coordinates
// Only read once, that release before returning PEN!				   
u8 Read_TP_Once(void)
{
	u8 t=0;	    
	Pen_Int_Set(0); // turn off interrupts
	Pen_Point.Key_Sta=Key_Up;
	Read_ADS2(&Pen_Point.X,&Pen_Point.Y);
	while(PEN==0&&t<=250)
	{
		t++;
		delay_ms(10);
	};
	Pen_Int_Set(1); // Open interrupt
	if(t>=250)return 0; // Press 2.5s considered invalid
	else return 1;	
}

//////////////////////////////////////////////////
// Function with the LCD part of the
// Draw a touch point
// Used for calibration
void Drow_Touch_Point(u8 x,u16 y)
{
	LCD_DrawLine(x-12,y,x+13,y); // horizontal line
	LCD_DrawLine(x,y-12,x,y+13); // vertical line
	LCD_DrawPoint(x+1,y+1);
	LCD_DrawPoint(x-1,y+1);
	LCD_DrawPoint(x+1,y-1);
	LCD_DrawPoint(x-1,y-1);
	Draw_Circle(x,y,6); // draw center circle
}	  

// Draw a big point
// 2 * 2 points			   
void Draw_Big_Point(u8 x,u16 y)
{	    
	LCD_DrawPoint(x,y); // center point
	LCD_DrawPoint(x+1,y);
	LCD_DrawPoint(x,y+1);
	LCD_DrawPoint(x+1,y+1);	 	  	
}
//////////////////////////////////////////////////

// Convert results
// According to a touch screen calibration parameters to determine the conversion result stored in X0, Y0 in
void Convert_Pos(void)
{		 	  
	if(Read_ADS2(&Pen_Point.X,&Pen_Point.Y))
	{
		Pen_Point.X0=Pen_Point.xfac*Pen_Point.X+Pen_Point.xoff;
		Pen_Point.Y0=Pen_Point.yfac*Pen_Point.Y+Pen_Point.yoff;  
	}
}	   

// Interrupt pin detects a falling edge of the PEN.
// Set Pen_Point.Key_Sta is depressed
// Line break in the detection of broken 0
void EXTI1_IRQHandler(void)
{ 		   			 
	Pen_Point.Key_Sta=Key_Down; // button pressed
	// EXTI->PR=1<<1;  // Clear interrupt flag on LINE1

	EXTI_ClearITPendingBit(EXTI_Line1);			
} 

// PEN interrupt settings
void Pen_Int_Set(u8 en)
{
	if(en)
		EXTI->IMR|=1<<1;     // turn on interrupts on line1 	
	else 
		EXTI->IMR&=~(1<<1); // turn off interrupts on line1 	
}	  
//////////////////////////////////////////////////////////////////////////
// This section involves the use of an external EEPROM, if no external EEPROM, can be screened in this section
#ifdef ADJ_SAVE_ENABLE
// Stored in the EEPROM address range inside the base, taking up 13 bytes (RANGE: SAVE_ADDR_BASE ~ SAVE_ADDR_BASE +12)
#define SAVE_ADDR_BASE 40

// Save the calibration parameters
void Save_Adjdata(void)
{
	s32 temp;			 

	temp=Pen_Point.xfac*100000000; // Save x correction factor
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE,temp,4);   
	temp=Pen_Point.yfac*100000000;//
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE+4,temp,4);

    AT24CXX_WriteLenByte(SAVE_ADDR_BASE+8,Pen_Point.xoff,2);		    

	AT24CXX_WriteLenByte(SAVE_ADDR_BASE+10,Pen_Point.yoff,2);	
			     							 
	temp=AT24CXX_ReadOneByte(SAVE_ADDR_BASE+12);
	temp&=0XF0;
	temp|=0X0A;
	AT24CXX_WriteOneByte(SAVE_ADDR_BASE+12,temp);			 
}

u8 Get_Adjdata(void)
{					  
	s32 tempfac;
	tempfac=AT24CXX_ReadOneByte(52);
	if((tempfac&0X0F)==0X0A)
	{    												 
		tempfac=AT24CXX_ReadLenByte(40,4);		   
		Pen_Point.xfac=(float)tempfac/100000000;
		tempfac=AT24CXX_ReadLenByte(44,4);			          
		Pen_Point.yfac=(float)tempfac/100000000;

		tempfac=AT24CXX_ReadLenByte(48,2);			   	  
		Pen_Point.xoff=tempfac;					 

		tempfac=AT24CXX_ReadLenByte(50,2);				 	  
		Pen_Point.yoff=tempfac;					 
		return 1;	 
	}
	return 0;
}
#endif		 

// Touch screen calibration code
// Get the four calibration parameters
void Touch_Adjust (void)
{
	u16 pos_temp[4][2]; // coordinates of the cache value
	u8 cnt = 0;
	u16 d1, d2;
	u32 tem1, tem2;
	float fac;
	
	cnt = 0;
	POINT_COLOR = BLUE;
	BACK_COLOR = WHITE;
	
	LCD_Clear(WHITE); // clear the screen
	POINT_COLOR = RED; // red
	
	LCD_Clear(WHITE); // clear the screen
	Drow_Touch_Point(20,20); // draw point 1
	Pen_Point.Key_Sta = Key_Up; // elimination of the trigger signal
	Pen_Point.xfac = 0; // xfac used to mark whether or calibrated, so the calibration must be cleared away before! To avoid errors
	
	while (1)
	{
		if (Pen_Point.Key_Sta == Key_Down) // button pressed
		{
			if (Read_TP_Once ())// get the value of a single button
			{
				pos_temp[cnt][0] = Pen_Point.X;
				pos_temp[cnt][1] = Pen_Point.Y;
				cnt++;
			}

			switch (cnt)
			{
			case 1:
				LCD_Clear(WHITE); // clear the screen
				Drow_Touch_Point(220,20); // draw point 2
				break;
			case 2:
				LCD_Clear(WHITE); // clear the screen
				Drow_Touch_Point(20,300); // draw point 3
				break;
			case 3:
				LCD_Clear(WHITE); // clear the screen
				Drow_Touch_Point(220,300); // draw point 4
				break;
			case 4: // all of the four points have been
				// On the same side
				tem1 = abs (pos_temp[0][0]-pos_temp[1][0]);// x1-x2
				tem2 = abs (pos_temp[0][1]-pos_temp[1][1]);// y1-y2
				tem1 *= tem1;
				tem2 *= tem2;
				d1 = sqrt (tem1 + tem2); // get the distance 1,2

				tem1 = abs (pos_temp[2][0]-pos_temp[3][0]);// x3-x4
				tem2 = abs (pos_temp[2][1]-pos_temp[3][1]);// y3-y4
				tem1 *= tem1;
				tem2 *= tem2;
				d2 = sqrt(tem1 + tem2); // get the distance 3,4
				fac = (float) d1/d2;
				
				if (fac < 0.95 || fac > 1.05 || d1 == 0 || d2 == 0) // fail
				{
					cnt = 0;
					LCD_Clear (WHITE); // clear the screen
					Drow_Touch_Point(20,20);
					continue;
				}
				
				tem1 = abs (pos_temp[0][0]-pos_temp[2][0]);// x1-x3
				tem2 = abs (pos_temp[0][1]-pos_temp[2][1]);// y1-y3
				tem1 *= tem1;
				tem2 *= tem2;
				d1 = sqrt (tem1 + tem2); // get the distance 1,3

				tem1 = abs (pos_temp[1][0]-pos_temp[3][0]);// x2-x4
				tem2 = abs (pos_temp[1][1]-pos_temp[3][1]);// y2-y4
				tem1 *= tem1;
				tem2 *= tem2;
				d2 = sqrt (tem1 + tem2); // get distance of 2,4
				fac = (float) d1/d2;
				
				if (fac < 0.95 || fac > 1.05) // fail
				{
					cnt = 0;
					LCD_Clear (WHITE); // clear the screen
					Drow_Touch_Point(20,20);
					continue;
				} // Correct the

				// Diagonal equal
				tem1 = abs (pos_temp[1][0]-pos_temp[2][0]);// x1-x3
				tem2 = abs (pos_temp[1][1]-pos_temp[2][1]);// y1-y3
				tem1 *= tem1;
				tem2 *= tem2;
				d1 = sqrt (tem1 + tem2); // get the distance of 1,4

				tem1 = abs (pos_temp[0][0]-pos_temp[3][0]);// x2-x4
				tem2 = abs (pos_temp[0][1]-pos_temp[3][1]);// y2-y4
				tem1 *= tem1;
				tem2 *= tem2;
				d2 = sqrt (tem1 + tem2); // get the distance 2,3
				fac = (float) d1/d2;
				
				if (fac < 0.95 || fac > 1.05) // fail
				{
					cnt = 0;
					LCD_Clear(WHITE); // clear the screen
					Drow_Touch_Point(20,20);
					continue;
				} // Correct the
				
				// Results
				Pen_Point.xfac = (float) 200/(pos_temp[1][0]-pos_temp[0][0]);// be xfac
				Pen_Point.xoff = (240-Pen_Point.xfac * (pos_temp[1][0] + pos_temp[0][0])) / 2; // by xoff

				Pen_Point.yfac = (float) 280/(pos_temp [2] [1]-pos_temp [0] [1 ]);// be yfac
				Pen_Point.yoff = (320-Pen_Point.yfac * (pos_temp[2][1] + pos_temp[0][1])) / 2; // get yoff
				POINT_COLOR = BLUE;
				LCD_Clear(WHITE); // clear the screen
				LCD_ShowString(35,110, "Touch Screen Adjust OK !");// calibration is completed,
				delay_ms(1000);
				LCD_Clear(WHITE); // clear the screen
				return; // calibration is completed,
			}
		}
	}
}

void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group)	 
{ 
	u32 temp,temp1;	  
	temp1=(~NVIC_Group)&0x07;
	temp1<<=8;
	temp=SCB->AIRCR;
	temp&=0X0000F8FF;
	temp|=0X05FA0000;
	temp|=temp1;	   
	SCB->AIRCR=temp; 
}

void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group)	 
{ 
	u32 temp;	
	u8 IPRADDR=NVIC_Channel/4; 
	u8 IPROFFSET=NVIC_Channel%4;
	IPROFFSET=IPROFFSET*8+4;    
	MY_NVIC_PriorityGroupConfig(NVIC_Group);
	temp=NVIC_PreemptionPriority<<(4-NVIC_Group);	  
	temp|=NVIC_SubPriority&(0x0f>>NVIC_Group);
	temp&=0xf;

	if(NVIC_Channel<32)
		NVIC->ISER[0]|=1<<NVIC_Channel;
	else 
		NVIC->ISER[1]|=1<<(NVIC_Channel-32);

	NVIC->IP[IPRADDR] |= temp<<IPROFFSET;
}

// External interrupt initialization function
void bsp_touch_Init(void)
{
#if 1
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;	
	
	/* Configure SPI1 pins: SCK, MOSI and CS */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_3 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Configure PC1, Touch PEN interrupt */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOC, &GPIO_InitStructure);	

	/* Configure MISO interrupt */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOC, &GPIO_InitStructure);		

#if 0	
	// Note that the clock is enabled after the operation is valid for GPIO
	// So the pull, you must enable the clock. To achieve a real pull-out
	RCC->APB2ENR |= 1 <<4; // PC clock enable
	RCC->APB2ENR |= 1 <<0; // Open the secondary clock
	GPIOC->CRL &= 0XFFFF0000; // PC0 ~ 3
	GPIOC->CRL |= 0X00003883;
	GPIOC->CRH &= 0XFF0FFFFF; // PC13
	GPIOC->CRH |= 0X00300000; // PC13 push-pull output
	GPIOC->ODR |= 0X200f; // PC0 ~ 3 13 Full Pull
#endif	
	
	Read_ADS(&Pen_Point.X, &Pen_Point.Y); // initialize the first reading

	/* Connect EXTI */	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource1);	

	/* Configure EXTI  to generate an interrupt on falling edge */
	EXTI_InitStructure.EXTI_Line = EXTI_Line1;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);		
	
	//MY_NVIC_Init(2,0, EXTI1_IRQChannel, 2);
	/* 2 bit for pre-emption priority, 2 bits for subpriority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);			

	// interrupt disable
	/* Clear EXTI Line Pending Bit */
	EXTI_ClearITPendingBit(EXTI_Line1);
	
	/* Enable the Key EXTI line Interrupt */
	NVIC_ClearPendingIRQ(EXTI1_IRQn);	

#if 0	
	RCC->APB2ENR |= 0x01; // Clock enable reuse io
	AFIO->EXTICR [0] |= 0X0020; // EXTI13 mapped to PC1
	EXTI->IMR |= 1 <<1; // turn on interrupts on line1
	EXTI->EMR |= 1 <<1; // no events on screen line1
	EXTI->FTSR |= 1 <<1; // line1 falling edge trigger on the event
#endif	
#endif

#if 0
RCC->APB2ENR|=1<<4; 
RCC->APB2ENR|=1<<0; 
GPIOC->CRL&=0XFFFF0000;
GPIOC->CRL|=0X00003883;
GPIOC->CRH&=0XFF0FFFFF;
GPIOC->CRH|=0X00300000;
GPIOC->ODR|=0X200f;
Read_ADS(&Pen_Point.X,&Pen_Point.Y);
MY_NVIC_Init(2,0,EXTI1_IRQn,2);
RCC->APB2ENR|=0x01;
AFIO->EXTICR[0]|=0X0020;
EXTI->IMR|=1<<1; 
EXTI->EMR|=1<<1;
EXTI->FTSR|=1<<1;
#endif

#ifdef ADJ_SAVE_ENABLE
	AT24CXX_Init();// initialize 24CXX
	if (Get_Adjdata()) 
		return; // calibrated
	else // not calibrated?
	{
		LCD_Clear(WHITE); // clear the screen
		Touch_Adjust(); // screen calibration
		Save_Adjdata();
	}
	Get_Adjdata();
#else
	LCD_Clear(WHITE); // clear the screen
	Touch_Adjust(); // screen calibration, with automatic save
#endif

	usart1_transmit_string_format("Pen_Point.xfac:% f \r\n", Pen_Point.xfac);
	usart1_transmit_string_format("Pen_Point.yfac:% f \r\n", Pen_Point.yfac);
	usart1_transmit_string_format("Pen_Point.xoff:% d \r\n", Pen_Point.xoff);
	usart1_transmit_string_format("Pen_Point.yoff:% d \r\n", Pen_Point.yoff);

}

