/**
 * @file    gpio.h
 * @brief   Short wrappers around the SPL GPIO calls that every driver repeats:
 *          clock enable, then GPIO_StructInit / fill in / GPIO_Init.
 *
 * `pins` is a mask (GPIO_Pin_x | GPIO_Pin_y), so several pins on the same
 * port can be set up in one call. Each function enables the port clock
 * itself, so the caller never has to remember RCC_AHBENR.
 */
#ifndef GPIO_H
#define GPIO_H

#include "stm32f30x_conf.h"
#include <stdint.h>

void    gpio_clock_enable(GPIO_TypeDef *port);

/* pupd: GPIO_PuPd_NOPULL, GPIO_PuPd_UP or GPIO_PuPd_DOWN */
void    gpio_init_input(GPIO_TypeDef *port, uint16_t pins, GPIOPuPd_TypeDef pupd);

/* Push-pull output, no pull. speed: GPIO_Speed_2MHz, GPIO_Speed_10MHz or
 * GPIO_Speed_50MHz. Slow edges are fine for LEDs and radiate less. */
void    gpio_init_output(GPIO_TypeDef *port, uint16_t pins, GPIOSpeed_TypeDef speed);

/* Analog mode, for ADC inputs. Disconnects the digital input buffer. */
void    gpio_init_analog(GPIO_TypeDef *port, uint16_t pins);

/* Alternate function, push-pull, 50 MHz. `pin` is ONE pin (GPIO_Pin_x),
 * `af` is GPIO_AF_0..GPIO_AF_15 from [DS] Tables 14-18. The AF number is
 * written before the pin is switched to AF mode, so the pin is never
 * briefly connected to AF0. pupd as for gpio_init_input(). */
void    gpio_init_af(GPIO_TypeDef *port, uint16_t pin, uint8_t af, GPIOPuPd_TypeDef pupd);

/* Pin number 0..15 for a single-bit GPIO_Pin_x mask (GPIO_Pin_6 -> 6). */
uint8_t gpio_pin_number(uint16_t pin);

static inline void    gpio_set(GPIO_TypeDef *port, uint16_t pins)    { port->BSRR = pins; }
static inline void    gpio_clear(GPIO_TypeDef *port, uint16_t pins)  { port->BRR  = pins; }
static inline void    gpio_toggle(GPIO_TypeDef *port, uint16_t pins) { port->ODR ^= pins; }
static inline uint8_t gpio_read(GPIO_TypeDef *port, uint16_t pin)    { return (port->IDR & pin) ? 1u : 0u; }

#endif /* GPIO_H */
