/**
 * @file    delay.c
 * @brief   SysTick millisecond counter and DWT-based microsecond delay.
 *
 * SysTick is the 24-bit down-counter built into the Cortex-M4 core
 * ([PM] 4.5). Loaded with HCLK / 1000 - 1 it underflows, and interrupts,
 * once per millisecond.
 *
 * The DWT (data watchpoint and trace) unit has a free-running 32-bit cycle
 * counter, CYCCNT, that counts HCLK cycles once enabled. It is used for
 * microsecond delays because a 1 us SysTick interrupt would eat the CPU.
 * The CMSIS 2.10 core_cm4.h in include/vendor has no DWT struct, so the two
 * registers are addressed directly (addresses from the ARMv7-M
 * Architecture Reference Manual, DWT section).
 */
#include "delay.h"
#include "board.h"

#define DWT_CTRL      (*(volatile uint32_t *)0xE0001000u)
#define DWT_CYCCNT    (*(volatile uint32_t *)0xE0001004u)
#define DWT_CYCCNTENA (1u << 0)

static volatile uint32_t msCount = 0;
static uint32_t cyclesPerUs = 64u;

/**
 * delay_init  Starts SysTick at 1 kHz and the DWT cycle counter.
 *
 * HCLK is read back from the RCC rather than taken from SystemCoreClock,
 * which the course's system_stm32f30x.c leaves at its initial 72 MHz.
 */
void delay_init(void)
{
    uint32_t hclk = board_hclk_hz();

    cyclesPerUs = hclk / 1000000u;

    SysTick_Config(hclk / 1000u);        /* also sets lowest priority (15) */

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;   /* enable DWT       */
    DWT_CYCCNT = 0;
    DWT_CTRL  |= DWT_CYCCNTENA;
}

/**
 * millis  Milliseconds since delay_init(). Wraps after 49.7 days.
 */
uint32_t millis(void)
{
    return msCount;
}

/**
 * delay_ms  Waits at least `ms` milliseconds.
 *
 * The first millisecond boundary can come right after the start, so one
 * extra tick is waited to make the delay never shorter than asked.
 */
void delay_ms(uint32_t ms)
{
    uint32_t start = msCount;

    while ((msCount - start) < ms + 1u) { }
}

/**
 * delay_us  Busy-waits `us` microseconds on the cycle counter.
 *           Accurate to a few cycles; up to 67 s at 64 MHz.
 */
void delay_us(uint32_t us)
{
    uint32_t start  = DWT_CYCCNT;
    uint32_t cycles = us * cyclesPerUs;

    while ((DWT_CYCCNT - start) < cycles) { }
}

/**
 * SysTick_Handler  Name fixed by the vector table in the startup file.
 */
void SysTick_Handler(void)
{
    msCount++;
}
