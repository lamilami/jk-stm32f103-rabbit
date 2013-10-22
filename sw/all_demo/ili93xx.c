
#include  <stm32f10x.h>
#include "hw_config.h"
#include "lcd.h"
#include "stdlib.h"
#include "font.h" 
#include "usart.h"	 

// 2.4, 2.8 inch TFT liquid crystal actuation
// supports actuates the IC model to include: ILI9325/RM68021/ILI9320/LGDP4531/SPFD5408 and so on
// paint brush color, background color
u16 POINT_COLOR = 0x0000, BACK_COLOR = 0xFFFF;  
u16 DeviceCode;	 



// writes the register function
void LCD_WR_REG (u8 data)
{ 
	LCD_RS=0; // writes the address  
 	LCD_CS=0; 
	LCD_DATAOUT(data); 
	LCD_WR=0; 
	LCD_WR=1; 
 	LCD_CS=1;  
}  
// writes the register
void LCD_WriteReg (u8 LCD_Reg, u16 LCD_RegValue)
{	
	LCD_WR_REG(LCD_Reg);  
	LCD_WR_DATA(LCD_RegValue);	    		 
}	   
// reads the register
u16 LCD_ReadReg (u8 LCD_Reg)
{										   
	u16 t;
	LCD_WR_REG(LCD_Reg);   // reads in the register number which must read  
	GPIOB->CRL=0X88888888;  //PB0-7 pulls the input
	GPIOB->CRH=0X88888888; //PB8-15 pulls the input
	GPIOB->ODR=0X0000;     // outputs 0 completely
	LCD_RS=1;
	LCD_CS=0;
	// read data reads register, does not need to read 2 times)
	LCD_RD=0;					   
	LCD_RD=1;
	t=LCD_DATAIN;  
	LCD_CS=1;   
	GPIOB->CRL=0X33333333; //PB0-7 pulls the output
	GPIOB->CRH=0X33333333; //PB8-15 pulls the output
	GPIOB->ODR=0XFFFF;    // outputs high completely
	return t;  
}   
// starts to write GRAM
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(R34);
}	 

//LCD writes GRAM
void LCD_WriteRAM (u16 RGB_Code)
{							    
	LCD_WR_DATA(RGB_Code); // writes 16 GRAM
}

// from the ILI93xx read-out's data is the GBR form, but we read in is the RGB form.
// through this function transformation
//c: GBR form color value
// returns value: RGB form color value
u16 LCD_BGR2RGB (u16 c)
{
  u16 r, g, b, rgb;   
  b=(c>>0)&0x1f;
  g=(c>>5)&0x3f;
  r=(c>>11)&0x1f;	 
  rgb=(b<<11)+(g<<5)+(r<<0);		 
  return(rgb);
}		 

// reads a some spot the color value	 
//x: 0~239
//y: 0~319
// returns value: This spot color
u16 LCD_ReadPoint (u16 x, u16 y)
{
	u16 t;	
	
	if(x>=LCD_W||y>=LCD_H)return 0; // has surpassed the scope, direct returns		   
	
	LCD_SetCursor(x, y);
	LCD_WR_REG(R34);       // chooses the GRAM address 
	GPIOB->CRL=0X88888888;  //PB0-7 pulls the input
	GPIOB->CRH=0X88888888;  //PB8-15 pulls the input
	GPIOB->ODR=0XFFFF;     // outputs high completely
	LCD_RS=1;
	LCD_CS=0;
	// read data reads GRAM, needs to read 2 times)
	LCD_RD=0;					   
	LCD_RD=1;
	//dummy READ
	LCD_RD=0;					   
	LCD_RD=1;
	t=LCD_DATAIN;  
	LCD_CS=1;   
	GPIOB->CRL=0X33333333; //PB0-7 pulls the output
	GPIOB->CRH=0X33333333; //PB8-15 pulls the output
	GPIOB->ODR=0XFFFF;    // outputs high completely  
	if(DeviceCode==0X4531) 
		return t; //4531 actuates IC
	else 
		return LCD_BGR2RGB(t);
	
}

//LCD opens the demonstration
void LCD_DisplayOn(void)
{					   
	LCD_WriteReg (R7, 0x0173); //26 ten thousand color demonstration opening
}	 

//LCD closes the demonstration
void LCD_DisplayOff(void)
{	   
	LCD_WriteReg (R7, 0x0); // closure demonstration 
}    

//LCD time delay function 10MS
void Delay (u32 nCount)
{
	volatile int i;	 	
	for (i=0; i<nCount*100; i++);
}

// establishment cursor position
//Xpos: X-coordinate
//Ypos: Y-coordinate
/*__inline*/ void LCD_SetCursor (u8 Xpos, u16 Ypos)
{
	LCD_WriteReg (R32, Xpos);
	LCD_WriteReg (R33, Ypos);
} 

// picture spot
//x: 0~239
//y: 0~319
//POINT_COLOR: This spot color
void LCD_DrawPoint (u16 x, u16 y)
{
	LCD_SetCursor(x, y); // establishment cursor position 
	LCD_WR_REG(R34); // starts to read in GRAM
	LCD_WR_DATA(POINT_COLOR); 
} 	 

