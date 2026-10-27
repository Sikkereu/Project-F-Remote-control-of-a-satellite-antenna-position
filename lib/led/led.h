/**
 * @file    led.h
 * @brief   RGB LED on the mbed application shield. From assignment 1.
 *
 * Common anode to +3V3, each cathode through 1k to a pin: the LED is active
 * low. led_set() hides the inversion.
 *
 *   red = PB4 (D5), green = PC7 (D9), blue = PA9 (D8)
 */
#ifndef LED_H
#define LED_H

#include "stm32f30x_conf.h"
#include <stdint.h>

/* Colour codes: bit 0 red, bit 1 green, bit 2 blue. 3 = 0b011 = red+green. */
#define LED_OFF     0u
#define LED_RED     1u
#define LED_GREEN   2u
#define LED_YELLOW  3u
#define LED_BLUE    4u
#define LED_MAGENTA 5u
#define LED_CYAN    6u
#define LED_WHITE   7u

void    led_init(void);
void    led_set(uint8_t colour);
uint8_t led_get(void);

#endif /* LED_H */
