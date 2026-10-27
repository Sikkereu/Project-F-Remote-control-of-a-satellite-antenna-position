#ifndef _30010_IO_H_
#define _30010_IO_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f30x_conf.h"
#include <stdio.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define UART_BUFFER_LENGTH 256

/* Exported functions ------------------------------------------------------- */
/****************************/
/*** USB Serial Functions ***/
/****************************/
void uart_init(uint32_t baud);
void uart_put_char(uint8_t c);
uint8_t uart_get_char();
uint8_t uart_get_count();
void uart_clear();

/* Standard project: blocking read, the uart_getc() that assignment 3
 * appendix 2 refers to. Waits (forever) for one received byte. */
uint8_t uart_getc(void);

#endif /* _30010_IO_H_ */
