/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : main.c
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 04/18/2011
* Description        : 
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/*
Predefine 설정 
64P
STM32F103RBT6(128KB, 32KB) :  USE_STDPERIPH_DRIVER, STM32F10X_MD
STM32F103R8T6(128KB, 20KB) :  USE_STDPERIPH_DRIVER, STM32F10X_MD
STM32F105RBT6(256KB, 64KB) :  USE_STDPERIPH_DRIVER, STM32F10X_CL
STM32F107RBT6(256KB, 64KB) :  USE_STDPERIPH_DRIVER, STM32F10X_CL

100p
STM32F103VCT6(512KB, 64KB) :  USE_STDPERIPH_DRIVER, STM32F10X_HD
STM32F105VCT6(512KB, 64KB) :  USE_STDPERIPH_DRIVER, STM32F10X_CL
STM32F107VBT6(256KB, 64KB) :  USE_STDPERIPH_DRIVER, STM32F10X_CL
STM32F107VCT6(256KB, 64KB) :  USE_STDPERIPH_DRIVER, STM32F10X_CL
STM32F107VBT6(128KB, 32KB) :  USE_STDPERIPH_DRIVER, STM32F10X_CL

144p
STM32F103ZET6(512KB, 64KB) :  USE_STDPERIPH_DRIVER, STM32F10X_HD
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include "main.h"
#include "led.h"
#include "timer.h"
#include "usart.h"
#include "rtc.h"
#include "mmc_sd.h"
#include "lcd.h"
#include "touch.h"
#include "key.h"
#include "nrf24l01.h"
#include "ov7670.h"
#include "diskio.h"

// fat
//#include "integer.h"
//#include "dataflash.h"
#include "ff.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


#define KEY_NONE	0x00
#define KEY_KEY1		0x01
#ifdef STM32_MIDDLE_HW_VER1	
#define KEY_KEY2		0x02
#endif


static volatile u8 s_current_service = serviceMAX;

static volatile s16 s_blink_cnt = 0;
static volatile s16 s_wait_cnt = 0;

#ifdef NRF24L01_BPS_CHECK
static volatile s16 s_rf_bps_check_cnt = 0;
#endif

vu32 time_display = 0;
static volatile s16 s_lcd_test_cnt = 0;

static volatile u16 s_key = 0;

extern u8 capture_button_status;
extern u8 capture_status;
unsigned int x_vert, y_horizen;
unsigned int val_rgb;

#ifdef OV7670_SW_V2
extern u8 vsync_count;
extern u8 capture_status;
#endif

FATFS fs;            // Work area (file system object) for logical drive  
FIL fsrca;      // file objects    
FRESULT res;         // FatFs function common result code      
UINT bw;         // File R/W count   
unsigned char buffera[2];
XCHAR file_name[]="IMG0000.bmp";
volatile unsigned char RxBuffer1[16];
const unsigned char bmp_tou[54]=
{
  0x42,0x4d,0x36,0x58, 0x02,0x00,0x00,0x00, 0x00,0x00,0x36,0x00, 0x00,0x00,0x28,0x00,//16
  0x00,0x00,0x40,0x01, 0x00,0x00,0xf0,0x00, 0x00,0x00,0x01,0x00, 0x10,0x00,0x00,0x00,//32
  0x00,0x00,0x00,0x58, 0x02,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,//48
  0x00,0x00,0x00,0x00, 0x00,0x00,													 //54
} ;
	 
struct FileList
{
	u8 filename[65];
	struct FileList *next;
	struct FileList *back;
};
 
typedef struct FileList dnode;
typedef dnode *dlink;

// setting menu tree.
static service_define_type s_menu_level = MainService;

service_type gbl_service[serviceMAX][serviceMAX] = 
{
	{
		{Ledservice, "[1] LED test\r\n", service_led, '1'},
		{TimerService, "[2] TIMER test\r\n", service_timer, '2'},	
		{RtcService, "[3] RTC test\r\n", service_rtc, '3'},
		{SdService, "[4] SD test\r\n", service_sd, '4'},
		{UsbService, "[5] USB test\r\n", service_usb, '5'},
		{LcdService, "[6] LCD test\r\n", service_lcd, '6'},
		{TouchService, "[7] Touch test\r\n", service_touch, '7'},
		{KeyService, "[8] KEY test\r\n", service_key, '8'},
		{RfService, "[9] nRF24L01 test\r\n", service_rf, '9'},
		{CameraService, "[a] OV7670 test\r\n", service_camera, 'a'},
		{serviceMAX, 0x00, 0x00, 0x00}	
	},
	{
		{ledUserCoreOnService, "[1] User Core on\r\n", service_user_led_core_on, '1'},
		{ledUserCoreOffService, "[2] User Core off\r\n", service_user_led_core_off, '2'},
		{ledUserBottomOnService, "[3] User Botton on\r\n", service_user_led_bottom_on, '3'},
		{ledUserBottomOffService, "[4] User Bottom off\r\n", service_user_led_bottom_off, '4'},
		{ledExitService, "[x] Exit led test\r\n", service_led_exit, 'x'},
		{ledServiceMAX, 0x00, 0x00, 0x00}	
	},	
	{
		{timerTIM2Service, "[1] TIM2 ticktime test\r\n", service_tim2_ticktime, '1'},
		{timerTIM2Stop, "[2] TIM2 blink stop\r\n", service_tim2_stop, '2'},
		{timerExitService, "[x] Exit timer test\r\n", service_timer_exit, 'x'},			
		{timerServiceMAX, 0x00, 0x00, 0x00}	
	},	
	{
		{rtcStartService, "[1] RTC service start\r\n", service_rtc_start, '1'},
		{rtcStopService, "[2] RTC service stop\r\n", service_rtc_stop, '2'},
		{rtcExitService, "[x] Exit RTC test\r\n", service_rtc_exit, 'x'},					
		{rtcServiceMAX, 0x00, 0x00, 0x00}	
	},		
	{
		{sdInitializeService, "[1] SD Initialize\r\n", service_sd_initialize, '1'},
		{sdGetCapacityService, "[2] SD GetCapacity\r\n", service_sd_get_capacity, '2'},
		{sdFileListService, "[3] SD SD File List\r\n", service_sd_file_list, '3'},
		{sdExitService, "[x] Exit SD test\r\n", service_sd_exit, 'x'},					
		{sdServiceMAX, 0x00, 0x00, 0x00}	
	},	
	{
		{usbStartService, "[1] USB service start\r\n", service_usb_start, '1'},
		{usbStopService, "[2] USB service stop\r\n", service_usb_stop, '2'},
		{usbExitService, "[x] Exit USB test\r\n", service_usb_exit, 'x'},					
		{usbServiceMAX, 0x00, 0x00, 0x00}	
	},	
	{
		{lcdStartService, "[1] LCD service start\r\n", service_lcd_start, '1'},
		{lcdStopService, "[2] LCD service stop\r\n", service_lcd_stop, '2'},
		{lcdExitService, "[x] Exit LCD test\r\n", service_lcd_exit, 'x'},					
		{lcdServiceMAX, 0x00, 0x00, 0x00}	
	},	
	{
		{touchStartService, "[1] TOUCH service start\r\n", service_touch_start, '1'},
		{touchStopService, "[2] TOUCH service stop\r\n", service_touch_stop, '2'},
		{touchExitService, "[x] Exit TOUCH test\r\n", service_touch_exit, 'x'},					
		{touchServiceMAX, 0x00, 0x00, 0x00}	
	},		
	{
		{keyStartPollingService, "[1] KEY service start(polling)\r\n", service_key_polling_start, '1'},
		{keyStopPollingService, "[2] KEY service stop(polling)\r\n", service_key_polling_stop, '2'},
		{keyStartInterruptService, "[3] KEY service start(interrupt)\r\n", service_key_interrupt_start, '3'},
		{keyStopInterruptService, "[4] KEY service stop(interrupt)\r\n", service_key_interrupt_stop, '4'},
		{keyExitService, "[x] Exit KEY test\r\n", service_key_exit, 'x'},					
		{keyServiceMAX, 0x00, 0x00, 0x00}	
	},	
	{
		{rfStartRxService, "[1] nRF24L01 RX service start\r\n", service_rf_start_rx, '1'},
		{rfStartTxService, "[2] nRF24L01 TX service start\r\n", service_rf_start_tx, '2'},
#ifdef NRF24L01_BPS_CHECK		
		{rfStartRxBpsService, "[3] nRF24L01 RX(BPS check) service start\r\n", service_rf_bps_start_rx, '3'},
		{rfStartTxBpsService, "[4] nRF24L01 TX(BPS check) service start\r\n", service_rf_bps_start_tx, '4'},		
		{rfStopService, "[5] nRF24L01 service stop\r\n", service_rf_stop, '5'},
#else
		{rfStopService, "[3] nRF24L01 service stop\r\n", service_rf_stop, '3'},
#endif
		{rfExitService, "[x] Exit nRF24L01 test\r\n", service_rf_exit, 'x'},					
		{rfServiceMAX, 0x00, 0x00, 0x00}	
	},	
	{
		{cameraStartOv7670Service, "[1] OV7670 preview start\r\n", service_ov7670_start, '1'},
		{cameraStopOv7670Service, "[2] OV7670 preview stop\r\n", service_ov7670_stop, '2'},
		{cameraExitOv7670Service, "[x] Exit nRF24L01 test\r\n", service_ov7670_exit, 'x'},					
		{cameraServiceMAX, 0x00, 0x00, 0x00}	
	},	
};




void wait_10ms(s16 ms_10)
{
	s_wait_cnt = 0;
	while( s_wait_cnt < ms_10 );
}

void timer2_event(void)
{
	// 

	if( s_wait_cnt++ == 0xffff )
		s_wait_cnt = 0;

#ifdef NRF24L01_BPS_CHECK
	if( s_menu_level == RfService && s_current_service == rfStartTxBpsService)	
	{
		s_rf_bps_check_cnt++;
		if( s_rf_bps_check_cnt > 1000 )
		{
			s_rf_bps_check_cnt = 1001;
		}
	}
#endif

	if( s_menu_level == KeyService && s_current_service == keyStartPollingService)
	{
		gpio_polling_key();
	}		

	// usart1_transmit_string_format("s_blink_cnt=%d\r\n", s_blink_cnt);	

	if( s_blink_cnt++ > 100 )
	{
		s_blink_cnt = 0;		

		if( s_menu_level == TimerService && s_current_service == timerTIM2Service )
		{
			bsp_led_core_toggle(ledUserCore);
		}

		if( s_menu_level == LcdService && s_current_service == lcdStartService )
		{
			lcd_test();
		}
	}
}

void rtc_event(void)
{
	if(RTC_GetITStatus(RTC_IT_SEC) != RESET)
	{
		/* Clear the RTC Second interrupt */
		RTC_ClearITPendingBit(RTC_IT_SEC);

		/* Toggle led connected to PC.06 pin each 1s */
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, (BitAction)(1-GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_6)));

		/* Enable time update */
		if( s_menu_level == RtcService && s_current_service == rtcStartService )
		{
			time_show();
		}

		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();
		/* Reset RTC Counter when Time is 23:59:59 */
		if(RTC_GetCounter() == 0x00015180)
		{
			RTC_SetCounter(0x0);
			/* Wait until last write operation on RTC registers has finished */
			RTC_WaitForLastTask();
		}
	}
}

