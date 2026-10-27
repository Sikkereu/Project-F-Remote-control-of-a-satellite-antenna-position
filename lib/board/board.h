/**
 * @file    board.h
 * @brief   Pin map and clock facts for the Nucleo-F302R8 with the mbed
 *          application shield (MBED-016.1). Course 30021.
 *
 * Every driver in lib/ takes its pins from here, so a pin only has to be
 * changed in one place. Sources:
 *   [A3] Assignment 3 text, appendix 1 "mbed Application Shield Pinout"
 *   [DS] STM32F302x8 datasheet, Table 13 (pin definitions) and
 *        Tables 14-15 (alternate functions)
 *   Documentation/ForbiddenPins.png (pins the Nucleo itself uses)
 *
 * Pins the shield or the Nucleo already use:
 *
 *   PA0  pot 1 (left), ADC1_IN1          PB0  joystick down
 *   PA1  pot 2 (right), ADC1_IN2         PB3  SWO (debug), do not use
 *   PA2  USART2 TX (ST-Link VCP)         PB4  RGB LED red
 *   PA3  USART2 RX (ST-Link VCP)         PB5  joystick centre
 *   PA4  joystick up                     PB6  LCD nCS
 *   PA8  LCD A0                          PB8  I2C SCL (MMA7660, LM75B)
 *   PA9  RGB LED blue                    PB9  I2C SDA (MMA7660, LM75B)
 *   PA10 Xbee status (unused socket)     PB10 speaker (TIM2_CH3, AF1)
 *   PA13 SWDIO, PA14 SWCLK               PB13 LCD SCK (also LD2)
 *   PC0  joystick right                  PB14 LCD nRESET
 *   PC1  joystick left                   PB15 LCD MOSI
 *   PC7  RGB LED green                   PC13 blue user button B1
 *
 * Free pins that are useful (checked against the list above):
 *
 *   PA5  TIM2_CH1 (AF1)
 *   PA6  TIM16_CH1 (AF1) or ADC1_IN10     <- used for PWM in assignment 3
 *   PA7  TIM17_CH1 (AF1) or ADC1_IN15
 *   PA12 TIM16_CH1 (AF1)
 *   PB1  ADC1_IN12
 *   PB11 TIM2_CH4 (AF1) or ADC1_IN14
 *   PC2  ADC1_IN8
 *   PC3  ADC1_IN9
 */
#ifndef BOARD_H
#define BOARD_H

#include "stm32f30x_conf.h"
#include <stdint.h>

/* ------------------------------------------------------------ clocks ---- */
/* system_stm32f30x.c: PLL = HSI/2 x 16, APB1 = HCLK/2, APB2 = HCLK/1.
 * A timer on an APB bus whose prescaler is not 1 runs at twice the bus
 * clock, so TIM2 and TIM6 (APB1) get 64 MHz as well.
 * Note: the CMSIS variable SystemCoreClock is initialised to 72 MHz and is
 * never updated by SystemInit(). Use board_hclk_hz() instead. */
#define BOARD_HCLK_HZ           64000000u
#define BOARD_PCLK1_HZ          32000000u
#define BOARD_PCLK2_HZ          64000000u

/* ---------------------------------------------------------- joystick ---- */
/* Common pin to +3V3, 4k7 pull-down per line: pressed = 1. */
#define JOY_UP_PORT             GPIOA
#define JOY_UP_PIN              GPIO_Pin_4
#define JOY_DOWN_PORT           GPIOB
#define JOY_DOWN_PIN            GPIO_Pin_0
#define JOY_LEFT_PORT           GPIOC
#define JOY_LEFT_PIN            GPIO_Pin_1
#define JOY_RIGHT_PORT          GPIOC
#define JOY_RIGHT_PIN           GPIO_Pin_0
#define JOY_CENTER_PORT         GPIOB
#define JOY_CENTER_PIN          GPIO_Pin_5

/* ----------------------------------------------------------- RGB LED ---- */
/* Common anode to +3V3: a pin driven LOW turns its colour on. */
#define LED_RED_PORT            GPIOB
#define LED_RED_PIN             GPIO_Pin_4
#define LED_GREEN_PORT          GPIOC
#define LED_GREEN_PIN           GPIO_Pin_7
#define LED_BLUE_PORT           GPIOA
#define LED_BLUE_PIN            GPIO_Pin_9

/* ---------------------------------------------------- potentiometers ---- */
#define POT1_PORT               GPIOA           /* left pot  */
#define POT1_PIN                GPIO_Pin_0
#define POT1_ADC_CHANNEL        ADC_Channel_1
#define POT2_PORT               GPIOA           /* right pot */
#define POT2_PIN                GPIO_Pin_1
#define POT2_ADC_CHANNEL        ADC_Channel_2

/* ADC1 channels on free pins, see the list at the top. Put the pin in
 * analog mode with adc_pin_init() first. */
#define ADC_CHANNEL_PA6         ADC_Channel_10
#define ADC_CHANNEL_PA7         ADC_Channel_15
#define ADC_CHANNEL_PB1         ADC_Channel_12
#define ADC_CHANNEL_PB11        ADC_Channel_14
#define ADC_CHANNEL_PC2         ADC_Channel_8
#define ADC_CHANNEL_PC3         ADC_Channel_9

/* --------------------------------------------------------------- LCD ---- */
/* Newhaven C12332A1Z, 128x32, on SPI2 (AF5). lcd.c uses these directly. */
#define LCD_SCK_PORT            GPIOB           /* PB13, SPI2_SCK  */
#define LCD_MOSI_PORT           GPIOB           /* PB15, SPI2_MOSI */
#define LCD_NRESET_PORT         GPIOB           /* PB14            */
#define LCD_NCS_PORT            GPIOB           /* PB6             */
#define LCD_A0_PORT             GPIOA           /* PA8             */

/* ----------------------------------------------------------- speaker ---- */
#define SPEAKER_PORT            GPIOB
#define SPEAKER_PIN             GPIO_Pin_10     /* TIM2_CH3, AF1 */

/* --------------------------------------------------------------- I2C ---- */
/* MMA7660 accelerometer (0x98) and LM75B temperature sensor (0x90). */
#define I2C_SCL_PORT            GPIOB
#define I2C_SCL_PIN             GPIO_Pin_8
#define I2C_SDA_PORT            GPIOB
#define I2C_SDA_PIN             GPIO_Pin_9

/* ------------------------------------------------------ clock helpers ---- */
uint32_t board_hclk_hz(void);
uint32_t board_timer_clock_hz(TIM_TypeDef *tim);

#endif /* BOARD_H */
