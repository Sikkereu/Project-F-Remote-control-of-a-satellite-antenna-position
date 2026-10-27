/**
 * @file    adc.h
 * @brief   ADC1, single software-triggered 12-bit conversions, plus absolute
 *          voltage via VREFINT. From assignment 2 (exercises 2.4-2.6) and
 *          reused for the buck converter feedback in assignment 3.
 *
 *     adc_init();                               // PA0, PA1 and VREFINT
 *     uint16_t raw  = adc_read(POT1_ADC_CHANNEL);
 *     float    vdda = adc_vdda();               // measure once in a while
 *     float    v    = adc_to_volt(raw, vdda);
 *
 * Another input (e.g. the buck converter output on PB1):
 *
 *     adc_pin_init(GPIOB, GPIO_Pin_1);
 *     uint16_t raw = adc_read_avg(ADC_CHANNEL_PB1, 16);
 *
 * Integer-only versions (adc_vdda_mv(), adc_to_mv()) avoid float and
 * printf("%f") altogether.
 */
#ifndef ADC_H
#define ADC_H

#include "stm32f30x_conf.h"
#include <stdint.h>

#define ADC_FULL_SCALE          4095u          /* 2^12 - 1 */

/* Factory VREFINT calibration value, measured at VDDA = 3.3 V, 30 degC.
 * [DS] Table 4, address 0x1FFFF7BA. */
#define VREFINT_CAL             (*((volatile uint16_t *)0x1FFFF7BAu))
#define VREFINT_CAL_VDDA_MV     3300u

#define ADC_CHANNEL_VREFINT     ADC_Channel_18

/* Sample time used by adc_read(). See adc.c for why 19.5 cycles. */
#ifndef ADC_DEFAULT_SAMPLE_TIME
#define ADC_DEFAULT_SAMPLE_TIME ADC_SampleTime_19Cycles5
#endif

void     adc_init(void);
void     adc_pin_init(GPIO_TypeDef *port, uint16_t pins);

uint16_t adc_read(uint8_t channel);
uint16_t adc_read_st(uint8_t channel, uint8_t sampleTime);
uint16_t adc_read_avg(uint8_t channel, uint16_t n);
uint16_t adc_read_vrefint(void);

float    adc_vdda(void);
float    adc_to_volt(uint16_t raw, float vdda);

uint32_t adc_vdda_mv(void);
uint32_t adc_to_mv(uint16_t raw, uint32_t vddaMv);

#endif /* ADC_H */