void welcome(void)
{
	usart1_transmit_string("\r\n*****************************************************************************\r\n");
	usart1_transmit_string("Rabbit Development Board Ver 2.1\r\n");

#ifdef STM32F10X_CL
	// STM32F105, 107 에서는 PLL설정이 약간 틀리다. 
	// 이후에 PLL설정 스트링을 수정해야 한다.
	usart1_transmit_string("HSE(High Speed External clock signal) Enable\r\n");
	usart1_transmit_string("HCLK(SYSCLK) = 8MHz, PCLK2 = HCLK, PCLK1 = HCLK/2, ADCCLK = PCLK2/2\r\n");
	usart1_transmit_string("PLLCLK = 8MHz / 1 * 9 = 72MHz, USBCLK = PLLCLK / 2 = 48MHz\r\n\r\n");
#else
	usart1_transmit_string("HSE(High Speed External clock signal) Enable\r\n");
	usart1_transmit_string("HCLK(SYSCLK) = 8MHz, PCLK2 = HCLK, PCLK1 = HCLK/2, ADCCLK = PCLK2/2\r\n");
	usart1_transmit_string("PLLCLK = 8MHz / 1 * 9 = 72MHz, USBCLK = PLLCLK / 2 = 48MHz\r\n");
#endif

}

void display_menu()
{
	int i;

	for(i=0;i<serviceMAX;i++)
	{
		if( gbl_service[s_menu_level][i].run == 0x00 )
			break;

		// wait(5);
		usart1_transmit_string(gbl_service[s_menu_level][i].service_string);
		//strcpy(gbl_zz, gbl_service[s_menu_level][i].service_string);
		//strcpy(zz, gbl_service[s_menu_level][i].service_string);
		
	}
	
	usart1_transmit_string("\r\n\r\nSelect menu ? ");
	
}


