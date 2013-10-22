

#include  <stm32f10x.h>
#include "key.h"
#include "ov7670.h"


const bsp_key_group_type key_group[keyMax]=
{
	{KEY_USER_PORT, KEY1_USER_PIN }
#ifdef STM32_MIDDLE_HW_VER1		
	,{KEY_USER_PORT, KEY2_USER_PIN }
#endif
};

exti_key_service_function_type gbl_ar_exti_key_service[extiKeyServiceFunctionMAX] = 
{
	{extiKey1ServiceFunction, NULL}
#ifdef STM32_MIDDLE_HW_VER1	
	,{extiKey2ServiceFunction, NULL}
#endif
};

void register_exti_key_function(exti_key_register_function_type exti_key_fn_type, exti_key_register_function fn)
{
	gbl_ar_exti_key_service[exti_key_fn_type].run = fn;
}

// Camera EXTI
exti_camera_service_function_type gbl_ar_exti_camera_service[extiCameraServiceFunctionMAX] = 
{
	{extiCameraVsyncServiceFunction, NULL},
	{extiCameraPclkServiceFunction, NULL},
	{extiCameraHrefServiceFunction, NULL}
};

void register_camera_key_function(exti_camera_register_function_type exti_camera_fn_type, exti_camera_register_function fn)
{
	gbl_ar_exti_camera_service[exti_camera_fn_type].run = fn;
}

void bsp_key_gpio_init(void)
{
	
	EXTI_InitTypeDef EXTI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure the GPIO ports( key2 ) */
	GPIO_InitStructure.GPIO_Pin =  KEY1_USER_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(KEY_USER_PORT, &GPIO_InitStructure);	

#ifdef STM32_MIDDLE_HW_VER1		
	GPIO_InitStructure.GPIO_Pin =  KEY2_USER_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(KEY_USER_PORT, &GPIO_InitStructure);		
#endif	

	
	// interrupt disable
	/* Clear EXTI Line Pending Bit */
	EXTI_ClearITPendingBit(KEY1_IRQ_EXTI_Line);
#ifdef STM32_MIDDLE_HW_VER1	
	EXTI_ClearITPendingBit(KEY2_IRQ_EXTI_Line);
#endif
	
	/* Configure EXTI  to generate an interrupt on falling edge */
	EXTI_InitStructure.EXTI_Line = KEY1_IRQ_EXTI_Line;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStructure);	

#ifdef STM32_MIDDLE_HW_VER1	
	EXTI_InitStructure.EXTI_Line = KEY2_IRQ_EXTI_Line;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStructure);		
#endif

		
}

void bsp_key_interrupt_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;


	/* Configure the GPIO ports( key2 ) */
	GPIO_InitStructure.GPIO_Pin =  KEY1_USER_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(KEY_USER_PORT, &GPIO_InitStructure);		

#ifdef STM32_MIDDLE_HW_VER1	
	GPIO_InitStructure.GPIO_Pin =  KEY2_USER_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(KEY_USER_PORT, &GPIO_InitStructure);		
#endif

	/* Connect EXTI */	
	GPIO_EXTILineConfig(KEY_IRQ_PORT_SOURCE, KEY1_IRQ_PIN_SOURCE);
#ifdef STM32_MIDDLE_HW_VER1	
	GPIO_EXTILineConfig(KEY_IRQ_PORT_SOURCE, KEY2_IRQ_PIN_SOURCE);
#endif
	

	/* Configure EXTI  to generate an interrupt on falling edge */
	EXTI_InitStructure.EXTI_Line = KEY1_IRQ_EXTI_Line;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

#ifdef STM32_MIDDLE_HW_VER1	
	EXTI_InitStructure.EXTI_Line = KEY2_IRQ_EXTI_Line;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
#endif	


	/* 2 bit for pre-emption priority, 2 bits for subpriority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitStructure.NVIC_IRQChannel = KEY1_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	

#ifdef STM32_MIDDLE_HW_VER1	
	NVIC_InitStructure.NVIC_IRQChannel = KEY2_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
#endif	

	/* Clear EXTI Line Pending Bit */	
	EXTI_ClearITPendingBit(KEY1_IRQ_EXTI_Line);	
#ifdef STM32_MIDDLE_HW_VER1	
	EXTI_ClearITPendingBit(KEY2_IRQ_EXTI_Line);	
#endif
	
	/* Enable the Key EXTI line Interrupt */
	NVIC_ClearPendingIRQ(KEY1_IRQ_CHANNEL);		
#ifdef STM32_MIDDLE_HW_VER1	
	NVIC_ClearPendingIRQ(KEY2_IRQ_CHANNEL);
#endif
	
	
}

void EXTI0_IRQHandler(void)
{


}

void EXTI2_IRQHandler(void)
{
	
	if(EXTI_GetITStatus(KEY1_IRQ_EXTI_Line) != RESET)
	{		 
		EXTI_ClearITPendingBit(KEY1_IRQ_EXTI_Line);			
		if( gbl_ar_exti_key_service[extiKey1ServiceFunction].run != NULL )
		{
			gbl_ar_exti_key_service[extiKey1ServiceFunction].run();
		}		
	}	

#ifdef CAM_PCLK
	if(EXTI_GetITStatus(CAMERA_PCLK_IRQ_EXTI_Line) != RESET)
	{		 
		EXTI_ClearITPendingBit(CAMERA_PCLK_IRQ_EXTI_Line);			
		if( gbl_ar_exti_camera_service[extiCameraPclkServiceFunction].run != NULL )
		{
			gbl_ar_exti_camera_service[extiCameraPclkServiceFunction].run();
		}		
	}
#endif
}


void EXTI4_IRQHandler(void)
{

#ifdef CAM_HREF
	if(EXTI_GetITStatus(CAMERA_HREF_IRQ_EXTI_Line) != RESET)
	{		 
		EXTI_ClearITPendingBit(CAMERA_HREF_IRQ_EXTI_Line);			
		if( gbl_ar_exti_camera_service[extiCameraHrefServiceFunction].run != NULL )
		{
			gbl_ar_exti_camera_service[extiCameraHrefServiceFunction].run();
		}		
	}
#endif
}

void EXTI9_5_IRQHandler(void)
{

	if(EXTI_GetITStatus(CAMERA_VSYNC_IRQ_EXTI_Line) != RESET)
	{		 
		EXTI_ClearITPendingBit(CAMERA_VSYNC_IRQ_EXTI_Line);			
		if( gbl_ar_exti_camera_service[extiCameraVsyncServiceFunction].run != NULL )
		{
			gbl_ar_exti_camera_service[extiCameraVsyncServiceFunction].run();
		}		
	}	
}



void EXTI15_10_IRQHandler(void)
{




}

