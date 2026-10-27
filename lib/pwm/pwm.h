/**
 * @file    pwm.h
 * @brief   PWM output on any timer channel, generalised from the TIM16/PA6
 *          driver in assignment 3.1.
 *
 * A PWM output is described by a pwm_pin_t: timer, channel, pin and AF
 * number. The ones that are free on the shield are predefined below (AF
 * numbers from [DS] Tables 14-15). Frequency and resolution are chosen at
 * run time:
 *
 *     // 3.1: 10 kHz, 256 steps on PA6, 50 %
 *     pwm_init(&PWM_PA6_TIM16_CH1, 10000, 256);
 *     pwm_set(&PWM_PA6_TIM16_CH1, 128);
 *
 *     // 3.3: two servos, 50 Hz, 1 us resolution (20000 steps)
 *     pwm_init(&PWM_PA6_TIM16_CH1, 50, 20000);
 *     pwm_init(&PWM_PA7_TIM17_CH1, 50, 20000);
 *     pwm_set_us(&PWM_PA6_TIM16_CH1, 1500);          // centre
 *
 *     // speaker tone, 1 kHz, 50 %
 *     pwm_init(&PWM_PB10_TIM2_CH3_SPEAKER, 1000, 1000);
 *     pwm_set_permille(&PWM_PB10_TIM2_CH3_SPEAKER, 500);
 *
 * Duty = compare / steps. compare = 0 is constant low, compare = steps is
 * constant high (100 %).
 *
 * Timer arithmetic (checked in pwm_init):
 *     f_counter = f_PWM * steps            e.g. 10 kHz * 256 = 2.56 MHz
 *     PSC       = f_TIM / f_counter - 1    must be a whole number, <= 65535
 *     ARR       = steps - 1                <= 65535 (TIM2: 32 bit)
 *
 * Channels of the same timer share PSC and ARR, so they must use the same
 * frequency and steps. TIM16 and TIM17 are separate timers, which is why
 * the two servos go on PA6 and PA7.
 */
#ifndef PWM_H
#define PWM_H

#include "stm32f30x_conf.h"
#include <stdint.h>

typedef struct {
    TIM_TypeDef  *tim;
    uint8_t       channel;   /* 1..4 */
    GPIO_TypeDef *port;
    uint16_t      pin;       /* GPIO_Pin_x, one pin */
    uint8_t       af;        /* GPIO_AF_x          */
} pwm_pin_t;

/* Free pins (see board.h). */
extern const pwm_pin_t PWM_PA6_TIM16_CH1;          /* assignment 3.1/3.2 */
extern const pwm_pin_t PWM_PA12_TIM16_CH1;         /* same timer as PA6  */
extern const pwm_pin_t PWM_PA7_TIM17_CH1;
extern const pwm_pin_t PWM_PA5_TIM2_CH1;
extern const pwm_pin_t PWM_PB11_TIM2_CH4;
/* On the shield. */
extern const pwm_pin_t PWM_PB10_TIM2_CH3_SPEAKER;
extern const pwm_pin_t PWM_PB4_TIM16_CH1_LED_RED;  /* shares TIM16 with PA6; LED is
                                                   * active low, so brightness
                                                   * = 100 % - duty           */

/* Return codes */
#define PWM_OK            0
#define PWM_ERR_ARG      -1   /* bad timer/channel, freq or steps 0      */
#define PWM_ERR_INEXACT  -2   /* f_TIM is not a multiple of f_PWM*steps  */
#define PWM_ERR_RANGE    -3   /* PSC or ARR does not fit                 */
#define PWM_ERR_SHARED   -4   /* timer already running at another f/steps */

int      pwm_init(const pwm_pin_t *p, uint32_t freqHz, uint32_t steps);
void     pwm_set(const pwm_pin_t *p, uint32_t compare);
uint32_t pwm_get(const pwm_pin_t *p);
void     pwm_set_permille(const pwm_pin_t *p, uint32_t permille);
void     pwm_set_us(const pwm_pin_t *p, uint32_t us);
uint32_t pwm_steps(const pwm_pin_t *p);
uint32_t pwm_counter_hz(const pwm_pin_t *p);
void     pwm_wait_periods(const pwm_pin_t *p, uint32_t periods);

#endif /* PWM_H */
