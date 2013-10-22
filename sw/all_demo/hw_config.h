/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : hw_config.h
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : Hardware Configuration & Setup
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H

/* Includes ------------------------------------------------------------------*/
#include "usb_type.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define  INTLOCK( )  
#define  INTFREE( )     

#undef STM32_MIDDLE_HW_VER1
#define STM32_MIDDLE_HW_VER2
#define HSE_8MHZ	// 크리스탈이 8MHZ가 아니면 여기를 수정 하세요.
#define USB_DISCONNECT_CONFIG

#ifdef USB_DISCONNECT_CONFIG
#define USB_DISCONNECT				GPIOA
#define USB_DISCONNECT_PIN			GPIO_Pin_1
//#define USB_DISCONNECT				GPIOC
//#define USB_DISCONNECT_PIN			GPIO_Pin_13
#endif

#undef NRF24L01_BPS_CHECK


#define JTAG_SWD_DISABLE   0X02
#define SWD_ENABLE         0X01
#define JTAG_SWD_ENABLE    0X00	

/* Exported functions ------------------------------------------------------- */
extern void bsp_init_rcc(void);
extern void bsp_init_gpio(void);
extern void bsp_init_interrupt(void);
extern void JTAG_Set (u8 mode);

void Set_USBClock(void);
void Enter_LowPowerMode(void);
void Leave_LowPowerMode(void);
void USB_Cable_Config (FunctionalState NewState);
void Get_SerialNum(void);

void delay_us (const uc32 usec);
void delay_ms (const uc32 usec);

#endif  /*__HW_CONFIG_H*/

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
