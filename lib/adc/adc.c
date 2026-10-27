/**
 * @file    adc.c
 * @brief   ADC1 driver. See adc.h.
 *
 * Clock: RCC_ADC12PLLCLK_Div8 gives 64 MHz / 8 = 8 MHz ADC clock, one cycle
 * = 125 ns.
 *
 * Sample time: [DS] Table 67 gives the largest source resistance R_AIN that
 * still settles to 12 bits for each sample time. The table is for a 72 MHz
 * ADC clock, so at 8 MHz every cycle is 9 times longer. 19.5 cycles at
 * 8 MHz = 2.44 us, which is about the 181.5-cycle row at 72 MHz (2.52 us):
 * R_AIN up to 15 kOhm even on the slowest channel type. A 10 kOhm pot has at
 * most 2.5 kOhm source resistance (at mid travel), so it is well inside.
 * The 1.5 cycles used in assignment 2 is 187.5 ns, between the 7.5 and 19.5
 * rows at 72 MHz (0.47-1.5 kOhm on "other" channels): too short for a pot
 * near mid travel. 19.5 cycles also meets the >= 2.2 us VREFINT needs.
 * Conversion time = (19.5 + 12.5) cycles = 4 us.
 */
#include "adc.h"
#include "board.h"
#include "gpio.h"

/**
 * adc_init  ADC1: 12 bit, right aligned, single conversion, software start.
 *           PA0 and PA1 (the two pots) in analog mode, VREFINT enabled.
 *
 * Order from [RM] chapter 15: clock, voltage regulator (T_ADCVREG_STUP =
 * 10 us start-up),
 * calibration with ADEN = 0, then ADEN and wait for ADRDY.
 */
void adc_init(void)
{
    ADC_InitTypeDef adc;

    RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div8);              /* 8 MHz          */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);  /* interface clock */

    gpio_init_analog(POT1_PORT, POT1_PIN | POT2_PIN);    /* both on port A */

    ADC_StructInit(&adc);
    adc.ADC_ContinuousConvMode    = DISABLE;
    adc.ADC_Resolution            = ADC_Resolution_12b;
    adc.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;
    adc.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
    adc.ADC_DataAlign             = ADC_DataAlign_Right;
    adc.ADC_OverrunMode           = DISABLE;
    adc.ADC_AutoInjMode           = DISABLE;
    adc.ADC_NbrOfRegChannel       = 1;
    ADC_Init(ADC1, &adc);

    ADC_VoltageRegulatorCmd(ADC1, ENABLE);
    for (volatile uint32_t i = 0; i < 10000u; i++) { }   /* >= 10 us */

    /* ADCAL needs ADEN = 0. Only disable if it is on: setting ADDIS with
     * the ADC already off hangs the wait below (seen in assignment 2). */
    if (ADC1->CR & ADC_CR_ADEN) {
        ADC_Cmd(ADC1, DISABLE);
        while (ADC_GetDisableCmdStatus(ADC1)) { }
    }
    ADC_SelectCalibrationMode(ADC1, ADC_CalibrationMode_Single);
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1)) { }
    for (volatile uint32_t i = 0; i < 100u; i++) { }     /* ADEN not at once */

    ADC_VrefintCmd(ADC1, ENABLE);                        /* before ADEN     */
    for (volatile uint32_t i = 0; i < 10000u; i++) { }

    ADC_Cmd(ADC1, ENABLE);
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY)) { }
}

/**
 * adc_pin_init  Puts extra pins in analog mode for adc_read().
 *               Channel numbers for the free pins are in board.h.
 */
void adc_pin_init(GPIO_TypeDef *port, uint16_t pins)
{
    gpio_init_analog(port, pins);
}

/**
 * adc_read_st  One conversion with an explicit sample time.
 * @param channel     ADC_Channel_1 .. ADC_Channel_18
 * @param sampleTime  ADC_SampleTime_1Cycles5 .. ADC_SampleTime_601Cycles5
 * @return            0 .. 4095
 */
uint16_t adc_read_st(uint8_t channel, uint8_t sampleTime)
{
    ADC_RegularChannelConfig(ADC1, channel, 1, sampleTime);
    ADC_StartConversion(ADC1);
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET) { }
    return ADC_GetConversionValue(ADC1);     /* reading DR clears EOC */
}

/**
 * adc_read  One conversion with ADC_DEFAULT_SAMPLE_TIME.
 */
uint16_t adc_read(uint8_t channel)
{
    return adc_read_st(channel, ADC_DEFAULT_SAMPLE_TIME);
}

/**
 * adc_read_avg  Mean of `n` conversions (n = 0 is treated as 1).
 *               n up to 65535 cannot overflow the 32-bit sum.
 */
uint16_t adc_read_avg(uint8_t channel, uint16_t n)
{
    uint32_t sum = 0;
    uint16_t i;

    if (n == 0u) {
        n = 1u;
    }
    for (i = 0; i < n; i++) {
        sum += adc_read(channel);
    }
    return (uint16_t)((sum + n / 2u) / n);   /* rounded */
}

/**
 * adc_read_vrefint  One conversion of the internal reference, channel 18.
 */
uint16_t adc_read_vrefint(void)
{
    return adc_read(ADC_CHANNEL_VREFINT);
}

/**
 * adc_vdda  VDDA in volts, from VREFINT (exercise 2.5):
 *           VDDA = 3.3 V * VREFINT_CAL / VREFINT_DATA
 */
float adc_vdda(void)
{
    uint16_t data = adc_read_avg(ADC_CHANNEL_VREFINT, 8);

    return ((float)VREFINT_CAL_VDDA_MV / 1000.0f) * (float)VREFINT_CAL / (float)data;
}

/**
 * adc_to_volt  raw * VDDA / 4095, in volts.
 */
float adc_to_volt(uint16_t raw, float vdda)
{
    return vdda * (float)raw / (float)ADC_FULL_SCALE;
}

/**
 * adc_vdda_mv  As adc_vdda(), in millivolts, integer only.
 */
uint32_t adc_vdda_mv(void)
{
    uint16_t data = adc_read_avg(ADC_CHANNEL_VREFINT, 8);

    return (VREFINT_CAL_VDDA_MV * (uint32_t)VREFINT_CAL + data / 2u) / data;
}

/**
 * adc_to_mv  raw * VDDA / 4095, in millivolts, rounded. Integer only.
 */
uint32_t adc_to_mv(uint16_t raw, uint32_t vddaMv)
{
    return ((uint32_t)raw * vddaMv + ADC_FULL_SCALE / 2u) / ADC_FULL_SCALE;
}