// initialization lcd
// should initialize the function to be possible to initialize each kind of ILI93XX liquid crystal, but other functions are based on ILI9320!!!
// has not tested on other model's actuation chip! 
void LCD_Init(void)
{ 
	GPIO_InitTypeDef GPIO_InitStructure;


 	RCC->APB2ENR|=1<<3; // enables the peripheral PORTB clock first
 	RCC->APB2ENR|=1<<4; // enables the peripheral PORTC clock first

	RCC->APB2ENR|=1<<0;    // opens the auxiliary clock
											 
	//PORTC6 ~10 multiplying push-pull output 	
	GPIOC->CRH&=0XFFFFF000;
	GPIOC->CRH|=0X00000333; 
	GPIOC->CRL&=0X00FFFFFF;
	GPIOC->CRL|=0X33000000;  
	GPIOC->ODR|=0X07C0; 	 
	
	//PORTB push-pull output 	
	GPIOB->CRH=0X33333333;
	GPIOB->CRL=0X33333333; 	 
	GPIOB->ODR=0XFFFF;


	/* Configure NRF_IRQ interrupt */
	GPIO_InitStructure.GPIO_Pin =  LCD_RST_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


#if 1
	LCD_RST = 0;
	GPIO_ResetBits(LCD_PORT, LCD_RST_PIN);
	Delay(200);
	LCD_RST = 1;
	GPIO_SetBits(LCD_PORT, LCD_RST_PIN);	
	Delay(200);
#endif

	// BackLight Off
	LCD_BL = 1;
	
	LCD_RD = 1;
	LCD_RS = 1;
	LCD_CS = 1;
	LCD_WR = 1;	
	  					 
	Delay(5); // delay 50 ms 
	LCD_WriteReg(0x0000,0x0001);
	Delay(50); // delay 50 ms 
	DeviceCode = LCD_ReadReg(0x0000);   
	usart1_transmit_string_format("LCD ID:0x%x \n", DeviceCode);   


	
	if(DeviceCode==0x9325||DeviceCode==0x9328)//ILI9325
	{
		LCD_WriteReg(0x00e7,0x0010);      
		LCD_WriteReg(0x0000,0x0001); // opening internal clock
		LCD_WriteReg(0x0001,0x0100);     
		LCD_WriteReg(0x0002,0x0700); // power source opening                    
		//LCD_WriteReg (0x0003,(1<<3)|(1<<4)); 	//65K RGB
		//DRIVE TABLE (register 03H)
		//BIT3 =AM BIT4:5=ID0:1
		//AM ID0 ID1 FUNCATION
		// 0 0 0 R->L D->U
		// 1 0 0 D->U R->L
		// 0 1 0 L->R D->U
		// 1 1 0 D->U L->R
		// 0 0 1 R->L U->D
		// 1 0 1 U->D R->L
		// 0 1 1 L->R U->D normal uses this.  	
		// 1 1 1 U->D L->R
        LCD_WriteReg(0x0003,(1<<12)|(3<<4)|(0<<3)); //65K    
        LCD_WriteReg(0x0004,0x0000);                                   
        LCD_WriteReg(0x0008,0x0207);	           
        LCD_WriteReg(0x0009,0x0000);         
        LCD_WriteReg(0x000a,0x0000); //display setting         
        LCD_WriteReg(0x000c,0x0001); //display setting          
        LCD_WriteReg(0x000d,0x0000); //0f3c          
        LCD_WriteReg(0x000f,0x0000);
		// power source disposition
        LCD_WriteReg(0x0010,0x0000);   
        LCD_WriteReg(0x0011,0x0007);
        LCD_WriteReg(0x0012,0x0000);                                                                 
        LCD_WriteReg(0x0013,0x0000);                 
        Delay(5); 
        LCD_WriteReg(0x0010,0x1590);   
        LCD_WriteReg(0x0011,0x0227);
        Delay(5); 
        LCD_WriteReg(0x0012,0x009c);                  
        Delay(5); 
        LCD_WriteReg(0x0013,0x1900);   
        LCD_WriteReg(0x0029,0x0023);
        LCD_WriteReg(0x002b,0x000e);
        Delay(5); 
        LCD_WriteReg(0x0020,0x0000);                                                            
        LCD_WriteReg(0x0021,0x013f);           
		Delay(5); 
		// gamma adjustment
        LCD_WriteReg(0x0030,0x0007); 
        LCD_WriteReg(0x0031,0x0707);   
        LCD_WriteReg(0x0032,0x0006);
        LCD_WriteReg(0x0035,0x0704);
        LCD_WriteReg(0x0036,0x1f04); 
        LCD_WriteReg(0x0037,0x0004);
        LCD_WriteReg(0x0038,0x0000);        
        LCD_WriteReg(0x0039,0x0706);     
        LCD_WriteReg(0x003c,0x0701);
        LCD_WriteReg(0x003d,0x000f);
        Delay(5); 
        LCD_WriteReg(0x0050,0x0000); // level GRAM reference 
        LCD_WriteReg(0x0051,0x00ef); // level GRAM terminated the position                    
        LCD_WriteReg(0x0052,0x0000); // vertical GRAM reference                    
        LCD_WriteReg(0x0053,0x013f); // vertical GRAM terminated the position  
        
        LCD_WriteReg(0x0060,0xa700);        
        LCD_WriteReg(0x0061,0x0001); 
        LCD_WriteReg(0x006a,0x0000);
        LCD_WriteReg(0x0080,0x0000);
        LCD_WriteReg(0x0081,0x0000);
        LCD_WriteReg(0x0082,0x0000);
        LCD_WriteReg(0x0083,0x0000);
        LCD_WriteReg(0x0084,0x0000);
        LCD_WriteReg(0x0085,0x0000);
      
        LCD_WriteReg(0x0090,0x0010);     
        LCD_WriteReg(0x0092,0x0000);  
        LCD_WriteReg(0x0093,0x0003);
        LCD_WriteReg(0x0095,0x0110);
        LCD_WriteReg(0x0097,0x0000);        
        LCD_WriteReg(0x0098,0x0000);  
        // opens the demonstration establishment    
        LCD_WriteReg(0x0007,0x0133);   
        LCD_WriteReg(0x0020,0x0000);                                                            
        LCD_WriteReg(0x0021,0x013f);
	} 
	else if(DeviceCode==0x9320 || DeviceCode==0x9300)
	{
		LCD_WriteReg(0x00,0x0001); // LCD_WriteReg(0x00,0x0000);
		Delay(50);
		
		LCD_WriteReg(0x01,0x0100);	//Driver Output Contral.
		LCD_WriteReg(0x02,0x0700);	//LCD Driver Waveform Contral.
		Delay(20); 
		LCD_WriteReg(0x03,0x1030); //Entry Mode Set.
		//LCD_WriteReg (0x03,0x1018);	//Entry Mode Set.
		Delay(20); 
	
		LCD_WriteReg(0x04,0x0000);	//Scalling Contral.
		LCD_WriteReg(0x08,0x0202);	//Display Contral 2. (0x0207)
		LCD_WriteReg(0x09,0x0000);	//Display Contral 3. (0x0000)
		LCD_WriteReg(0x0a,0x0000);	//Frame Cycle Contal. (0x0000)
		LCD_WriteReg(0x0c,(1<<0));	//Extern Display Interface Contral 1. (0x0000)
		LCD_WriteReg(0x0d,0x0000);	//Frame Maker Position.
		LCD_WriteReg(0x0f,0x0000);	//Extern Display Interface Contral 2.	    
		Delay(20); 
		LCD_WriteReg(0x07,0x0101);	//Display Contral.
		Delay(20); 								  
		LCD_WriteReg(0x10,(1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4));	//Power Control 1. (0x16b0)
		Delay(20); 
		LCD_WriteReg(0x11,0x0007);								//Power Control 2. (0x0001)
		Delay(40); 
		LCD_WriteReg(0x12,(1<<8)|(1<<4)|(0<<0));				//Power Control 3. (0x0138)
		Delay(40); 
		LCD_WriteReg(0x13,0x0b00);								//Power Control 4.
		Delay(20); 
		LCD_WriteReg(0x29,0x0000);								//Power Control 7.
		Delay(20); 
	
		LCD_WriteReg(0x2b,(1<<14)|(1<<4));	    
		LCD_WriteReg(0x50,0);	//Set X Star
		// level GRAM terminated position Set X End.
		LCD_WriteReg(0x51,239);	//Set Y Star
		LCD_WriteReg(0x52,0);	//Set Y End.t.
		LCD_WriteReg(0x53,319);	//
		Delay(20); 
	
		LCD_WriteReg(0x60,0x2700);	//Driver Output Control.
		LCD_WriteReg(0x61,0x0001);	//Driver Output Control.
		LCD_WriteReg(0x6a,0x0000);	//Vertical Srcoll Control.
		Delay(20); 
	
		LCD_WriteReg(0x80,0x0000);	//Display Position? Partial Display 1.
		LCD_WriteReg(0x81,0x0000);	//RAM Address Start? Partial Display 1.
		LCD_WriteReg(0x82,0x0000);	//RAM Address End-Partial Display 1.
		LCD_WriteReg(0x83,0x0000);	//Displsy Position? Partial Display 2.
		LCD_WriteReg(0x84,0x0000);	//RAM Address Start? Partial Display 2.
		LCD_WriteReg(0x85,0x0000);	//RAM Address End? Partial Display 2.
		Delay(20); 
	
		LCD_WriteReg(0x90,(0<<7)|(16<<0));	//Frame Cycle Contral. (0x0013)
		LCD_WriteReg(0x92,0x0000);	//Panel Interface Contral 2. (0x0000)
		LCD_WriteReg(0x93,0x0001);	//Panel Interface Contral 3.
		LCD_WriteReg(0x95,0x0110);	//Frame Cycle Contral. (0x0110)
		LCD_WriteReg(0x97,(0<<8));	//
		LCD_WriteReg(0x98,0x0000);	//Frame Cycle Contral.	   
		LCD_WriteReg(0x07,0x0173);	// (0x0173)
		Delay(40);
	} 
	else if(DeviceCode==0x5408)
	{
#if 1	
		LCD_WriteReg(0x01,0x0100);								  
		LCD_WriteReg(0x02,0x0700); //LCD Driving Waveform Contral 

		// 세로모드
		//LCD_WriteReg(0x03,0x1030); //Entry Mode establishment 	   
		// 가로모드
		LCD_WriteReg(0x03,0x1018); 
		
		// indicator top-down increases the pattern automatically from left to right
		//Normal Mode (Window Mode disable)
		//RGB form
		//16 bit data 2 transmission 8 main line establishments
		LCD_WriteReg(0x04,0x0000); //Scalling Control register     
		LCD_WriteReg(0x08,0x0207); //Display Control 2 
		LCD_WriteReg(0x09,0x0000); //Display Control 3	 
		LCD_WriteReg(0x0A,0x0000); //Frame Cycle Control	 
		LCD_WriteReg(0x0C,0x0000); //External Display Interface Control 1 
		LCD_WriteReg(0x0D,0x0000); //Frame Maker Position		 
		LCD_WriteReg(0x0F,0x0000); //External Display Interface Control 2 
 		Delay(20);
		//TFT liquid crystal color image display packing 14
		LCD_WriteReg(0x10,0x16B0); //0x14B0 //Power Control 1
		LCD_WriteReg(0x11,0x0001); //0x0007 //Power Control 2
		LCD_WriteReg(0x17,0x0001); //0x0000 //Power Control 3
		LCD_WriteReg(0x12,0x0138); //0x013B //Power Control 4
		LCD_WriteReg(0x13,0x0800); //0x0800 //Power Control 5

		// 새로 추가 함
	     //LCD_WriteReg(0x0020,0x00ef);
	     //LCD_WriteReg(0x0021,0x013f);
		// 새로 추가 함
		
		LCD_WriteReg(0x29,0x0009); //NVM read data 2
		LCD_WriteReg(0x2a,0x0009); //NVM read data 3
		LCD_WriteReg(0xa4,0x0000);	 
		LCD_WriteReg(0x50,0x0000); // establishes the operation window X axis to start the row
		LCD_WriteReg(0x51,0x00EF); // establishes the operation window X axis conclusion row
		LCD_WriteReg(0x52,0x0000); // establishes the operation window the Y axis to start the line
		LCD_WriteReg(0x53,0x013F); // establishes the operation window the Y axis conclusion line

		LCD_WriteReg(0x60,0x2700); //Driver Output Control  --> original
		//LCD_WriteReg(0x0060, 0xA700); // Gate Scan Line
		
		// establishes screen's points as well as the scanning outset line
		LCD_WriteReg(0x61,0x0001); //Driver Output Control
		LCD_WriteReg(0x6A,0x0000); //Vertical Scroll Control
		LCD_WriteReg(0x80,0x0000); //Display Position - Partial Display 1
		LCD_WriteReg(0x81,0x0000); //RAM Address Start - Partial Display 1
		LCD_WriteReg(0x82,0x0000); //RAM address End - Partial Display 1
		LCD_WriteReg(0x83,0x0000); //Display Position - Partial Display 2
		LCD_WriteReg(0x84,0x0000); //RAM Address Start - Partial Display 2
		LCD_WriteReg(0x85,0x0000); //RAM address End - Partail Display2
		LCD_WriteReg(0x90,0x0013); //Frame Cycle Control
		LCD_WriteReg(0x92,0x0000);  //Panel Interface Control 2
		LCD_WriteReg(0x93,0x0003); //Panel Interface control 3
		LCD_WriteReg(0x95,0x0110);  //Frame Cycle Control
		LCD_WriteReg(0x07,0x0173);		 
		Delay(5);
#endif

#if 0
	LCD_WriteReg(0x00E3,0x3008);        
	LCD_WriteReg(0x00E7,0x0012);
	LCD_WriteReg(0x00Ef,0x1231); 	// Set the internal timing;    
	//initializing funciton 1 
         LCD_WriteReg(0x00,0x0001);
         LCD_WriteReg(0x01, 0x0000); // set SS and SM bit
         LCD_WriteReg(0x02, 0x0700); // set 1 line inversion
        // LCD_WriteReg(0x03, 0x10B0); // set GRAM write direction and BGR=1.
#if   ID_AM==000       
	     LCD_WriteReg(0x0003,0x1000);
#elif ID_AM==001        
	     LCD_WriteReg(0x0003,0x1008);      
#elif ID_AM==010  
	     LCD_WriteReg(0x0003,0x1010);        
#elif ID_AM==011
	     LCD_WriteReg(0x0003,0x1018);
#elif ID_AM==100  
	     LCD_WriteReg(0x0003,0x1020);      
#elif ID_AM==101  
	     LCD_WriteReg(0x0003,0x1028);      
#elif ID_AM==110  
	     LCD_WriteReg(0x0003,0x1030);      
#elif ID_AM==111  
	     LCD_WriteReg(0x0003,0x1038);
#endif    
         LCD_WriteReg(0x04, 0x0000); // Resize register
         LCD_WriteReg(0x08, 0x0404); // set the back porch and front porch
         LCD_WriteReg(0x09, 0x0000); // set non-display area refresh cycle ISC[3:0]
         LCD_WriteReg(0x0A, 0x0000); // FMARK function
        // LCD_WriteReg(0x0C, 0x0000); // RGB interface setting
        // LCD_WriteReg(0x0D, 0x0000); // Frame marker Position
        // LCD_WriteReg(0x0F, 0x0000); // RGB interface polarity
//Power On sequence //
         LCD_WriteReg(0x10, 0x0080); // SAP, BT[3:0], AP, DSTB, SLP, STB
         LCD_WriteReg(0x11, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0]
         LCD_WriteReg(0x12, 0x0000); // VREG1OUT voltage
         LCD_WriteReg(0x13, 0x0000); // VDV[4:0] for VCOM amplitude
	 LCD_WriteReg(0x07, 0x0001); // 
		 
         Delay(10); // Dis-charge capacitor power voltage
         
         LCD_WriteReg(0x10, 0x1590); // SAP, BT[3:0], AP, DSTB, SLP, STB
         LCD_WriteReg(0x11, 0x0227); // DC1[2:0], DC0[2:0], VC[2:0]
         Delay(5); // Delay 50ms
        
         LCD_WriteReg(0x12, 0x001c); // Internal reference voltage= Vci;
         Delay(5); // Delay 50ms
         LCD_WriteReg(0x13, 0x1500); // Set VDV[4:0] for VCOM amplitude
         LCD_WriteReg(0x29, 0x0010); // Set VCM[5:0] for VCOMH
         LCD_WriteReg(0x2B, 0x000f); // Set Frame Rate
         Delay(5); // Delay 50ms
         
#if   ID_AM==000         
	     LCD_WriteReg(0x0020,0x00ef);
	     LCD_WriteReg(0x0021,0x013f);      
#elif ID_AM==001
	     LCD_WriteReg(0x0020,0x00ef);
	     LCD_WriteReg(0x0021,0x013f);      
#elif ID_AM==010
	     LCD_WriteReg(0x0020,0x0000);
	     LCD_WriteReg(0x0021,0x013f);      
#elif ID_AM==011
	     LCD_WriteReg(0x0020,0x0000);
	     LCD_WriteReg(0x0021,0x013f);       
#elif ID_AM==100
	     LCD_WriteReg(0x0020,0x00ef);
	     LCD_WriteReg(0x0021,0x0000);      
#elif ID_AM==101  
	     LCD_WriteReg(0x0020,0x00ef);
	     LCD_WriteReg(0x0021,0x0000);      
#elif ID_AM==110
	     LCD_WriteReg(0x0020,0x0000);
	     LCD_WriteReg(0x0021,0x0000);      
#elif ID_AM==111
	     LCD_WriteReg(0x0020,0x0000);
	     LCD_WriteReg(0x0021,0x0000);         
#endif  
// ----------- Adjust the Gamma Curve ----------//
		LCD_WriteReg(0x0030,0x0101);
		LCD_WriteReg(0x0031,0x0707);
		LCD_WriteReg(0x0032,0x0505);
		LCD_WriteReg(0x0035,0x0107);
		LCD_WriteReg(0x0036,0x0108);
		LCD_WriteReg(0x0037,0x0102);
		LCD_WriteReg(0x0038,0x0202);
		LCD_WriteReg(0x0039,0x0106);
		LCD_WriteReg(0x003C,0x0202);
		LCD_WriteReg(0x003D,0x0806);
//------------------ Set GRAM area ---------------//
         LCD_WriteReg(0x0050, 0x0000); // Horizontal GRAM Start Address
         LCD_WriteReg(0x0051, 0x00EF); // Horizontal GRAM End Address
         LCD_WriteReg(0x0052, 0x0000); // Vertical GRAM Start Address
         LCD_WriteReg(0x0053, 0x013F); // Vertical GRAM Start Address
         LCD_WriteReg(0x0060, 0xA700); // Gate Scan Line
         LCD_WriteReg(0x0061, 0x0001); // NDL,VLE, REV
         LCD_WriteReg(0x006A, 0x0000); // set scrolling line
//-------------- Partial Display Control ---------//
         LCD_WriteReg(0x80, 0x0000);
         LCD_WriteReg(0x81, 0x0000);
         LCD_WriteReg(0x82, 0x0000);
         LCD_WriteReg(0x83, 0x0000);
         LCD_WriteReg(0x84, 0x0000);
         LCD_WriteReg(0x85, 0x0000);
//-------------- Panel Control -------------------//
         LCD_WriteReg(0x90, 0x0010);
         LCD_WriteReg(0x92, 0x0000);
         LCD_WriteReg(0x93, 0x0003);
         LCD_WriteReg(0x95, 0x0110);
         LCD_WriteReg(0x97, 0x0000);
         LCD_WriteReg(0x98, 0x0000);
         LCD_WriteReg(0x07, 0x0173); // 262K color and display ON   0133

		 Delay(5);
#endif		 
	}	
	else if(DeviceCode==0x1505)
	{
		// second release on 3/5, luminance is acceptable, water wave appear during camera preview LCD_WriteReg(0x0007,0x0000);
        Delay(5); 
        LCD_WriteReg(0x0012,0x011C); //0x011A   why need to set several times?
        LCD_WriteReg(0x00A4,0x0001); //NVM	 
        LCD_WriteReg(0x0008,0x000F);
        LCD_WriteReg(0x000A,0x0008);
        LCD_WriteReg(0x000D,0x0008);	    
  		// gamma adjustment
        LCD_WriteReg(0x0030,0x0707);
        LCD_WriteReg(0x0031,0x0007); //0x0707
        LCD_WriteReg(0x0032,0x0603); 
        LCD_WriteReg(0x0033,0x0700); 
        LCD_WriteReg(0x0034,0x0202); 
        LCD_WriteReg(0x0035,0x0002); //? 0x0606
        LCD_WriteReg(0x0036,0x1F0F);
        LCD_WriteReg(0x0037,0x0707); //0x0f0f 0x0105
        LCD_WriteReg(0x0038,0x0000); 
        LCD_WriteReg(0x0039,0x0000); 
        LCD_WriteReg(0x003A,0x0707); 
        LCD_WriteReg(0x003B,0x0000); //0x0303
        LCD_WriteReg(0x003C,0x0007); //0x0707
        LCD_WriteReg(0x003D,0x0000); //0x1313//0x1f08
        Delay(5); 
        LCD_WriteReg(0x0007,0x0001);
        LCD_WriteReg(0x0017,0x0001); // opens the power source
        Delay(5); 
  		// power source disposition
        LCD_WriteReg(0x0010,0x17A0); 
        LCD_WriteReg(0x0011,0x0217); //reference voltage VC[2:0] Vciout = 1.00*Vcivl
        LCD_WriteReg(0x0012,0x011E); //0x011c //Vreg1out = Vcilvl*1.80 is it the same as Vgama1out?
        LCD_WriteReg(0x0013,0x0F00); //VDV[4:0]-->VCOM Amplitude VcomL = VcomH - Vcom Ampl
        LCD_WriteReg(0x002A,0x0000);  
        LCD_WriteReg(0x0029,0x000A); //0x0001F Vcomh = VCM1[4:0]*Vreg1out gate source voltage??
        LCD_WriteReg(0x0012,0x013E); // 0x013C  power supply on
        //Coordinates Control//
        LCD_WriteReg(0x0050,0x0000); //0x0e00
        LCD_WriteReg(0x0051,0x00EF); 
        LCD_WriteReg(0x0052,0x0000); 
        LCD_WriteReg(0x0053,0x013F); 
    	//Pannel Image Control//
        LCD_WriteReg(0x0060,0x2700); 
        LCD_WriteReg(0x0061,0x0001); 
        LCD_WriteReg(0x006A,0x0000); 
        LCD_WriteReg(0x0080,0x0000); 
    	//Partial Image Control//
        LCD_WriteReg(0x0081,0x0000); 
        LCD_WriteReg(0x0082,0x0000); 
        LCD_WriteReg(0x0083,0x0000); 
        LCD_WriteReg(0x0084,0x0000); 
        LCD_WriteReg(0x0085,0x0000); 
  		//Panel Interface Control//
        LCD_WriteReg(0x0090,0x0013); //0x0010 frenqucy
        LCD_WriteReg(0x0092,0x0300); 
        LCD_WriteReg(0x0093,0x0005); 
        LCD_WriteReg(0x0095,0x0000); 
        LCD_WriteReg(0x0097,0x0000); 
        LCD_WriteReg(0x0098,0x0000); 
  
        LCD_WriteReg(0x0001,0x0100); 
        LCD_WriteReg(0x0002,0x0700); 
        LCD_WriteReg(0x0003,0x1030); 
        LCD_WriteReg(0x0004,0x0000); 
        LCD_WriteReg(0x000C,0x0000); 
        LCD_WriteReg(0x000F,0x0000); 
        LCD_WriteReg(0x0020,0x0000); 
        LCD_WriteReg(0x0021,0x0000); 
        LCD_WriteReg(0x0007,0x0021); 
        Delay(20);
        LCD_WriteReg(0x0007,0x0061); 
        Delay(20);
        LCD_WriteReg(0x0007,0x0173); 
        Delay(20);
	}							 
	else if(DeviceCode==0x8989)
	{
		LCD_WriteReg(0x0000,0x0001); Delay(5); // opens the crystal oscillator
    	LCD_WriteReg(0x0003,0xA8A4); Delay(5); //0xA8A4
    	LCD_WriteReg(0x000C,0x0000); Delay(5);    
    	LCD_WriteReg(0x000D,0x080C); Delay(5);    
    	LCD_WriteReg(0x000E,0x2B00); Delay(5);    
    	LCD_WriteReg(0x001E,0x00B0); Delay(5);    
    	LCD_WriteReg(0x0001,0x2B3F); Delay(5); // actuation output control 320*240 0x6B3F
    	LCD_WriteReg(0x0002,0x0600); Delay(5); 
    	LCD_WriteReg(0x0010,0x0000); Delay(5); 
    	LCD_WriteReg(0x0011,0x6070); Delay(5); // definition data format 16 color horizontal screen 0x6058
    	LCD_WriteReg(0x0005,0x0000); Delay(5); 
    	LCD_WriteReg(0x0006,0x0000); Delay(5); 
    	LCD_WriteReg(0x0016,0xEF1C); Delay(5); 
    	LCD_WriteReg(0x0017,0x0003); Delay(5); 
    	LCD_WriteReg(0x0007,0x0233); Delay(5); //0x0233       
    	LCD_WriteReg(0x000B,0x0000); Delay(5); 
    	LCD_WriteReg(0x000F,0x0000); Delay(5); // scanning starts the address
    	LCD_WriteReg(0x0041,0x0000); Delay(5); 
    	LCD_WriteReg(0x0042,0x0000); Delay(5); 
    	LCD_WriteReg(0x0048,0x0000); Delay(5); 
    	LCD_WriteReg(0x0049,0x013F); Delay(5); 
    	LCD_WriteReg(0x004A,0x0000); Delay(5); 
    	LCD_WriteReg(0x004B,0x0000); Delay(5); 
    	LCD_WriteReg(0x0044,0xEF00); Delay(5); 
    	LCD_WriteReg(0x0045,0x0000); Delay(5); 
    	LCD_WriteReg(0x0046,0x013F); Delay(5); 
    	LCD_WriteReg(0x0030,0x0707); Delay(5); 
    	LCD_WriteReg(0x0031,0x0204); Delay(5); 
    	LCD_WriteReg(0x0032,0x0204); Delay(5); 
    	LCD_WriteReg(0x0033,0x0502); Delay(5); 
    	LCD_WriteReg(0x0034,0x0507); Delay(5); 
    	LCD_WriteReg(0x0035,0x0204); Delay(5); 
    	LCD_WriteReg(0x0036,0x0204); Delay(5); 
    	LCD_WriteReg(0x0037,0x0502); Delay(5); 
    	LCD_WriteReg(0x003A,0x0302); Delay(5); 
    	LCD_WriteReg(0x003B,0x0302); Delay(5); 
    	LCD_WriteReg(0x0023,0x0000); Delay(5); 
    	LCD_WriteReg(0x0024,0x0000); Delay(5); 
    	LCD_WriteReg(0x0025,0x8000); Delay(5); 
    	LCD_WriteReg(0x004f,0);        // line of first site 0
    	LCD_WriteReg(0x004e,0);        // row first site 0
	} 
	else if(DeviceCode==0x4531)
	{
		LCD_WriteReg(0X00,0X0001);   
		Delay(50);   
		LCD_WriteReg(0X10,0X1628);   
		LCD_WriteReg(0X12,0X000e); //0x0006    
		LCD_WriteReg(0X13,0X0A39);   
		Delay(10);   
		LCD_WriteReg(0X11,0X0040);   
		LCD_WriteReg(0X15,0X0050);   
		Delay(40);   
		LCD_WriteReg(0X12,0X001e); //16    
		Delay(40);   
		LCD_WriteReg(0X10,0X1620);   
		LCD_WriteReg(0X13,0X2A39);   
		Delay(10);   
		LCD_WriteReg(0X01,0X0100);   
		LCD_WriteReg(0X02,0X0300);   
		LCD_WriteReg(0X03,0X1030); // change direction   
		LCD_WriteReg(0X08,0X0202);   
		LCD_WriteReg(0X0A,0X0008);   
		LCD_WriteReg(0X30,0X0000);   
		LCD_WriteReg(0X31,0X0402);   
		LCD_WriteReg(0X32,0X0106);   
		LCD_WriteReg(0X33,0X0503);   
		LCD_WriteReg(0X34,0X0104);   
		LCD_WriteReg(0X35,0X0301);   
		LCD_WriteReg(0X36,0X0707);   
		LCD_WriteReg(0X37,0X0305);   
		LCD_WriteReg(0X38,0X0208);   
		LCD_WriteReg(0X39,0X0F0B);   
		LCD_WriteReg(0X41,0X0002);   
		LCD_WriteReg(0X60,0X2700);   
		LCD_WriteReg(0X61,0X0001);   
		LCD_WriteReg(0X90,0X0210);   
		LCD_WriteReg(0X92,0X010A);   
		LCD_WriteReg(0X93,0X0004);   
		LCD_WriteReg(0XA0,0X0100);   
		LCD_WriteReg(0X07,0X0001);   
		LCD_WriteReg(0X07,0X0021);   
		LCD_WriteReg(0X07,0X0023);   
		LCD_WriteReg(0X07,0X0033);   
		LCD_WriteReg(0X07,0X0133);   
		LCD_WriteReg(0XA0,0X0000); 
	}					  
	Delay(5000);
	LCD_BL=0; // lightens the back light	 
	LCD_Clear(WHITE);
}  		  
  
