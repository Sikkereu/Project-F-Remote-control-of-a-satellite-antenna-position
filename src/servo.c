#include "stm32f30x_conf.h"
#include "servo.h"

#define SERVO_MIN_US    500 // pulse at 0 deg - to be calibrated
#define SERVO_MAX_US    2500 // pulse at 180 deg - to be calibrated
#define SERVO_CENTER_US 1500 // pulse at 90 deg
#define SERVO_FRAME_US  20000 // 50 Hz frame

// TIM16/TIM17 sit on APB2 = 64 MHz here; /64 gives 1 tick = 1 us - compare value = puls width in us
#define SERVO_TIMER_HZ 64000000u
#define SERVO_TICK_HZ 1000000u

// Same setup for both timers: 50 Hz, channel 1 PWM
static void servo_timer_start(TIM_TypeDef *tim)
{
    TIM_TimeBaseInitTypeDef base;
    TIM_TimeBaseStructInit(&base);
    base.TIM_Prescaler = (SERVO_TIMER_HZ / SERVO_TICK_HZ) - 1; // 1 us per tick
    base.TIM_Period = SERVO_FRAME_US - 1; // 20 ms
    TIM_TimeBaseInit(tim, &base);

    TIM_OCInitTypeDef oc;
    TIM_OCStructInit(&oc);
    oc.TIM_OCMode = TIM_OCMode_PWM1; // high until compare match
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_Pulse = SERVO_CENTER_US;
    TIM_OC1Init(tim, &oc);

    TIM_OC1PreloadConfig(tim, TIM_OCPreload_Enable); // new pulse applies at frame end
    TIM_ARRPreloadConfig(tim, ENABLE);
    TIM_CtrlPWMOutputs(tim, ENABLE); // TIM16/17 need MOE set
    TIM_Cmd(tim, ENABLE);
}

void servo_init(void)
{
    // Pins: PA6 and PA7 driven by the timers (AF1)
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_Level_2;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &gpio);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_1); // TIM16_CH1
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_1); // TIM17_CH1

    // Timers: one per servo
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16 | RCC_APB2Periph_TIM17, ENABLE);
    servo_timer_start(TIM16); // servo 1
    servo_timer_start(TIM17); // servo 2
}

void servo_set_pulse_us(servo_id_t servo, uint16_t us)
{
    if (us < SERVO_MIN_US) us = SERVO_MIN_US; // stay inside the safe range
    if (us > SERVO_MAX_US) us = SERVO_MAX_US;

    if (servo == SERVO_1) TIM_SetCompare1(TIM16, us);
    else TIM_SetCompare1(TIM17, us);
}

void servo_set_angle(servo_id_t servo, uint8_t deg)
{
    if (deg > 180) deg = 180;
    servo_set_pulse_us(servo, SERVO_MIN_US + (deg * (SERVO_MAX_US - SERVO_MIN_US)) / 180);
}
