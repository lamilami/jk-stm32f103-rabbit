
#ifndef  LED_PRESENT
#define  LED_PRESENT

#include "hw_config.h"
#include <stm32f10x.h>


typedef enum
{
	ledUserCore = 0x00,
	ledCoreMax
} BSP_LED_CORE_Def;

typedef enum
{
	ledUserBottom = 0x00,
	ledBottomMax
} BSP_LED_BOTTOM_Def;


#define LED_USER_CORE_PORT			GPIOC
#define LED_USER_CORE_PIN			GPIO_Pin_12

#define LED_USER_BOTTOM_PORT		GPIOA
#define LED_USER_BOTTOM_PIN		GPIO_Pin_0


typedef struct
{
	GPIO_TypeDef* gpio_reg;
	s16          pin;
}bsp_led_core_group_type;

typedef struct
{
	GPIO_TypeDef* gpio_reg;
	s16          pin;
}bsp_led_bottom_group_type;


/* ------------------------------------------------------------------------------------------------- */
/* BSP LED */
/* ------------------------------------------------------------------------------------------------- */

extern void bsp_led_gpio_init(void);
extern void bsp_led_core_on(BSP_LED_CORE_Def led);
extern void bsp_led_core_off(BSP_LED_CORE_Def led);
extern void bsp_led_core_toggle(BSP_LED_CORE_Def led);

extern void bsp_led_bottom_on(BSP_LED_BOTTOM_Def led);
extern void bsp_led_bottom_off(BSP_LED_BOTTOM_Def led);
extern void bsp_led_bottom_toggle(BSP_LED_BOTTOM_Def led);


#endif
