/**
 * @file    board.c
 * @brief   Clock read-back helpers. The pin map is in board.h.
 */
#include "board.h"

/**
 * board_hclk_hz  Core/AHB clock, read from the RCC registers at run time.
 */
uint32_t board_hclk_hz(void)
{
    RCC_ClocksTypeDef clocks;

    RCC_GetClocksFreq(&clocks);
    return clocks.HCLK_Frequency;
}

/**
 * board_timer_clock_hz  The clock that actually reaches a timer's prescaler.
 * @param tim  TIM1, TIM2, TIM6, TIM15, TIM16 or TIM17.
 * @return     frequency in Hz, 0 for an unknown timer.
 *
 * [RM] clock tree: a timer is clocked from its APB clock, doubled when the
 * APB prescaler is not 1. With the project's system_stm32f30x.c both cases
 * give 64 MHz, but reading it back keeps the drivers right if the clock
 * setup is ever changed. TIM1 can also be clocked from the PLL; the SPL
 * already works that out in TIM1CLK_Frequency.
 */
uint32_t board_timer_clock_hz(TIM_TypeDef *tim)
{
    RCC_ClocksTypeDef clocks;

    RCC_GetClocksFreq(&clocks);

    if (tim == TIM1) {
        return clocks.TIM1CLK_Frequency;
    }

    if (tim == TIM2 || tim == TIM6) {                               /* APB1 */
        if ((RCC->CFGR & RCC_CFGR_PPRE1) == RCC_CFGR_PPRE1_DIV1) {
            return clocks.PCLK1_Frequency;
        }
        return 2u * clocks.PCLK1_Frequency;
    }

    if (tim == TIM15 || tim == TIM16 || tim == TIM17) {             /* APB2 */
        if ((RCC->CFGR & RCC_CFGR_PPRE2) == RCC_CFGR_PPRE2_DIV1) {
            return clocks.PCLK2_Frequency;
        }
        return 2u * clocks.PCLK2_Frequency;
    }

    return 0u;
}