// clear screen function
//Color: Wants the clear screen the packing color
void LCD_Clear (u16 Color)
{
	u32 index=0;      
	LCD_SetCursor(0x00,0x0000); // establishment cursor position 
	LCD_WriteRAM_Prepare();     // starts to read in GRAM	 	  
	for (index=0; index<76800; index++)
	{
		LCD_WR_DATA(Color);    
	}
}  

// in assigns in the region to fill assigns the color
// region size:
// (xend-xsta)*(yend-ysta)
void LCD_Fill (u8 xsta, u16 ysta, u8 xend, u16 yend, u16 color)
{                    
    u32 n;
	// establishment window										
	LCD_WriteReg (R80, xsta); // horizontal direction GRAM start address
	LCD_WriteReg (R81, xend); // horizontal direction GRAM end address
	LCD_WriteReg (R82, ysta); // vertical direction GRAM start address
	LCD_WriteReg (R83, yend); // vertical direction GRAM end address	
	LCD_SetCursor(xsta, ysta); // establishment cursor position  
	LCD_WriteRAM_Prepare();  // starts to read in GRAM	 	   	   
	n=(u32)(yend-ysta+1)*(xend-xsta+1);    
	while (n--) {LCD_WR_DATA(color);}// demonstration fills color. 
	// resumes the establishment
	LCD_WriteReg (R80, 0x0000); // horizontal direction GRAM start address
	LCD_WriteReg (R81, 0x00EF); // horizontal direction GRAM end address
	LCD_WriteReg (R82, 0x0000); // vertical direction GRAM start address
	LCD_WriteReg (R83, 0x013F); // vertical direction GRAM end address	    
}  