int run_menu_selection(void)
{
	int i;
	char* pdata;
	s8 data;

	pdata = (char*)usart1_get_data();

	if( pdata == 0x00 )
		return 0x00;

	data = pdata[0];

#if 0
	usart1_transmit_string_format("\r\n%c", data);
#endif

	if( s_menu_level == UsbService && s_current_service == usbStartService)
	{
		usb_mouse_control(data);
	}


	for(i=0;i<serviceMAX;i++)
	{
		if( gbl_service[s_menu_level][i].run == 0x00 )
			break;

		if( gbl_service[s_menu_level][i].cmd == data )
		{
			gbl_service[s_menu_level][i].run();
			return 1;
		}
	}

	return 0;
	
}

/*******************************************************************************
* Function Name  : main.
* Description    : main routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int main(void)
{

	/* Initialize the Demo */
	bsp_init_rcc();

	bsp_init_gpio();

#ifdef USB_DISCONNECT_CONFIG
	USB_Cable_Config(DISABLE);
#endif

	bsp_init_interrupt();

	register_timer_function(timer2ServiceFunction, timer2_event);
	register_rtc_function(rtcServiceFunction, rtc_event);

	bsp_init_timer2();

	bsp_init_irq_usart1();
	//bsp_init_dma_usart1();

	// 10msec
	// wait_10ms(1);

	// User LED ON
	bsp_led_core_off(ledUserCore);
	bsp_led_core_on(ledUserCore);

	welcome();
	

#if 1
	// 64Pin MCU의 경우 포트가 부족하여 JTAG을 잠시 중단하고 SWD 모드로 전환한다.
	JTAG_Set(SWD_ENABLE);  // opens SWD
	LCD_Init();
	s_lcd_test_cnt = 7;
	lcd_test();
	s_lcd_test_cnt = 0;
	//
#endif	
	
	usart1_transmit_string("\r\n*****************************************************************************\r\n");	
	usart1_transmit_string("User led2 ( on )\r\n");
	usart1_transmit_string("Initialize gpio service.\r\n");
	usart1_transmit_string("Start USART1 service on mode interrupt.\r\n");
	usart1_transmit_string_format("BaudRate = %d, Databit = %dbit, StopBits = %d, Parity = no, FlowControl = none\r\n", 115200, 8, 1);

#ifndef USB_DISCONNECT_CONFIG
	USB_Init();
	usart1_transmit_string("USB 2.0 FS(Full speed) Initialize.\r\n");
#endif
	
	usart1_transmit_string("\r\n*****************************************************************************\r\n");		

	display_menu();

	usart1_tx_proc();
	
	while( 1 )
	{

		if( run_menu_selection() != 0 )
			display_menu();
	}

}



void service_led(void)
{
	s_menu_level = Ledservice;
	usart1_transmit_string("\r\nservice_led()\r\n");
}

void service_timer(void)
{
	s_menu_level = TimerService;
	usart1_transmit_string("\r\nservice_timer()\r\n");	
}

void service_rtc(void)
{
	s_menu_level = RtcService;
	usart1_transmit_string("\r\nservice_rtc()\r\n");	
}

void service_sd(void)
{
	s_menu_level = SdService;
	usart1_transmit_string("\r\nservice_sd()\r\n");	
}

