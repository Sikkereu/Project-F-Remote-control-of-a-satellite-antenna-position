/**
 * @file    pwm.c
 * @brief   Timer PWM output. See pwm.h for usage and the arithmetic.
 *
 * Set-up steps are the ones from the assignment 3 text and [RM] 21.3.9
 * (TIM2) / 22.4.10 (TIM15/16/17):
 *   1. timer clock on (RCC_APBx)
 *   2. time base: PSC, ARR, up-counting, ARR preload
 *   3. output compare channel in PWM mode 1, CCR preload on (mandatory)
 *   4. MOE = 1 for timers with a break unit (TIM1/15/16/17), not for TIM2
 *   5. counter on
 *   6. pin to alternate function, last, so it never outputs garbage
 */
#include "pwm.h"
#include "board.h"
#include "gpio.h"

/* ---------------------------------------------------- pin descriptors --- */
const pwm_pin_t PWM_PA6_TIM16_CH1          = { TIM16, 1, GPIOA, GPIO_Pin_6,  GPIO_AF_1 };
const pwm_pin_t PWM_PA12_TIM16_CH1         = { TIM16, 1, GPIOA, GPIO_Pin_12, GPIO_AF_1 };
const pwm_pin_t PWM_PA7_TIM17_CH1          = { TIM17, 1, GPIOA, GPIO_Pin_7,  GPIO_AF_1 };
const pwm_pin_t PWM_PA5_TIM2_CH1           = { TIM2,  1, GPIOA, GPIO_Pin_5,  GPIO_AF_1 };
const pwm_pin_t PWM_PB11_TIM2_CH4          = { TIM2,  4, GPIOB, GPIO_Pin_11, GPIO_AF_1 };
const pwm_pin_t PWM_PB10_TIM2_CH3_SPEAKER  = { TIM2,  3, GPIOB, GPIO_Pin_10, GPIO_AF_1 };
const pwm_pin_t PWM_PB4_TIM16_CH1_LED_RED  = { TIM16, 1, GPIOB, GPIO_Pin_4,  GPIO_AF_1 };

/* ------------------------------------------------------ timer state ----- */
/* PSC and ARR belong to the timer, not the channel, so they are kept per
 * timer: a second channel on a running timer must match them. */
typedef struct {
    TIM_TypeDef *tim;
    uint32_t     freqHz;
    uint32_t     steps;
    uint32_t     counterHz;
    uint8_t      running;
} pwm_timer_t;

static pwm_timer_t pwmTimers[] = {
    { TIM1,  0, 0, 0, 0 },
    { TIM2,  0, 0, 0, 0 },
    { TIM15, 0, 0, 0, 0 },
    { TIM16, 0, 0, 0, 0 },
    { TIM17, 0, 0, 0, 0 },
};
#define PWM_NTIMERS (sizeof pwmTimers / sizeof pwmTimers[0])

static pwm_timer_t *timer_state(TIM_TypeDef *tim)
{
    for (uint32_t i = 0; i < PWM_NTIMERS; i++) {
        if (pwmTimers[i].tim == tim) {
            return &pwmTimers[i];
        }
    }
    return 0;
}

static uint8_t max_channel(TIM_TypeDef *tim)
{
    if (tim == TIM1 || tim == TIM2) return 4;
    if (tim == TIM15)               return 2;
    if (tim == TIM16 || tim == TIM17) return 1;
    return 0;
}

static uint8_t has_break_unit(TIM_TypeDef *tim)
{
    return (tim == TIM1 || tim == TIM15 || tim == TIM16 || tim == TIM17);
}

static void timer_clock_enable(TIM_TypeDef *tim)
{
    if      (tim == TIM1)  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,  ENABLE);
    else if (tim == TIM2)  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,  ENABLE);
    else if (tim == TIM15) RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE);
    else if (tim == TIM16) RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);
    else if (tim == TIM17) RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, ENABLE);
}

static void set_compare(TIM_TypeDef *tim, uint8_t ch, uint32_t v)
{
    switch (ch) {
    case 1: TIM_SetCompare1(tim, v); break;
    case 2: TIM_SetCompare2(tim, v); break;
    case 3: TIM_SetCompare3(tim, v); break;
    case 4: TIM_SetCompare4(tim, v); break;
    default: break;
    }
}

/* PWM mode 1, up-counting: output high while CNT < CCRx, low after the
 * match, high again at the reload. Compare starts at 0 (output low). */
static void channel_init(TIM_TypeDef *tim, uint8_t ch)
{
    TIM_OCInitTypeDef oc;

    TIM_OCStructInit(&oc);
    oc.TIM_OCMode       = TIM_OCMode_PWM1;
    oc.TIM_OutputState  = TIM_OutputState_Enable;     /* CCxE = 1          */
    oc.TIM_OutputNState = TIM_OutputNState_Disable;   /* no complementary  */
    oc.TIM_Pulse        = 0;
    oc.TIM_OCPolarity   = TIM_OCPolarity_High;
    oc.TIM_OCIdleState  = TIM_OCIdleState_Reset;      /* low when MOE = 0  */

    /* OCxPE: a new CCR value waits in the shadow register until the next
     * update event, so a duty change never cuts a period short. */
    switch (ch) {
    case 1: TIM_OC1Init(tim, &oc); TIM_OC1PreloadConfig(tim, TIM_OCPreload_Enable); break;
    case 2: TIM_OC2Init(tim, &oc); TIM_OC2PreloadConfig(tim, TIM_OCPreload_Enable); break;
    case 3: TIM_OC3Init(tim, &oc); TIM_OC3PreloadConfig(tim, TIM_OCPreload_Enable); break;
    case 4: TIM_OC4Init(tim, &oc); TIM_OC4PreloadConfig(tim, TIM_OCPreload_Enable); break;
    default: break;
    }
}

