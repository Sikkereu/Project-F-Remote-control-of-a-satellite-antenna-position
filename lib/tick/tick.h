/**
 * @file    tick.h
 * @brief   Periodic tick from basic timer TIM6, for the "do something every
 *          N ms" main loop used in assignments 1-3 (100 Hz stopwatch,
 *          10 Hz voltmeter update).
 *
 * The interrupt only counts. Everything slow, printing above all, stays in
 * the main loop:
 *
 *     tick_init(100);                     // 10 ms
 *     while (1) {
 *         if (tick_take()) {              // once per tick
 *             uint8_t p = joystick_poll();
 *             ...
 *         }
 *     }
 *
 * Why TIM6 and not TIM2 as in assignment 1: TIM6 is a basic timer with no
 * output channels, so using it for the time base costs no PWM-capable
 * timer. TIM2 stays free for PWM on PA5/PB10/PB11.
 */
#ifndef TICK_H
#define TICK_H

#include "stm32f30x_conf.h"
#include <stdint.h>

int      tick_init(uint32_t hz);
uint32_t tick_take(void);
uint32_t tick_count(void);
uint32_t tick_hz(void);
void     tick_wait(void);

#endif /* TICK_H */
