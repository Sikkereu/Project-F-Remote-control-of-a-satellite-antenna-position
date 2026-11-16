#ifndef UART1_H_
#define UART1_H_

#include "stm32f30x_conf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void initUART1(uint32_t baudrate);
void uart1SendFloat(float number);
void uart1Send3Floats(float number1, float number2, float number3);
void uart1ReceiveFloat(float *number);


#endif /* UART1_H_ */
