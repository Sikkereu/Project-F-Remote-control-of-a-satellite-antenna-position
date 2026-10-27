/**
 * @file    joystick.c
 * @brief   Joystick driver. See joystick.h for wiring and usage.
 */
#include "joystick.h"
#include "board.h"
#include "gpio.h"

static uint8_t joyLastRaw = 0;   /* previous raw sample, for debouncing   */
static uint8_t joyStable  = 0;   /* last debounced state                  */

/**
 * joystick_init  Puts the five joystick pins in input mode.
 *
 * The internal pull-down pulls the same way as the shield's 4k7, so it
 * changes nothing with the shield fitted, but it keeps the pins from
 * floating (and toggling randomly) with the shield unplugged.
 */
void joystick_init(void)
{
    gpio_init_input(JOY_UP_PORT,     JOY_UP_PIN,     GPIO_PuPd_DOWN);
    gpio_init_input(JOY_DOWN_PORT,   JOY_DOWN_PIN,   GPIO_PuPd_DOWN);
    gpio_init_input(JOY_LEFT_PORT,   JOY_LEFT_PIN,   GPIO_PuPd_DOWN);
    gpio_init_input(JOY_RIGHT_PORT,  JOY_RIGHT_PIN,  GPIO_PuPd_DOWN);
    gpio_init_input(JOY_CENTER_PORT, JOY_CENTER_PIN, GPIO_PuPd_DOWN);

    joyLastRaw = joystick_read();
    joyStable  = joyLastRaw;
}

/**
 * joystick_read  All five switches right now, no debouncing.
 * @return bit 0 up, 1 down, 2 left, 3 right, 4 centre. 1 = pressed.
 */
uint8_t joystick_read(void)
{
    uint8_t s = 0;

    if (gpio_read(JOY_UP_PORT,     JOY_UP_PIN))     s |= JOY_UP;
    if (gpio_read(JOY_DOWN_PORT,   JOY_DOWN_PIN))   s |= JOY_DOWN;
    if (gpio_read(JOY_LEFT_PORT,   JOY_LEFT_PIN))   s |= JOY_LEFT;
    if (gpio_read(JOY_RIGHT_PORT,  JOY_RIGHT_PIN))  s |= JOY_RIGHT;
    if (gpio_read(JOY_CENTER_PORT, JOY_CENTER_PIN)) s |= JOY_CENTER;

    return s;
}

/**
 * joystick_poll  Debounced press detection. Call at a fixed rate, 5-20 ms.
 * @return the switches that went from released to pressed in this call.
 *
 * A new state is only accepted when two samples in a row agree, so a
 * change has to last at least one poll period. Contact bounce is over in a
 * few ms, so at a 10 ms poll rate one press gives exactly one event.
 * "raw & ~stable" keeps only the 0 -> 1 edges: holding a switch down
 * reports it once, releasing it reports nothing.
 */
uint8_t joystick_poll(void)
{
    uint8_t raw     = joystick_read();
    uint8_t pressed = 0;

    if (raw == joyLastRaw) {
        pressed   = (uint8_t)(raw & (uint8_t)~joyStable);
        joyStable = raw;
    }
    joyLastRaw = raw;

    return pressed;
}

/**
 * joystick_state  Last debounced state from joystick_poll(), 1 = held.
 */
uint8_t joystick_state(void)
{
    return joyStable;
}

/* ================================================================ EXTI === */
#ifndef JOYSTICK_NO_IRQ_HANDLERS

static volatile uint8_t joyIrqEvents = 0;  /* set by the ISRs, cleared by take */
static uint8_t joyLine0Owner = 0;          /* JOY_DOWN or JOY_RIGHT            */