void service_usb(void)
{
	s_menu_level = UsbService;
	usart1_transmit_string("\r\nservice_usb()\r\n");	
}

void service_lcd(void)
{
	s_menu_level = LcdService;
	usart1_transmit_string("\r\nservice_lcd()\r\n");	
}

void service_touch(void)
{
	s_menu_level = TouchService;
	usart1_transmit_string("\r\nservice_touch()\r\n");	
}

void service_key(void)
{
	s_menu_level = KeyService;
	usart1_transmit_string("\r\nservice_key()\r\n");	
}

void service_rf(void)
{
	s_menu_level = RfService;
	usart1_transmit_string("\r\nservice_rf()\r\n");	
}

void service_camera(void)
{
	s_menu_level = CameraService;
	usart1_transmit_string("\r\nservice_camera()\r\n");	
}

void service_user_led_core_on(void)
{
	bsp_led_core_on(ledUserCore);
	usart1_transmit_string("\r\nservice_user_led_core_on()\r\n");
}

void service_user_led_core_off(void)
{
	bsp_led_core_off(ledUserCore);
	usart1_transmit_string("\r\nservice_user_led_core_off()\r\n");
}

void service_user_led_bottom_on(void)
{
	bsp_led_bottom_on(ledUserBottom);
	usart1_transmit_string("\r\nservice_user_led_bottom_on()\r\n");
}

void service_user_led_bottom_off(void)
{
	bsp_led_bottom_off(ledUserBottom);
	usart1_transmit_string("\r\nservice_user_led_bottom_off()\r\n");
}

void service_led_exit(void)
{
	s_menu_level = MainService;
	s_current_service = ledExitService;	
	usart1_transmit_string("\r\nservice_led_exit()\r\n");
}


void service_tim2_ticktime(void)
{
	usart1_transmit_string("\r\nservice_tim2_ticktime()\r\n");	
	usart1_transmit_string("Start timer2 10msec period.\r\n");
	usart1_transmit_string("User led blink start 1 second period\r\n");

	s_current_service = timerTIM2Service;
}

void service_tim2_stop(void)
{
	usart1_transmit_string("\r\nservice_tim2_stop()\r\n");	
	usart1_transmit_string("User led blink stop.\r\n");

	s_current_service = timerTIM2Stop;
}


void service_timer_exit(void)
{
	s_menu_level = MainService;
	s_current_service = timerExitService;	
	usart1_transmit_string("\r\nsservice_timer_exit()\r\n");
}

void service_rtc_start(void)
{

	usart1_transmit_string("\r\nservice_rtc_start()\r\n");	

	bsp_init_rtc();
	
	usart1_transmit_string("Start RTC Service to uart.\r\n");

	time_show();

	s_current_service = rtcStartService;
}

void service_rtc_stop(void)
{
	usart1_transmit_string("\r\nservice_rtc_stop()\r\n");	
	usart1_transmit_string("Stop RTC Service to uart.\r\n");

	s_current_service = rtcStopService;
}

void service_rtc_exit(void)
{
	s_menu_level = MainService;
	s_current_service = rtcExitService;	
	usart1_transmit_string("\r\nservice_rtc_exit()\r\n");
}

void service_sd_initialize(void)
{
	usart1_transmit_string("\r\nservice_sd_initialize()\r\n");	

	bsp_sd_gpio_init();

	if( SD_Init() == 0 )
		usart1_transmit_string("\r\nSD initialize successed!\r\n");
	else
		usart1_transmit_string("\r\nSD initialize failed!\r\n");
}

void service_sd_get_capacity(void)
{
	u32 sd_size;
	
	usart1_transmit_string("\r\nservice_sd_get_capacity()\r\n");	

	sd_size = SD_GetCapacity();

	usart1_transmit_string_format("\r\nCapacity is %d Bytes\r\n", sd_size);
	
}

void service_sd_file_list(void)
{
	FATFS fs;            // Work area (file system object) for logical drive

	FILINFO finfo;
	DIR dirs;
	char path[50] = {""}; 
  
	usart1_transmit_string("\r\nservice_sd_file_list()\r\n");	

	f_mount(0,&fs);

	//usart1_transmit_string("\r\nf_mount()\r\n");	

	if (f_opendir (&dirs, path) == FR_OK) 
  	{
  		//usart1_transmit_string("\r\nf_opendir()\r\n");	
      	while (f_readdir (&dirs, &finfo) == FR_OK)  
    		{
    			//usart1_transmit_string("\r\nf_readdir()\r\n");	
			//usart1_transmit_string_format("%s", finfo.fname);	
      		if (!(finfo.fattrib & AM_DIR)) 
      		{
      			
        			if (!finfo.fname[0])	
        			{
        				//usart1_transmit_string("\r\nfinfo.fname\r\n");	
          				break;         

				}
        			usart1_transmit_string_format ("\r\n%s", finfo.fname);
      		}                      
      	}

		usart1_transmit_string("\r\n");	
	}
	
}

void service_sd_exit(void)
{
	s_menu_level = MainService;
	s_current_service = sdExitService;
	usart1_transmit_string("\r\nservice_sd_exit()\r\n");
}

void service_usb_start(void)
{

#ifdef USB_DISCONNECT_CONFIG
	USB_Cable_Config(ENABLE);
	USB_Init();
	usart1_transmit_string("USB 2.0 FS(Full speed) Initialize.\r\n");
#endif

	usart1_transmit_string("\r\nservice_usb_start()\r\n");		
	usart1_transmit_string("Start USB mouse control.\r\n");
	usart1_transmit_string("Press 'L', 'R', 'U', 'D' Key for control PC mouse cursor.\r\n");

	s_current_service = usbStartService;
}