// draws a line
//x1, y1: Beginning coordinates
//x2, y2: End point coordinates  
void LCD_DrawLine (u16 x1, u16 y1, u16 x2, u16 y2)
{
	u16 t; 
	int xerr=0, yerr=0, delta_x, delta_y, distance; 
	int incx, incy, uRow, uCol; 

	delta_x=x2-x1; // computation increase of coordinates 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; // establishes on foot the direction 
	else if(delta_x==0)incx=0; // perpendicular line 
	else {incx=-1; delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0; // level line 
	else {incy=-1; delta_y=-delta_y;} 
	if (delta_x>delta_y) distance=delta_x; // selects the basic increase coordinate axis 
	else distance=delta_y; 
	for (t=0; t<=distance+1; t++) // linedraw output 
	{  
		LCD_DrawPoint(uRow, uCol); // picture spot 
		xerr+=delta_x; 
		yerr+=delta_y; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}    
// picture rectangle
void LCD_DrawRectangle (u8 x1, u16 y1, u8 x2, u16 y2)
{
	LCD_DrawLine(x1, y1, x2, y1);
	LCD_DrawLine(x1, y1, x1, y2);
	LCD_DrawLine(x1, y2, x2, y2);
	LCD_DrawLine(x2, y1, x2, y2);
}
// is assigning the position to draw one to assign the size the circle
// (x, y): Central point
//r: Radius
void Draw_Circle (u8 x0, u16 y0, u8 r)
{
	int a, b;
	int di;
	a=0; b=r;	  
	di=3-(r<<1);             // judgment spot position symbol
	while(a<=b)
	{
		LCD_DrawPoint(x0-b, y0-a);             //3           
		LCD_DrawPoint(x0+b, y0-a);             //0           
		LCD_DrawPoint(x0-a, y0+b);             //1       
		LCD_DrawPoint(x0-b, y0-a);             //7           
		LCD_DrawPoint(x0-a, y0-b);             //2             
		LCD_DrawPoint(x0+b, y0+a);             //4               
		LCD_DrawPoint(x0+a, y0-b);             //5
		LCD_DrawPoint(x0+a, y0+b);             //6 
		LCD_DrawPoint(x0-b, y0+a);             
		a++;
		// uses the Bresenham algorithm to make a circle     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 
		LCD_DrawPoint(x0+a, y0+b);
	}
} 
// is assigning the position to demonstrate a character
//x: 0~234
//y: 0~308
//num: Must demonstrate character: ""---> " ~"
//size: Font size 12/16
//mode: The superimposition way (1) is also the non-superimposition way (0)
void LCD_ShowChar (u8 x, u16 y, u8 num, u8 size, u8 mode)
{       
#define MAX_CHAR_POSX 232
#define MAX_CHAR_POSY 304 
    u8 temp;
    u8 pos, t;      
    if(x>MAX_CHAR_POSX||y>MAX_CHAR_POSY)return;	    
	// establishment window										
	LCD_WriteReg(R80, x);           // horizontal direction GRAM start address
	LCD_WriteReg(R81, x+(size/2-1)); // horizontal direction GRAM end address
	LCD_WriteReg(R82, y);           // vertical direction GRAM start address
	LCD_WriteReg(R83, y+size-1);    // vertical direction GRAM end address	
	LCD_SetCursor(x, y);            // establishment cursor position  
	LCD_WriteRAM_Prepare();        // starts to read in GRAM	   
	num = num - ' '; // obtains the displacement value
	if (!mode) // non-superimposition way
	{
		for (pos=0; pos<size; pos++)
		{
			if(size==12)temp=asc2_1206[num][pos]; // transfers 1206 typefaces
			else temp=asc2_1608[num][pos];		 // transfers 1608 typefaces
			for (t=0; t<size/2; t++)
		    {                 
		        if(temp&0x01)
				{
					LCD_WR_DATA(POINT_COLOR);
				} else LCD_WR_DATA(BACK_COLOR);	        
		        temp>>=1; 
		    }
		}	
	} else//superimposition way
	{
		for (pos=0; pos<size; pos++)
		{
			if(size==12)temp=asc2_1206[num][pos]; // transfers 1206 typefaces
			else temp=asc2_1608[num][pos];		 // transfers 1608 typefaces
			for (t=0; t<size/2; t++)
		    {                 
		        if(temp&0x01)LCD_DrawPoint(x+t, y+pos); // draws a spot     
		        temp>>=1; 
		    }
		}
	}	    
	// restores the window size	 
	LCD_WriteReg (R80, 0x0000); // horizontal direction GRAM start address
	LCD_WriteReg (R81, 0x00EF); // horizontal direction GRAM end address
	LCD_WriteReg (R82, 0x0000); // vertical direction GRAM start address
	LCD_WriteReg (R83, 0x013F); // vertical direction GRAM end address
}  
//m ^n function
u32 mypow (u8 m, u8 n)
{
	u32 result=1;	 
	while (n--) result*=m;    
	return result;
}			 
// demonstrates 2 numerals
//x, y: Beginning coordinates	 
//len: Digital figure
//size: Font size
//color: Color
//num: Value (0~4294967295);	 
void LCD_ShowNum (u8 x, u16 y, u32 num, u8 len, u8 size)
{         	
	u8 t, temp;
	u8 enshow=0;						   
	for (t=0; t<len; t++)
	{
		temp=(num/mypow(10, len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(size/2) *t, y, ' ', size,0);
				continue;
			} else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2) *t, y, temp+'0', size,0); 
	}
} 
// demonstrates 2 numerals
//x, y: Beginning coordinates
//size: Font size
//mode: Pattern 0, packing pattern; 1, superimposition pattern
//num: Value (0~99);	 
void LCD_Show2Num (u8 x, u16 y, u16 num, u8 len, u8 size, u8 mode)
{         	
	u8 t, temp;						   
	for (t=0; t<len; t++)
	{
		temp=(num/mypow(10, len-t-1))%10;
	 	LCD_ShowChar(x+(size/2) *t, y, temp+'0', size, mode); 
	}
} 
// demonstration string of character
//x, y: Beginning coordinates  
// *p: String of character start address
// uses 16 typefaces
void LCD_ShowString (u8 x, u16 y, const u8 *p)
{         
    while (*p != '\0')
    {       
        if(x>MAX_CHAR_POSX) {x=0; y+=16;}
        if(y>MAX_CHAR_POSY) {y=x=0; LCD_Clear(WHITE);}
        LCD_ShowChar(x, y,*p,16,0);
        x+=8;
        p++;
    }  
}


