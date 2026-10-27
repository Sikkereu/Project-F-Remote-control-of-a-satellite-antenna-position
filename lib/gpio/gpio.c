/**
 * @file    gpio.c
 * @brief   GPIO set-up helpers. See gpio.h.
 */
#include "gpio.h"

/**
 * gpio_clock_enable  Switches on the AHB clock for one GPIO port
 *                    (RCC_AHBENR, [RM] 9.4.6). A port without its clock
 *                    ignores every write, so this always comes first.
 */
void gpio_clock_enable(GPIO_TypeDef *port)
{
    if      (port == GPIOA) RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    else if (port == GPIOB) RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    else if (port == GPIOC) RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
    else if (port == GPIOD) RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);
    else if (port == GPIOF) RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);
}

void gpio_init_input(GPIO_TypeDef *port, uint16_t pins, GPIOPuPd_TypeDef pupd)
{
    GPIO_InitTypeDef g;

    gpio_clock_enable(port);
    GPIO_StructInit(&g);
    g.GPIO_Pin  = pins;
    g.GPIO_Mode = GPIO_Mode_IN;
    g.GPIO_PuPd = pupd;
    GPIO_Init(port, &g);
}

void gpio_init_output(GPIO_TypeDef *port, uint16_t pins, GPIOSpeed_TypeDef speed)
{
    GPIO_InitTypeDef g;

    gpio_clock_enable(port);
    GPIO_StructInit(&g);
    g.GPIO_Pin   = pins;
    g.GPIO_Mode  = GPIO_Mode_OUT;
    g.GPIO_OType = GPIO_OType_PP;
    g.GPIO_Speed = speed;
    g.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(port, &g);
}

void gpio_init_analog(GPIO_TypeDef *port, uint16_t pins)
{
    GPIO_InitTypeDef g;

    gpio_clock_enable(port);
    GPIO_StructInit(&g);
    g.GPIO_Pin  = pins;
    g.GPIO_Mode = GPIO_Mode_AN;
    g.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(port, &g);
}

void gpio_init_af(GPIO_TypeDef *port, uint16_t pin, uint8_t af, GPIOPuPd_TypeDef pupd)
{
    GPIO_InitTypeDef g;

    gpio_clock_enable(port);
    GPIO_PinAFConfig(port, gpio_pin_number(pin), af);

    GPIO_StructInit(&g);
    g.GPIO_Pin   = pin;
    g.GPIO_Mode  = GPIO_Mode_AF;
    g.GPIO_OType = GPIO_OType_PP;
    g.GPIO_Speed = GPIO_Speed_50MHz;
    g.GPIO_PuPd  = pupd;
    GPIO_Init(port, &g);
}

uint8_t gpio_pin_number(uint16_t pin)
{
    uint8_t n = 0;

    while (n < 15u && (pin & (1u << n)) == 0u) {
        n++;
    }
    return n;
}
