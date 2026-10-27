/**
 * @file    delay.h
 * @brief   Millisecond clock and blocking delays from the Cortex-M4 core
 *          (SysTick and the DWT cycle counter). Uses no STM32 peripheral,
 *          so it cannot collide with any timer used for PWM or ticks.
 *
 *     delay_init();                  // once, early in main()
 *     uint32_t t0 = millis();
 *     delay_ms(250);
 *     delay_us(10);                  // e.g. a sensor start-up time
 *     uint32_t elapsed = millis() - t0;   // unsigned subtraction handles wrap
 *
 * Do not call delay_ms() from an interrupt handler: the millisecond count is
 * itself an interrupt (priority 15, the lowest) and would never advance.
 * delay_us() does not depend on interrupts and works anywhere.
 */
#ifndef DELAY_H
#define DELAY_H

#include "stm32f30x_conf.h"
#include <stdint.h>

void     delay_init(void);
uint32_t millis(void);
void     delay_ms(uint32_t ms);
void     delay_us(uint32_t us);

#endif /* DELAY_H */