void service_usb_stop(void)
{
	usart1_transmit_string("\r\nservice_usb_stop()\r\n");	
	usart1_transmit_string("Stop USB mouse control.\r\n");

	s_current_service = usbStopService;
}

void service_usb_exit(void)
{
	s_menu_level = MainService;
	s_current_service = usbExitService;
	usart1_transmit_string("\r\nservice_usb_exit()\r\n");
}

void service_lcd_start(void)
{
	usart1_transmit_string("\r\nservice_lcd_start()\r\n");

	LCD_Init();
	
	s_current_service = lcdStartService;

	
}

void service_lcd_stop(void)
{
	usart1_transmit_string("\r\nservice_lcd_stop()\r\n");

	s_current_service = lcdStopService;
}

void service_lcd_exit(void)
{
	s_menu_level = MainService;
	s_current_service = lcdExitService;
	usart1_transmit_string("\r\nservice_lcd_exit()\r\n");
}

void lcd_test(void)
{
	if( s_lcd_test_cnt > 11 )
		s_lcd_test_cnt = 0;
	
	switch(s_lcd_test_cnt)
	{
		case 0:
			LCD_Clear(WHITE);
			break;
		case 1:
			LCD_Clear(BLACK);
			break;
		case 2:
			LCD_Clear(BLUE);
			break;
		case 3:
			LCD_Clear(RED);
			break;
		case 4:
			LCD_Clear(MAGENTA);
			break;
		case 5:
			LCD_Clear(GREEN);
			break;
		case 6:
			LCD_Clear(CYAN);
			break;
		case 7:
			LCD_Clear(YELLOW);
			break;
		case 8:
			LCD_Clear(BRRED);
			break;
		case 9:
			LCD_Clear(GRAY);
			break;
		case 10:
			LCD_Clear(LGRAY);
			break;
		case 11:
			LCD_Clear(BROWN);
			break;
		default:
			LCD_Clear(WHITE);
			break;				
	}
	
	POINT_COLOR=RED;	  
	LCD_ShowString(30,50, "--- STM32 ---");	
	LCD_ShowString(30,70, "2.8' TFT LCD TEST");	
	LCD_ShowString(30,90, "Written by JK Electronics");
	LCD_ShowString(30,110, "http://www.deviceshop.net");
	LCD_ShowString(30,130, "Date : 2011/4/18");
	
	s_lcd_test_cnt++;
	
}

void service_touch_start(void)
{
	usart1_transmit_string("\r\nTest ADS7846 touch screen driver..\r\n");
	usart1_transmit_string("\r\nExit press Key1.\r\n");
	
	s_current_service = touchStartService;

	touch_test();
}

void service_touch_stop(void)
{
	usart1_transmit_string("\r\nservice_touch_stop()\r\n");

	s_current_service = touchStopService;
}

void service_touch_exit(void)
{
	s_menu_level = MainService;
	s_current_service = touchExitService;
	usart1_transmit_string("\r\nservice_touch_exit()\r\n");
}

void Load_Drow_Dialog(void)
{
	LCD_Clear(WHITE);
 	POINT_COLOR=BLUE;
	LCD_ShowString(216,0,"RST");
  	POINT_COLOR=RED;
}

void touch_test(void)
{
	s_key = KEY_NONE;

	bsp_key_interrupt_init();

	register_exti_key_function(extiKey1ServiceFunction, key1_interrupt_event);
#ifdef STM32_MIDDLE_HW_VER1	
	register_exti_key_function(extiKey2ServiceFunction, key2_interrupt_event);
#endif
	
	LCD_Init();
	bsp_touch_Init();

	while(!(s_key & KEY_KEY1))
	{
		if(Pen_Point.Key_Sta==Key_Down)
		{
			Pen_Int_Set(0);
			do
			{
				Convert_Pos();
				Pen_Point.Key_Sta=Key_Up;
				if(Pen_Point.X0>216&&Pen_Point.Y0<16)
					Load_Drow_Dialog();
				else 
				{
					Draw_Big_Point(Pen_Point.X0,Pen_Point.Y0);
					GPIOC->ODR|=1<<1;
				}
			} while(PEN==0);
			
			Pen_Int_Set(1);
		}
		else 
			delay_ms(1);
	}
	
}


void service_key_polling_start(void)
{
	usart1_transmit_string("\r\nservice_key_polling_start()\r\n");

	usart1_transmit_string("Press Key1.\r\n");

	s_current_service = keyStartPollingService;

	s_key = KEY_NONE;	

	bsp_key_gpio_init();
}

void service_key_polling_stop(void)
{
	usart1_transmit_string("\r\nservice_key_polling_stop()\r\n");

	s_current_service = keyStopPollingService;
}

void service_key_interrupt_start(void)
{
	usart1_transmit_string("\r\nservice_key_interrupt_start()\r\n");

	usart1_transmit_string("Press Key1.\r\n");

	s_current_service = keyStartInterruptService;

	s_key = KEY_NONE;

	bsp_key_interrupt_init();

	register_exti_key_function(extiKey1ServiceFunction, key1_interrupt_event);
#ifdef STM32_MIDDLE_HW_VER1	
	register_exti_key_function(extiKey2ServiceFunction, key2_interrupt_event);
#endif
}

void service_key_interrupt_stop(void)
{
	usart1_transmit_string("\r\nservice_key_interrupt_stop()\r\n");

	s_current_service = keyStopInterruptService;
}

