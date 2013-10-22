
#ifndef  KEY_PRESENT
#define  KEY_PRESENT

#include "hw_config.h"
#include <stm32f10x.h>


typedef enum
{
	key1User = 0x00,
#ifdef STM32_MIDDLE_HW_VER1
	key2User = 0x01,
#endif	
	keyMax
} BSP_KEY_Def;


#define KEY_USER_PORT				GPIOA
#define KEY1_USER_PIN				GPIO_Pin_2
#ifdef STM32_MIDDLE_HW_VER1
#define KEY2_USER_PIN				GPIO_Pin_8
#endif

// KEY IRQ Pin define
#define KEY_IRQ_PORT_SOURCE			GPIO_PortSourceGPIOA
#define KEY1_IRQ_PIN_SOURCE			GPIO_PinSource2
#ifdef STM32_MIDDLE_HW_VER1
#define KEY2_IRQ_PIN_SOURCE			GPIO_PinSource8
#endif

// KEY IRQ External Line
#define KEY1_IRQ_EXTI_Line			EXTI_Line2
#ifdef STM32_MIDDLE_HW_VER1
#define KEY2_IRQ_EXTI_Line			EXTI_Line8
#endif

// KEY IRQ channel
#define KEY1_IRQ_CHANNEL			EXTI2_IRQn
#ifdef STM32_MIDDLE_HW_VER1
#define KEY2_IRQ_CHANNEL			EXTI9_5_IRQn
#endif

typedef struct
{
	GPIO_TypeDef* gpio_reg;
	u16          pin;
}bsp_key_group_type;


typedef enum
{
	extiKey1ServiceFunction = 0x00,
#ifdef STM32_MIDDLE_HW_VER1		
	extiKey2ServiceFunction = 0x01,
#endif	
	extiKeyServiceFunctionMAX
} exti_key_register_function_type;

typedef            void     (*exti_key_register_function)(void);

typedef struct _exti_key_service_function_type
{
	exti_key_register_function_type service_type;
	exti_key_register_function run;
} exti_key_service_function_type;

// Camera EXTI

typedef enum
{
	extiCameraVsyncServiceFunction = 0x00,
	extiCameraPclkServiceFunction = 0x01,
	extiCameraHrefServiceFunction = 0x02,
	extiCameraServiceFunctionMAX
} exti_camera_register_function_type;

typedef            void     (*exti_camera_register_function)(void);

typedef struct _exti_camera_service_function_type
{
	exti_camera_register_function_type service_type;
	exti_camera_register_function run;
} exti_camera_service_function_type;

/* ------------------------------------------------------------------------------------------------- */
/* BSP KEY */
/* ------------------------------------------------------------------------------------------------- */

extern void register_exti_key_function(exti_key_register_function_type exti_key_fn_type, exti_key_register_function fn);
extern void register_camera_key_function(exti_camera_register_function_type exti_camera_fn_type, exti_camera_register_function fn);
extern void bsp_key_interrupt_init(void);
extern void bsp_key_gpio_init(void);

void EXTI0_IRQHandler(void);
void EXTI2_IRQHandler(void);
void EXTI4_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
	
#endif
