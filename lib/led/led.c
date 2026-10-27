/**
 * @file    led.c
 * @brief   RGB LED driver. See led.h for the wiring.
 */
#include "led.h"
#include "board.h"
#include "gpio.h"

static uint8_t ledColour = LED_OFF;

/**
 * led_init  Makes the three LED pins push-pull outputs, LED off.
 *
 * ODR resets to 0, and 0 means "on" here, so the pins are driven high
 * BEFORE they become outputs. Otherwise the LED flashes white at start-up.
 */
void led_init(void)
{
    gpio_clock_enable(LED_RED_PORT);
    gpio_clock_enable(LED_GREEN_PORT);
    gpio_clock_enable(LED_BLUE_PORT);

    gpio_set(LED_RED_PORT,   LED_RED_PIN);
    gpio_set(LED_GREEN_PORT, LED_GREEN_PIN);
    gpio_set(LED_BLUE_PORT,  LED_BLUE_PIN);

    gpio_init_output(LED_RED_PORT,   LED_RED_PIN,   GPIO_Speed_2MHz);
    gpio_init_output(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_Speed_2MHz);
    gpio_init_output(LED_BLUE_PORT,  LED_BLUE_PIN,  GPIO_Speed_2MHz);

    led_set(LED_OFF);
}

/**
 * led_set  Shows one of the eight colours (LED_OFF .. LED_WHITE).
 *
 * BSRR/BRR writes are atomic per pin, so an interrupt landing between the
 * three writes cannot corrupt the other colours.
 */
void led_set(uint8_t colour)
{
    ledColour = colour & 7u;

    if (colour & LED_RED)   gpio_clear(LED_RED_PORT,   LED_RED_PIN);
    else                    gpio_set  (LED_RED_PORT,   LED_RED_PIN);
    if (colour & LED_GREEN) gpio_clear(LED_GREEN_PORT, LED_GREEN_PIN);
    else                    gpio_set  (LED_GREEN_PORT, LED_GREEN_PIN);
    if (colour & LED_BLUE)  gpio_clear(LED_BLUE_PORT,  LED_BLUE_PIN);
    else                    gpio_set  (LED_BLUE_PORT,  LED_BLUE_PIN);
}

uint8_t led_get(void)
{
    return ledColour;
}