void service_key_exit(void)
{
	s_menu_level = MainService;
	s_current_service = keyExitService;
	usart1_transmit_string("\r\nservice_key_exit()\r\n");
}

void gpio_polling_key(void)
{

	if( (!GPIO_ReadInputDataBit(KEY_USER_PORT, KEY1_USER_PIN )) && !(s_key & KEY_KEY1) )
	{
		usart1_transmit_string("Key1 Pressed.\r\n");
		s_key |= KEY_KEY1;
	}
	else if( (GPIO_ReadInputDataBit(KEY_USER_PORT, KEY1_USER_PIN )) && (s_key & KEY_KEY1) )
	{
		usart1_transmit_string("Key1 Released.\r\n");
		s_key &= ~KEY_KEY1;
	}

#ifdef STM32_MIDDLE_HW_VER1	
	if( (!GPIO_ReadInputDataBit(KEY_USER_PORT, KEY2_USER_PIN )) && !(s_key & KEY_KEY2) )
	{
		usart1_transmit_string("Key2 Pressed.\r\n");
		s_key |= KEY_KEY2;
	}
	else if( (GPIO_ReadInputDataBit(KEY_USER_PORT, KEY2_USER_PIN )) && (s_key & KEY_KEY2) )
	{
		usart1_transmit_string("Key2 Released.\r\n");
		s_key &= ~KEY_KEY2;
	}
#endif	

}

void key1_interrupt_event(void)
{	
	wait_10ms(1);

	usart1_transmit_string("key1_interrupt_event\r\n");
	
	if( (!GPIO_ReadInputDataBit(KEY_USER_PORT, KEY1_USER_PIN )) && !(s_key & KEY_KEY1) )
	{
		usart1_transmit_string("Key1 Pressed.\r\n");
		s_key |= KEY_KEY1;
	}
	else if( (GPIO_ReadInputDataBit(KEY_USER_PORT, KEY1_USER_PIN )) && (s_key & KEY_KEY1) )
	{
		usart1_transmit_string("Key1 Released.\r\n");
		s_key &= ~KEY_KEY1;
	}
}

#ifdef STM32_MIDDLE_HW_VER1	
void key2_interrupt_event(void)
{

	wait_10ms(1);

	usart1_transmit_string("key2_interrupt_event\r\n");
	
	if( (!GPIO_ReadInputDataBit(KEY_USER_PORT, KEY2_USER_PIN )) && !(s_key & KEY_KEY2) )
	{
		usart1_transmit_string("Key2 Pressed.\r\n");
		s_key |= KEY_KEY2;
	}
	else if( (GPIO_ReadInputDataBit(KEY_USER_PORT, KEY2_USER_PIN )) && (s_key & KEY_KEY2) )
	{
		usart1_transmit_string("Key2 Released.\r\n");
		s_key &= ~KEY_KEY2;
	}


}
#endif

void service_rf_start_rx(void)
{
	u8 recv_buf[33];
	
	usart1_transmit_string("\r\nservice_rf_start_rx()\r\n");

	s_current_service = rfStartRxService;

	bsp_key_interrupt_init();

	register_exti_key_function(extiKey1ServiceFunction, key1_interrupt_event);	
#ifdef STM32_MIDDLE_HW_VER1	
	register_exti_key_function(extiKey2ServiceFunction, key2_interrupt_event);
#endif

	bsp_nrf24l01_init();

	delay_ms(10);

	if( nRF24L01_Check() == 1 )
	{
		usart1_transmit_string("\r\nnRF24L01 detect fail.\r\n");
	}
	else
	{
		nRF24L01_RX_Mode();
		usart1_transmit_string("\r\nWait RF Data. Exit KEY1.\r\n");

		s_key = KEY_NONE;

		while(!(s_key & KEY_KEY1))
		{		
			if(nRF24L01_RxPacket(recv_buf) == 0)
			{
				recv_buf[32] = 0x00;
				usart1_transmit_string_format("\r\nReceived data = %s\r\n", recv_buf);
			}
			else
			{
				delay_us(100);
			}
		}
	}
}

void service_rf_start_tx(void)
{
#if 1
	u8 send_buf[33] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 0x00};
	
	usart1_transmit_string("\r\nservice_rf_start_tx()\r\n");

	s_current_service = rfStartTxService;

	bsp_nrf24l01_init();	

	delay_ms(10);	

	if( nRF24L01_Check() == 1 )
	{
		usart1_transmit_string("\r\nnRF24L01 detect fail.\r\n");
	}
	else
	{
		nRF24L01_TX_Mode();
		usart1_transmit_string("\r\nSend RF Data.\r\n");

		if(nRF24L01_TxPacket(send_buf)==TX_OK)
		{
			usart1_transmit_string_format("\r\nSent data = %s\r\n", send_buf);
		}
		else
		{
			usart1_transmit_string("\r\nFail Send data\r\n");
		}
	}	
#endif	

}

