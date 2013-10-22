

#ifndef  USART_PRESENT
#define  USART_PRESENT

#include <stm32f10x_usart.h>

/* ------------------------------------------------------------------------------------------------- */
/* BSP USART */
/* ------------------------------------------------------------------------------------------------- */
typedef enum
{
	usartmodeDMA,
	usartmodeIRQ,
	usartmodeMAX
} usartmode_type;

/* ------------------------------------------------------------------------------------------------- */
/* function USART */
/* ------------------------------------------------------------------------------------------------- */

void USART1_IRQHandler(void);


/* ------------------------------------------------------------------------------------------------- */
/* extern USART */
/* ------------------------------------------------------------------------------------------------- */

extern void init_usart1_buffer(void);				    
extern void usart_transmit_byte( USART_TypeDef* port, u16 chr);
extern void usart1_transmit_byte( u16 chr);
extern void usart1_tx_proc(void);

extern void usart_transmit_string(USART_TypeDef* port, const void *data);
extern void usart1_transmit_string(const void *data);
extern void usart1_transmit_string_format(const char * szFormat, ... );


extern void bsp_init_irq_usart1(void/*isr_function usart1_isr*/);

extern int usart_is_ne(USART_TypeDef* port);
extern int usart1_is_ne(void);
extern void* usart1_get_data(void);

#endif                                                          

/* End of module include.                               */

