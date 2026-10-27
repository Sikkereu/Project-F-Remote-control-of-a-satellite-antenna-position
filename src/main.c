/**
 * @file    main.c
 * @brief   30021 standard project: hardware self-test and usage example.
 *          STM32 Nucleo-F302R8 with mbed application shield MBED-016.1.
 *
 * Replace this file with the assignment's own main.c. As it stands it
 * exercises every module in lib/, so flashing it is a quick check that the
 * board, the shield and the toolchain all work:
 *
 *   LCD      pot 1 and 2 (raw and mV), VDDA and the PWM duty, 10 Hz
 *   UART     the same once per second, 9600 8N1
 *   PA6      10 kHz PWM, 256 steps (assignment 3.1), start at 50 %
 *   joystick up/down +-16 steps, right/left +-1 step, centre back to 50 %
 *   LED      changes colour on every joystick press
 *
 * Only integer printf formats are used, so it does not depend on float
 * support in printf.
 */
#include "stm32f30x_conf.h"
#include "30010_io.h"
#include "board.h"
#include "delay.h"
#include "tick.h"
#include "joystick.h"
#include "led.h"
#include "adc.h"
#include "pwm.h"
#include "lcd.h"

#include <stdint.h>
#include <stdio.h>

#define TICK_HZ         100u     /* 10 ms main loop tick                 */
#define DISPLAY_TICKS   10u      /* LCD every 10 ticks = 100 ms          */
#define PRINT_TICKS     100u     /* UART every 100 ticks = 1 s           */

#define PWM_OUT         PWM_PA6_TIM16_CH1
#define PWM_FREQ_HZ     10000u
#define PWM_STEPS       256u
#define STEP_COARSE     16u
#define STEP_FINE       1u

static uint32_t clampAdd(uint32_t v, int32_t d, uint32_t max);

int main(void)
{
    uint32_t ticks = 0;
    uint32_t pulse = PWM_STEPS / 2u;
    uint8_t  colour = LED_RED;
    uint8_t  pressed;
    int      err;

    uart_init(9600);
    delay_init();
    led_init();
    joystick_init();
    adc_init();
    lcd_init();

    err = pwm_init(&PWM_OUT, PWM_FREQ_HZ, PWM_STEPS);
    pwm_set(&PWM_OUT, pulse);

    tick_init(TICK_HZ);

    printf("\n30021 standard project self-test\n");
    printf("HCLK %lu Hz, TIM16 clock %lu Hz, PWM init %d, counter %lu Hz\n",
           (unsigned long)board_hclk_hz(),
           (unsigned long)board_timer_clock_hz(TIM16), err,
           (unsigned long)pwm_counter_hz(&PWM_OUT));
    led_set(colour);

    while (1) {
        if (tick_take() == 0u) {
            continue;
        }
        ticks++;

        /* --- joystick, every tick (debounced by joystick_poll) --------- */
        pressed = joystick_poll();
        if (pressed) {
            if (pressed & JOY_UP)     pulse = clampAdd(pulse,  STEP_COARSE, PWM_STEPS);
            if (pressed & JOY_DOWN)   pulse = clampAdd(pulse, -(int32_t)STEP_COARSE, PWM_STEPS);
            if (pressed & JOY_RIGHT)  pulse = clampAdd(pulse,  STEP_FINE, PWM_STEPS);
            if (pressed & JOY_LEFT)   pulse = clampAdd(pulse, -(int32_t)STEP_FINE, PWM_STEPS);
            if (pressed & JOY_CENTER) pulse = PWM_STEPS / 2u;

            pwm_set(&PWM_OUT, pulse);

            colour = (uint8_t)(colour % 7u + 1u);      /* 1..7, never off */
            led_set(colour);
        }

        /* --- measurements, LCD at 10 Hz, UART at 1 Hz ------------------ */
        if (ticks % DISPLAY_TICKS == 0u) {
            uint32_t vdda  = adc_vdda_mv();
            uint16_t raw1  = adc_read_avg(POT1_ADC_CHANNEL, 4);
            uint16_t raw2  = adc_read_avg(POT2_ADC_CHANNEL, 4);
            uint32_t mv1   = adc_to_mv(raw1, vdda);
            uint32_t mv2   = adc_to_mv(raw2, vdda);
            uint32_t dutyC = pulse * 1000u / PWM_STEPS;   /* 0.1 % units */

            lcd_clear();
            lcd_printf(0, 0, "POT1 %4u %4lumV", (unsigned)raw1, (unsigned long)mv1);
            lcd_printf(0, 1, "POT2 %4u %4lumV", (unsigned)raw2, (unsigned long)mv2);
            lcd_printf(0, 2, "VDDA %4lumV", (unsigned long)vdda);
            lcd_printf(0, 3, "PA6 %3lu/%u %3lu.%lu%%",
                       (unsigned long)pulse, PWM_STEPS,
                       (unsigned long)(dutyC / 10u), (unsigned long)(dutyC % 10u));
            lcd_update();

            if (ticks % PRINT_TICKS == 0u) {
                printf("t=%lus pot1=%u (%lu mV) pot2=%u (%lu mV) vdda=%lu mV duty=%lu.%lu%%\n",
                       (unsigned long)(millis() / 1000u),
                       (unsigned)raw1, (unsigned long)mv1, (unsigned)raw2, (unsigned long)mv2,
                       (unsigned long)vdda,
                       (unsigned long)(dutyC / 10u), (unsigned long)(dutyC % 10u));
            }
        }
    }
}

/* v + d, limited to 0..max. */
static uint32_t clampAdd(uint32_t v, int32_t d, uint32_t max)
{
    int32_t r = (int32_t)v + d;

    if (r < 0)            return 0u;
    if ((uint32_t)r > max) return max;
    return (uint32_t)r;
}
