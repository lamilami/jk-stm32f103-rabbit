/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************
* File Name          : main.h
* Author             : MCD Application Team
* Version            : V2.0.0
* Date               : 04/27/2009
* Description        : Header for main.c module
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "hw_config.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void bsp_init_rcc(void);
void bsp_init_gpio(void);
void bsp_init_interrupt(void);
ErrorStatus Get_HSEStartUpStatus(void);


typedef enum
{
	MainService,
	Ledservice,
	TimerService,
	RtcService,
	SdService,	
	UsbService,
	LcdService,
	TouchService,
	KeyService,	
	RfService,
	CameraService,
	serviceMAX
} service_define_type;


typedef enum
{
	ledUserCoreOnService,
	ledUserCoreOffService,
	ledUserBottomOnService,
	ledUserBottomOffService,
	ledExitService,
	ledServiceMAX
} service_led_type;

typedef enum
{
	timerTIM2Service,
	timerTIM2Stop,		
	timerExitService,
	timerServiceMAX
} service_timer_type;

typedef enum
{
	rtcStartService,
	rtcStopService,
	rtcExitService,	
	rtcServiceMAX
} service_rtc_type;

typedef enum
{
	sdInitializeService,
	sdGetCapacityService,
	sdFileListService,
	sdExitService,	
	sdServiceMAX
} service_sd_type;

typedef enum
{
	usbStartService,
	usbStopService,
	usbExitService,	
	usbServiceMAX
} service_usb_type;

typedef enum
{
	lcdStartService,
	lcdStopService,
	lcdExitService,	
	lcdServiceMAX
} service_lcd_type;

typedef enum
{
	touchStartService,
	touchStopService,
	touchExitService,	
	touchServiceMAX
} service_touch_type;

typedef enum
{
	keyStartPollingService,
	keyStopPollingService,
	keyStartInterruptService,
	keyStopInterruptService,
	keyExitService,	
	keyServiceMAX
} service_key_type;

typedef enum
{
	rfStartRxService,
	rfStartTxService,
#ifdef NRF24L01_BPS_CHECK	
	rfStartRxBpsService,
	rfStartTxBpsService,
#endif	
	rfStopService,
	rfExitService,	
	rfServiceMAX
} service_rf_type;

typedef enum
{
	cameraStartOv7670Service,
	cameraStopOv7670Service,
	cameraExitOv7670Service,	
	cameraServiceMAX
} service_camera_type;

typedef            void     (*service_function)(void);


typedef struct _service_type
{
	int service;
	char* service_string;
	service_function run;
	s8 cmd;	
} service_type;

void service_led(void);
void service_timer(void);
void service_rtc(void);
void service_sd(void);
void service_usb(void);
void service_lcd(void);
void service_touch(void);
void service_key(void);
void service_rf(void);
void service_camera(void);

void service_user_led_core_on(void);
void service_user_led_core_off(void);
void service_user_led_bottom_on(void);
void service_user_led_bottom_off(void);
void service_led_exit(void);

void service_tim2_ticktime(void);
void service_tim2_stop(void);
void service_timer_exit(void);

void service_rtc_start(void);
void service_rtc_stop(void);
void service_rtc_exit(void);

void service_sd_initialize(void);
void service_sd_get_capacity(void);
void service_sd_file_list(void);
void service_sd_exit(void);

void service_usb_start(void);
void service_usb_stop(void);
void service_usb_exit(void);

void service_lcd_start(void);
void service_lcd_stop(void);
void service_lcd_exit(void);
void lcd_test(void);

void service_touch_start(void);
void service_touch_stop(void);
void service_touch_exit(void);
void touch_test(void);

void service_key_polling_start(void);
void service_key_polling_stop(void);
void service_key_interrupt_start(void);
void service_key_interrupt_stop(void);
void service_key_exit(void);
void gpio_polling_key(void);
void key1_interrupt_event(void);
void key2_interrupt_event(void);

void service_rf_start_rx(void);
void service_rf_start_tx(void);
void service_rf_bps_start_rx(void);
void service_rf_bps_start_tx(void);
void service_rf_stop(void);
void service_rf_exit(void);

void service_ov7670_start(void);
void service_ov7670_stop(void);
void service_ov7670_exit(void);

void tim_display(u32 TimeVar);
void time_show(void);

void usb_mouse_control(s8 Keys);

#endif /* __MAIN_H */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