#ifdef NRF24L01_BPS_CHECK
void service_rf_bps_start_rx(void)
{
	u8 recv_buf[33];
	
	usart1_transmit_string("\r\nservice_rf_bps_start_rx()\r\n");

	s_current_service = rfStartRxBpsService;

	bsp_key_interrupt_init();

	register_exti_key_function(extiKey1ServiceFunction, key1_interrupt_event);	
#ifdef STM32_MIDDLE_HW_VER1	
	register_exti_key_function(extiKey2ServiceFunction, key2_interrupt_event);
#endif

	bsp_nrf24l01_init();

	delay_ms(10);

	if( nRF24L01_Check() == 1 )
	{
		usart1_transmit_string("\r\nnRF24L01 detect fail.\r\n");
	}
	else
	{
		nRF24L01_RX_Mode();
		// usart1_transmit_string("\r\nWait RF Data. Exit KEY1.\r\n");

		s_key = KEY_NONE;

		while(!(s_key & KEY_KEY1))
		{		
			if(nRF24L01_RxPacket(recv_buf) == 0)
			{
				recv_buf[0] = 0x00;
				// usart1_transmit_string_format("\r\nReceived data = %s\r\n", recv_buf);
			}
			else
			{
				delay_us(10);
			}

		}
	}
}

void service_rf_bps_start_tx(void)
{

	u8 i;
	u16 send_count = 0;
	u8 send_buf[33] = {0x00};
	
	usart1_transmit_string("\r\nservice_rf_bps_start_tx()\r\n");

	s_current_service = rfStartTxBpsService;

	bsp_nrf24l01_init();	

	delay_ms(10);	

	if( nRF24L01_Check() == 1 )
	{
		usart1_transmit_string("\r\nnRF24L01 detect fail.\r\n");
	}
	else
	{
		for(i=0;i<32;i++)
			send_buf[i] = i;
		
		nRF24L01_TX_Mode();		

		s_rf_bps_check_cnt = 0;

		while(s_rf_bps_check_cnt < 1000 )
		{
			if(nRF24L01_TxPacket(send_buf) != TX_OK)
			{
				usart1_transmit_string("\r\nFail Send data\r\n");
			}
			else
			{
				send_count++;
			}
		}

		usart1_transmit_string_format("\r\nSend Speed = %dbps\r\n", 32*send_count*8);
	}	
	
}
#endif

void service_rf_stop(void)
{
	usart1_transmit_string("\r\nservice_rf_stop()\r\n");

	s_current_service = rfStopService;
}

void service_rf_exit(void)
{
	s_menu_level = MainService;
	usart1_transmit_string("\r\nservice_rf_exit()\r\n");
}

void service_ov7670_start(void)
{
	
	s_current_service = cameraStartOv7670Service;
	
	usart1_transmit_string("\r\nservice_ov7670_start()\r\n");
	usart1_transmit_string("\r\nKEY1 press --> Save BMP to SD Memory.\r\n");

	bsp_sd_gpio_init();

	if( disk_initialize(0) == 0 )
		usart1_transmit_string("\r\ndisk_initialize success.\r\n");
	else
		usart1_transmit_string("\r\ndisk_initialize failed.\r\n");


	//LCD_Init();
	LCD_Init();

	LCD_Clear(BLACK);	

#if 0
	LCD_WriteReg(0x0020,0);		// GRAM horizontal start position
	LCD_WriteReg(0x0021,319);     
	
	LCD_WriteReg(0x0050,0x00);		// horizontal direction GRAM start address
	LCD_WriteReg(0x0051,239);		// horizontal direction GRAM end address
	
	LCD_WriteReg(0x0052,0x00);		// vertical direction GRAM start address
	LCD_WriteReg(0x0053,319);		// vertical direction GRAM end address	

	// Display를 맨 마지막위치 에서부터 decrement 하면서 Display 하도록 한다.
	LCD_WriteReg(0x0003,0x1018); 		

	// GRAM prepare
	LCD_WR_REG(R34);	
#endif	

	//while(1 !=ov7670_init() );
	//delay_ms(100);
	while(1 !=ov7670_init() );
	delay_ms(100);
	
	usart1_transmit_string("\r\nov7670_Initialize successed.\r\n");

	ov7670_set_qvga();
	delay_ms(1);
	usart1_transmit_string("\r\nov7670_set_qvga.\r\n");

	//wrOV7670Reg(REG_BRIGHT,0x50);	
	//wrOV7670Reg(REG_CONTRAST,0x60);	
	
	s_key = KEY_NONE;
	
	while(1)
	{

		if( capture_button_status == 1 )
		{

		  	if( vsync_count == 2 )
		  	{
				FIFO_RRST_L(); 
				GPIOE->BRR  =FIFO_RD_PIN;  
				GPIOE->BSRR =FIFO_RD_PIN; 							
				FIFO_RRST_H();			
				GPIOE->BRR  =FIFO_RD_PIN;  
				GPIOE->BSRR =FIFO_RD_PIN;					

				for(x_vert=0;x_vert<240;x_vert++)
				{
					for(y_horizen=0;y_horizen<320;y_horizen++)
					{
						GPIOE->BRR  =FIFO_RD_PIN;
						//__nop();
						//__nop();
						GPIOE->BSRR =FIFO_RD_PIN;
						//__nop();
						//__nop();
					
						val_rgb=((unsigned char)GPIOD->IDR); 
						val_rgb=(val_rgb<<8);

						GPIOE->BRR  =FIFO_RD_PIN; 
						//__nop();
						//__nop();
						GPIOE->BSRR =FIFO_RD_PIN;
						//__nop();
						//__nop();

						val_rgb|=(unsigned char)GPIOD->IDR; 
						LCD_WR_DATA(val_rgb);				

					}
				  
				}	
#ifdef OV7670_SW_V2			
				vsync_count = 0;
				//usart1_transmit_string_format("\r\npreview");
#endif				
		  	}

		}
		// KEY1을 누르면 SD메모리에 파일로 저장 한다.
		else if( capture_button_status == 0 )
		{
			if(capture_status==3)
			{

				usart1_transmit_string("Capture start and ov7670_set_cif().\r\n");

				f_mount(0, &fs); 

				file_name[6]++;
				if(file_name[6]>'9')
				{
				  file_name[6]='0';
				  file_name[5]++;
				  if(file_name[5]>'9')
				  {
				    file_name[5]='0';
				    file_name[4]++;
				    if(file_name[4]>'9')
				      {
				         file_name[4]='0';
				         file_name[3]++;            
				      } 
				  }
				}

				res = f_open(&fsrca,file_name, FA_CREATE_ALWAYS | FA_WRITE);  
				usart1_transmit_string_format("\r\nf_open=%s, res=%d\r\n", file_name, res);
				res = f_write(&fsrca, bmp_tou,54, &bw); 
				//usart1_transmit_string_format("\r\nres=%d, bw=%d\r\n", res, bw);
		

			  
				FIFO_RRST_L(); 
				GPIOE->BRR  =FIFO_RD_PIN;  
				GPIOE->BSRR =FIFO_RD_PIN;
				FIFO_RRST_H();			  
				GPIOE->BRR  =FIFO_RD_PIN;  
				GPIOE->BSRR =FIFO_RD_PIN;	
			

				for(x_vert=0;x_vert<240/*288*/;x_vert++)
				{
					for(y_horizen=0;y_horizen<320/*352*/;y_horizen++)
					{
						GPIOE->BRR  =FIFO_RD_PIN; 
						GPIOE->BSRR =FIFO_RD_PIN;

						val_rgb = ((unsigned char)GPIOD->IDR); 
						buffera[1] = ((unsigned char)val_rgb)>>1; 
						val_rgb=(val_rgb<<8);

						GPIOE->BRR  =FIFO_RD_PIN; 
						GPIOE->BSRR =FIFO_RD_PIN;

						val_rgb |= (unsigned char)GPIOD->IDR; 


						buffera[0] = (unsigned char)(val_rgb&0x1f)|((val_rgb>>1)&0xe0); 
						res = f_write(&fsrca, buffera,2, &bw);   
						//usart1_transmit_string_format("\r\nres=%d, bw=%d\r\n", res, bw);
					}
				  
				}					


				f_close(&fsrca); 
				f_mount(0, NULL);   

				capture_button_status = 1;
				capture_status = 0;
				
				//ov7670_set_qvga();

				//usart1_transmit_string_format("\r\nf_close\r\n");
				//usart1_transmit_string_format("\r\nov7670_set_qvga\r\n");
			}
		}



	}


}

