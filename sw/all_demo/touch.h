
#ifndef __TOUCH_H__
#define __TOUCH_H__



// ADS7843/7846/UH7843/7846/XPT2046/TSC2046 driver function
#define Key_Down 0x01
#define Key_Up   0x00 

typedef struct 
{
	u16 X0; // original coordinates
	u16 Y0;
	u16 X; // temporary coordinate
	u16 Y;
	u8  Key_Sta; // pen state
	
	// touch screen calibration parametersÊý
	float xfac;
	float yfac;
	short xoff;
	short yoff;
} Pen_Holder;	   

extern Pen_Holder Pen_Point;

// chip connection pin and touch screen
#define PEN  PCin(1)   //PC1  INT
#define DOUT PCin(2)   //PC2  MISO
#define TDIN PCout(3)  //PC3  MOSI
#define TCLK PCout(0)  //PC0  SCLK
#define TCS  PCout(13) //PC13 CS  

// ADS7843/7846/UH7843/7846/XPT2046/TSC2046
#define CMD_RDY 0X90  //0B10010000 read a differential manner X coordinate
#define CMD_RDX	0XD0  //0B11010000 read Y coordinates with a differential approach
#define TEMP_RD	0XF0  //0B11110000 read Y coordinates with a differential approach

// use the Save
#undef ADJ_SAVE_ENABLE	    
			  
void bsp_touch_Init(void);
u8 Read_ADS(u16 *x,u16 *y);
u8 Read_ADS2(u16 *x,u16 *y);
u16 ADS_Read_XY(u8 xy);
u16 ADS_Read_AD(u8 CMD);
void ADS_Write_Byte(u8 num);
void Drow_Touch_Point(u8 x,u16 y);
void Draw_Big_Point(u8 x,u16 y);
void Touch_Adjust(void);
void Save_Adjdata(void);
u8 Get_Adjdata(void);
void Pen_Int_Set(u8 en);
void Convert_Pos(void);


void EXTI1_IRQHandler(void);
#endif



