/**
 * pwm_init  Starts PWM on one pin, output low (compare 0) until pwm_set().
 * @param p       pin descriptor, e.g. &PWM_PA6_TIM16_CH1
 * @param freqHz  PWM frequency
 * @param steps   duty resolution; the period is `steps` counter ticks
 * @return        PWM_OK or a PWM_ERR_* code (nothing is changed on error)
 */
int pwm_init(const pwm_pin_t *p, uint32_t freqHz, uint32_t steps)
{
    pwm_timer_t *t;
    uint32_t fTim, fCnt, psc;
    uint64_t prod;

    if (p == 0 || freqHz == 0u || steps == 0u) {
        return PWM_ERR_ARG;
    }
    t = timer_state(p->tim);
    if (t == 0 || p->channel == 0u || p->channel > max_channel(p->tim)) {
        return PWM_ERR_ARG;
    }

    if (t->running) {
        if (t->freqHz != freqHz || t->steps != steps) {
            return PWM_ERR_SHARED;
        }
    } else {
        TIM_TimeBaseInitTypeDef tb;

        fTim = board_timer_clock_hz(p->tim);
        prod = (uint64_t)freqHz * steps;
        if (prod == 0u || prod > fTim) {
            return PWM_ERR_RANGE;
        }
        fCnt = (uint32_t)prod;
        if (fTim % fCnt != 0u) {
            return PWM_ERR_INEXACT;
        }
        psc = fTim / fCnt - 1u;
        if (psc > 0xFFFFu) {
            return PWM_ERR_RANGE;
        }
        if (p->tim != TIM2 && steps > 0x10000u) {    /* 16-bit ARR */
            return PWM_ERR_RANGE;
        }

        timer_clock_enable(p->tim);
        TIM_Cmd(p->tim, DISABLE);

        TIM_TimeBaseStructInit(&tb);
        tb.TIM_Prescaler         = (uint16_t)psc;
        tb.TIM_Period            = steps - 1u;
        tb.TIM_CounterMode       = TIM_CounterMode_Up;
        tb.TIM_ClockDivision     = TIM_CKD_DIV1;
        tb.TIM_RepetitionCounter = 0;          /* update every period     */
        TIM_TimeBaseInit(p->tim, &tb);         /* UG loads PSC/ARR now    */
        TIM_ARRPreloadConfig(p->tim, ENABLE);

        t->freqHz    = freqHz;
        t->steps     = steps;
        t->counterHz = fCnt;
    }

    channel_init(p->tim, p->channel);

    if (has_break_unit(p->tim)) {
        TIM_CtrlPWMOutputs(p->tim, ENABLE);    /* BDTR.MOE */
    }
    if (!t->running) {
        TIM_Cmd(p->tim, ENABLE);
        t->running = 1;
    }

    gpio_init_af(p->port, p->pin, p->af, GPIO_PuPd_DOWN);  /* low until driven */

    return PWM_OK;
}

/**
 * pwm_set  New compare value, 0 .. steps (clamped). Takes effect at the
 *          start of the next period.
 */
void pwm_set(const pwm_pin_t *p, uint32_t compare)
{
    pwm_timer_t *t = timer_state(p->tim);

    if (t == 0 || !t->running) {
        return;
    }
    if (compare > t->steps) {
        compare = t->steps;
    }
    set_compare(p->tim, p->channel, compare);
}

/**
 * pwm_get  The compare value last written (the one in effect from the next
 *          period on).
 */
uint32_t pwm_get(const pwm_pin_t *p)
{
    switch (p->channel) {
    case 1: return TIM_GetCapture1(p->tim);
    case 2: return TIM_GetCapture2(p->tim);
    case 3: return TIM_GetCapture3(p->tim);
    case 4: return TIM_GetCapture4(p->tim);
    default: return 0u;
    }
}

/**
 * pwm_set_permille  Duty in 0.1 % units, 0 .. 1000, rounded to the nearest
 *                   step.
 */
void pwm_set_permille(const pwm_pin_t *p, uint32_t permille)
{
    pwm_timer_t *t = timer_state(p->tim);

    if (t == 0) {
        return;
    }
    if (permille > 1000u) {
        permille = 1000u;
    }
    pwm_set(p, (uint32_t)(((uint64_t)permille * t->steps + 500u) / 1000u));
}

/**
 * pwm_set_us  High time in microseconds (servo pulses: 1000-2000 us).
 *             Resolution is one counter tick, 1 / pwm_counter_hz().
 */
void pwm_set_us(const pwm_pin_t *p, uint32_t us)
{
    pwm_timer_t *t = timer_state(p->tim);

    if (t == 0) {
        return;
    }
    pwm_set(p, (uint32_t)(((uint64_t)us * t->counterHz + 500000u) / 1000000u));
}

uint32_t pwm_steps(const pwm_pin_t *p)
{
    pwm_timer_t *t = timer_state(p->tim);
    return t ? t->steps : 0u;
}

uint32_t pwm_counter_hz(const pwm_pin_t *p)
{
    pwm_timer_t *t = timer_state(p->tim);
    return t ? t->counterHz : 0u;
}

/**
 * pwm_wait_periods  Busy-waits whole PWM periods by polling the update flag
 *                   (from assignment 3.1). The first period may be partial.
 */
void pwm_wait_periods(const pwm_pin_t *p, uint32_t periods)
{
    TIM_ClearFlag(p->tim, TIM_FLAG_Update);

    while (periods--) {
        while (TIM_GetFlagStatus(p->tim, TIM_FLAG_Update) == RESET) { }
        TIM_ClearFlag(p->tim, TIM_FLAG_Update);
    }
}