void service_ov7670_stop(void)
{
	usart1_transmit_string("\r\nservice_ov7670_stop()\r\n");

	s_current_service = cameraStopOv7670Service;
}

void service_ov7670_exit(void)
{
	s_menu_level = MainService;
	usart1_transmit_string("\r\nservice_ov7670_exit()\r\n");
}

/*******************************************************************************
* Function Name  : tim_display
* Description    : Displays the current time.
* Input          : - TimeVar: RTC counter value.
* Output         : None
* Return         : None
*******************************************************************************/
void tim_display(u32 TimeVar)
{ 
	u32 THH = 0, TMM = 0, TSS = 0;

	/* Compute  hours */
	THH = TimeVar/3600;
	/* Compute minutes */
	TMM = (TimeVar % 3600)/60;
	/* Compute seconds */
	TSS = (TimeVar % 3600)% 60;

	usart1_transmit_string_format("\r\nTime: %0.2d:%0.2d:%0.2d",THH, TMM, TSS);
}

/*******************************************************************************
* Function Name  : Time_Show
* Description    : Shows the current time (HH:MM:SS) on the Hyperterminal.
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void time_show(void)
{
	// usart1_transmit_string("\n\r");
	tim_display(RTC_GetCounter());

#if 0	

	/* Infinite loop */ 
	while(1)
	{
		/* If 1s has paased */
		if(time_display == 1)
		{    
			/* Display current time */
			tim_display(RTC_GetCounter());
			time_display = 0;
		}
	}
#endif
}

void usb_mouse_control(s8 Keys)
{
	u8 mouse_buffer[4] = {0, 0, 0, 0};
	s8 x = 0, y = 0;

	switch (Keys)
	{
		case 'l' :
		case 'L':
			x += 10;
			break;

		case 'r':
		case 'R':
			x -= 10;
			break;
		case 'u':
		case 'U':
			y -= 10;
			break;
		case 'd':
		case 'D':
			y += 10;
			break;
		default:
			return;
	}

	/* prepare buffer to send */
	mouse_buffer[0] = 0x00;
	mouse_buffer[1] = x;
	mouse_buffer[2] = y;

#if 0
	/*copy mouse position info in ENDP1 Tx Packet Memory Area*/
	UserToPMABufferCopy(mouse_buffer, GetEPTxAddr(ENDP1), 4);
	if(mouse_buffer[0]!= 0)
	{
		mouse_buffer[0] = 0;
		UserToPMABufferCopy(mouse_buffer, GetEPTxAddr(ENDP1), 4);
	} 
	
	/* enable endpoint for transmission */
	SetEPTxValid(ENDP1);
#endif

	/* Copy mouse position info in ENDP1 Tx Packet Memory Area*/
	USB_SIL_Write(EP1_IN, mouse_buffer, 4);

#ifndef STM32F10X_CL
	/* Enable endpoint for transmission */
	SetEPTxValid(ENDP1);
#endif /* STM32F10X_CL */

}



		


#ifdef  USE_FULL_ASSERT
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
