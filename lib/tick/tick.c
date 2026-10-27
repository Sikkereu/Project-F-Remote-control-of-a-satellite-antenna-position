/**
 * @file    tick.c
 * @brief   TIM6 periodic interrupt. See tick.h.
 *
 * Prescaler and reload
 * --------------------
 * The timer runs from f_TIM = 64 MHz (APB1 x 2, see board.h). One update
 * event every (PSC + 1) * (ARR + 1) input clocks, so for a tick rate f_tick
 *
 *     (PSC + 1) * (ARR + 1) = f_TIM / f_tick
 *
 * PSC and ARR are both 16 bit on TIM6. tick_init() takes the smallest
 * PSC + 1 that divides the product exactly with ARR + 1 <= 65536, so the
 * rate is exact (no rounding) and the counter resolution is as fine as
 * possible. Examples at 64 MHz:
 *
 *     100 Hz: 640000  = 10 * 64000    -> PSC = 9,   ARR = 63999
 *      10 Hz: 6400000 = 100 * 64000   -> PSC = 99,  ARR = 63999
 *       1 Hz: 64e6    = 1000 * 64000  -> PSC = 999, ARR = 63999
 *
 * A rate that does not divide 64 MHz (for example 7 Hz) is refused with -1
 * rather than silently rounded.
 */
#include "tick.h"
#include "board.h"

#define TICK_IRQ_PRIORITY  2u    /* 0 = highest, 15 = lowest */

static volatile uint32_t tickCount   = 0;   /* ticks since tick_init() */
static volatile uint32_t tickPending = 0;   /* ticks not yet taken     */
static uint32_t tickRate = 0;

/**
 * tick_init  Starts TIM6 with an update interrupt at `hz`.
 * @param hz  tick rate in Hz, 1 .. f_TIM.
 * @return    0 on success, -1 if the rate cannot be made exactly.
 */
int tick_init(uint32_t hz)
{
    TIM_TimeBaseInitTypeDef tb;
    uint32_t fTim, total, div;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
    fTim = board_timer_clock_hz(TIM6);

    if (hz == 0u || fTim % hz != 0u) {
        return -1;
    }
    total = fTim / hz;                             /* (PSC+1) * (ARR+1)  */

    for (div = (total + 65535u) / 65536u; div <= 65536u; div++) {
        if (div != 0u && total % div == 0u && total / div <= 65536u) {
            break;
        }
    }
    if (div > 65536u) {
        return -1;
    }

    TIM_Cmd(TIM6, DISABLE);
    TIM_TimeBaseStructInit(&tb);
    tb.TIM_Prescaler   = (uint16_t)(div - 1u);
    tb.TIM_Period      = total / div - 1u;
    tb.TIM_CounterMode = TIM_CounterMode_Up;        /* TIM6 only counts up */
    TIM_TimeBaseInit(TIM6, &tb);   /* sets UG, which also sets UIF ...     */
    TIM_ClearFlag(TIM6, TIM_FLAG_Update);  /* ... so clear it, or the first
                                            * tick comes immediately        */

    tickCount   = 0;
    tickPending = 0;
    tickRate    = hz;

    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
    NVIC_SetPriority(TIM6_DAC_IRQn, TICK_IRQ_PRIORITY);
    NVIC_EnableIRQ(TIM6_DAC_IRQn);
    TIM_Cmd(TIM6, ENABLE);

    return 0;
}

/**
 * tick_take  Number of ticks since the last call (usually 0 or 1), then
 *            clears it. More than 1 means the main loop fell behind.
 */
uint32_t tick_take(void)
{
    uint32_t n;
    uint32_t primask = __get_PRIMASK();

    __disable_irq();
    n = tickPending;
    tickPending = 0;
    __set_PRIMASK(primask);

    return n;
}

/**
 * tick_count  Ticks since tick_init(). Wraps after 2^32 ticks
 *             (497 days at 100 Hz); use unsigned subtraction for intervals.
 */
uint32_t tick_count(void)
{
    return tickCount;          /* 32-bit aligned read is atomic on M4 */
}

uint32_t tick_hz(void)
{
    return tickRate;
}

/**
 * tick_wait  Blocks until the next tick. Do not call from an interrupt.
 */
void tick_wait(void)
{
    uint32_t start = tickCount;

    while (tickCount == start) { }
    (void)tick_take();
}

/**
 * TIM6_DACUNDER_IRQHandler  TIM6 update interrupt.
 *
 * The name comes from the vector table in src/vendor/startup_stm32f302r8tx.s
 * (CMSIS calls the IRQ number TIM6_DAC_IRQn, but the startup file names the
 * handler TIM6_DACUNDER_IRQHandler). A misspelt name compiles fine and the
 * default handler, an endless loop, runs instead.
 */
void TIM6_DACUNDER_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) {
        tickCount++;
        tickPending++;
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
    }
}