/* One EXTI line: rising edge, interrupt mode ([RM] 13.2). */
static void exti_line_enable(uint8_t portSource, uint8_t pinSource,
                             uint32_t line, IRQn_Type irq, uint8_t priority)
{
    EXTI_InitTypeDef e;

    SYSCFG_EXTILineConfig(portSource, pinSource);

    EXTI_StructInit(&e);
    e.EXTI_Line    = line;
    e.EXTI_Mode    = EXTI_Mode_Interrupt;
    e.EXTI_Trigger = EXTI_Trigger_Rising;    /* pressed = rising edge */
    e.EXTI_LineCmd = ENABLE;
    EXTI_Init(&e);

    EXTI_ClearITPendingBit(line);
    /* CMSIS instead of SPL NVIC_Init(): with the reset priority grouping,
     * NVIC_Init() shifts the priority out of range and always writes 0.
     * NVIC_SetPriority() takes 0 (highest) .. 15 (lowest) directly. */
    NVIC_SetPriority(irq, priority & 0x0Fu);
    NVIC_EnableIRQ(irq);
}

/**
 * joystick_irq_enable  Interrupt on press for the switches in `mask`.
 * @param mask      any combination of JOY_* except JOY_DOWN | JOY_RIGHT.
 * @param priority  NVIC priority, 0 (highest) .. 15 (lowest).
 * @return          0 on success, -1 if DOWN and RIGHT were both asked for.
 *
 * Call joystick_init() first.
 */
int joystick_irq_enable(uint8_t mask, uint8_t priority)
{
    if ((mask & JOY_DOWN) && (mask & JOY_RIGHT)) {
        return -1;                     /* both need EXTI line 0 */
    }

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);   /* EXTICR */

    if (mask & JOY_UP) {               /* PA4 -> line 4 */
        exti_line_enable(EXTI_PortSourceGPIOA, EXTI_PinSource4,
                         EXTI_Line4, EXTI4_IRQn, priority);
    }
    if (mask & JOY_DOWN) {             /* PB0 -> line 0 */
        joyLine0Owner = JOY_DOWN;
        exti_line_enable(EXTI_PortSourceGPIOB, EXTI_PinSource0,
                         EXTI_Line0, EXTI0_IRQn, priority);
    }
    if (mask & JOY_RIGHT) {            /* PC0 -> line 0 */
        joyLine0Owner = JOY_RIGHT;
        exti_line_enable(EXTI_PortSourceGPIOC, EXTI_PinSource0,
                         EXTI_Line0, EXTI0_IRQn, priority);
    }
    if (mask & JOY_LEFT) {             /* PC1 -> line 1 */
        exti_line_enable(EXTI_PortSourceGPIOC, EXTI_PinSource1,
                         EXTI_Line1, EXTI1_IRQn, priority);
    }
    if (mask & JOY_CENTER) {           /* PB5 -> line 5 */
        exti_line_enable(EXTI_PortSourceGPIOB, EXTI_PinSource5,
                         EXTI_Line5, EXTI9_5_IRQn, priority);
    }
    return 0;
}

/**
 * joystick_irq_take  Switches that raised an interrupt since the last call.
 *                    Read-and-clear, with interrupts masked so an event
 *                    landing between the read and the clear is not lost.
 */
uint8_t joystick_irq_take(void)
{
    uint8_t ev;
    uint32_t primask = __get_PRIMASK();

    __disable_irq();
    ev = joyIrqEvents;
    joyIrqEvents = 0;
    __set_PRIMASK(primask);

    return ev;
}

/* The handlers only record the event and clear the pending bit. If the
 * pending bit is not cleared the NVIC re-enters the handler forever. */

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        joyIrqEvents |= joyLine0Owner;
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void EXTI1_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line1) != RESET) {
        joyIrqEvents |= JOY_LEFT;
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

void EXTI4_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line4) != RESET) {
        joyIrqEvents |= JOY_UP;
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line5) != RESET) {
        joyIrqEvents |= JOY_CENTER;
        EXTI_ClearITPendingBit(EXTI_Line5);
    }
}

#endif /* JOYSTICK_NO_IRQ_HANDLERS */
