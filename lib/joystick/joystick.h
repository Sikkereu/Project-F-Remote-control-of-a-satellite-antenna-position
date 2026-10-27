/**
 * @file    joystick.h
 * @brief   Five-way joystick on the mbed application shield.
 *          From assignment 1 (polling) and 2.6 (EXTI on the centre switch).
 *
 * ALPS navswitch SW1: common terminal to +3V3, each line has a 4k7
 * pull-down (R7..R11), so pressed = 1, released = 0.
 *
 *   PA4 up (A2), PB0 down (A3), PC1 left (A4), PC0 right (A5), PB5 centre (D4)
 *
 * Three ways to use it:
 *
 *   1. joystick_read()  raw levels, no debouncing.
 *   2. joystick_poll()  call it at a fixed rate, 5-20 ms apart (for example
 *                       on every tick_take()). Returns the switches that were
 *                       newly pressed, debounced. A switch held down is
 *                       reported once.
 *   3. joystick_irq_enable(mask)  EXTI interrupt on the rising edge of the
 *                       chosen lines; read the events with joystick_irq_take().
 *                       Contact bounce can give several interrupts per
 *                       press, so treat this as "something happened" and
 *                       debounce in the main loop.
 *
 * EXTI limitation: DOWN (PB0) and RIGHT (PC0) are both pin 0 and share EXTI
 * line 0, and only one port can own a line (SYSCFG_EXTICR1). Asking for both
 * returns -1.
 *
 * This file defines EXTI0/EXTI1/EXTI4/EXTI9_5_IRQHandler. If an assignment
 * needs its own handler for one of those, build with
 * -D JOYSTICK_NO_IRQ_HANDLERS and joystick_irq_* is left out.
 */
#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "stm32f30x_conf.h"
#include <stdint.h>

/* Bit positions in the byte the functions return (order from assignment 1). */
#define JOY_UP      (1u << 0)
#define JOY_DOWN    (1u << 1)
#define JOY_LEFT    (1u << 2)
#define JOY_RIGHT   (1u << 3)
#define JOY_CENTER  (1u << 4)
#define JOY_ANY     (JOY_UP | JOY_DOWN | JOY_LEFT | JOY_RIGHT | JOY_CENTER)

void    joystick_init(void);
uint8_t joystick_read(void);
uint8_t joystick_poll(void);
uint8_t joystick_state(void);

#ifndef JOYSTICK_NO_IRQ_HANDLERS
int     joystick_irq_enable(uint8_t mask, uint8_t priority);
uint8_t joystick_irq_take(void);
#endif

#endif /* JOYSTICK_H */
