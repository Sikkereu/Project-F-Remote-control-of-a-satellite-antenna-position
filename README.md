# 30021 standard project

Baseline PlatformIO project for course 30021 Digital Instrumentation: STM32
Nucleo-F302R8 with the mbed application shield (MBED-016.1), bare metal,
Standard Peripheral Library. The drivers are the code from assignments 1 to 3,
generalised and given one naming scheme (`module_verb()`).

## Starting a new assignment

1. Copy the folder `30021_Standard` and rename the copy (for example
   `Assignment4`).
2. Replace `src/main.c`. The current one is a self-test that uses every
   module; it is also the quickest example of how they fit together.
3. Include only the headers you need. PlatformIO's library finder only builds
   the modules that are `#include`d from `src/`.
4. `pio run -t upload`, then `pio device monitor` (9600 8N1) or PuTTY.

Old versions of `main.c` can be parked in `src/Archive/`; that folder is
excluded from the build.

## Layout

```
platformio.ini
ldscript/        linker script (ROM limited to 62 KB, see flash)
include/vendor/  CMSIS + SPL headers (unchanged)
src/vendor/      SPL sources, startup, system_stm32f30x.c (unchanged)
src/main.c       your program
lib/<module>/    the drivers below
```

## Modules

| Module | Header | From | Main functions |
|---|---|---|---|
| board | `board.h` | all | Pin map of shield and Nucleo, list of free pins, `board_hclk_hz()`, `board_timer_clock_hz(TIMx)` |
| gpio | `gpio.h` | 1 | `gpio_init_input/output/analog/af()`, `gpio_set/clear/toggle/read()` |
| uart | `30010_io.h` | 1 | Course USART2 driver: `uart_init(baud)`, printf, `uart_get_char()`, new blocking `uart_getc()` |
| joystick | `joystick.h` | 1, 2.6 | `joystick_init()`, `joystick_read()` raw, `joystick_poll()` debounced press events, `joystick_irq_enable(mask, prio)` + `joystick_irq_take()` |
| led | `led.h` | 1 | `led_init()`, `led_set(LED_RED ... LED_WHITE)` |
| tick | `tick.h` | 1, 2 | TIM6 periodic interrupt: `tick_init(hz)`, `tick_take()`, `tick_count()`, `tick_wait()` |
| delay | `delay.h` | new | SysTick: `delay_init()`, `millis()`, `delay_ms()`, and `delay_us()` on the DWT cycle counter |
| adc | `adc.h` | 2 | `adc_init()`, `adc_read(ch)`, `adc_read_avg(ch, n)`, `adc_vdda()` / `adc_vdda_mv()`, `adc_to_volt()` / `adc_to_mv()`, `adc_pin_init()` for extra inputs |
| pwm | `pwm.h` | 3 | Any timer channel: `pwm_init(&PWM_PA6_TIM16_CH1, freq, steps)`, `pwm_set()`, `pwm_set_permille()`, `pwm_set_us()` (servos), `pwm_wait_periods()` |
| lcd | `lcd.h` | 2 | Course LCD driver plus `lcd_init()`, `lcd_clear()`, `lcd_printf(x, line, fmt, ...)`, `lcd_update()`, `lcd_set_pixel()` |
| flash | `flash.h` | 2 | Course flash helpers plus `flash_write_words()` / `flash_write_floats()` (erase, write, verify) |

## Which hardware each module takes

| Module | Peripherals | Interrupt handlers | Pins |
|---|---|---|---|
| uart | USART2 | `USART2_IRQHandler` | PA2, PA3 |
| joystick | GPIO, EXTI (only with `joystick_irq_enable`) | `EXTI0`, `EXTI1`, `EXTI4`, `EXTI9_5` | PA4, PB0, PB5, PC0, PC1 |
| led | GPIO | none | PB4, PC7, PA9 |
| tick | TIM6 | `TIM6_DACUNDER_IRQHandler` | none |
| delay | SysTick, DWT | `SysTick_Handler` | none |
| adc | ADC1 | none | PA0, PA1 (+ any added) |
| pwm | TIM1/2/15/16/17, as chosen | none | as chosen |
| lcd | SPI2 | none | PB6, PB13, PB14, PB15, PA8 |
| flash | FLASH, page 31 | none | none |

TIM2 is no longer used for the time base (assignment 1 did), so it is free
for PWM: PA5 (CH1), PB10 (CH3, the speaker) and PB11 (CH4). If an assignment
needs its own EXTI handler, add `-D JOYSTICK_NO_IRQ_HANDLERS` in
`platformio.ini`.

## PWM examples

```c
/* 3.1: 10 kHz, 256 steps on PA6, 50 % */
pwm_init(&PWM_PA6_TIM16_CH1, 10000, 256);
pwm_set(&PWM_PA6_TIM16_CH1, 128);

/* 3.3: two servos on separate timers, 50 Hz, 1 us resolution */
pwm_init(&PWM_PA6_TIM16_CH1, 50, 20000);
pwm_init(&PWM_PA7_TIM17_CH1, 50, 20000);
pwm_set_us(&PWM_PA6_TIM16_CH1, 1500);
```

`pwm_init()` returns a negative `PWM_ERR_*` code if the frequency times the
steps does not divide the 64 MHz timer clock exactly, if PSC or ARR would not
fit, or if another channel already runs the same timer at a different setting.

## Changes to the course-provided files

- `lcd.c`, `init_spi_lcd()`: `GPIOA->MODER &= ~0x00000003 << (8 * 2)` clears
  bits 0 to 17, not just PA8, because `~` binds tighter than `<<`. It reset
  PA0 to PA8, which is why UART stopped working and the original needed its
  "needed for UART to work" line. It also reset the ADC pins (PA0, PA1), the
  joystick pull-down on PA4 and a PWM pin on PA6 if they were set up before
  the LCD. Fixed with parentheses; the init order no longer matters.
- `lcd.c`: `generate_line_buff()` wrapped after 42 characters (8-bit index)
  and read outside the font for non-printable characters;
  `write_line_buff()` overran the buffer for x >= 128 or line > 3.
- `30010_io.c`: the USART2 handler now clears an overrun (ORE). With the RX
  interrupt on, an overrun re-enters the handler forever and the program
  hangs. `uart_get_char()` masks interrupts around the buffer update.
- `flash.c`: float write via `memcpy` instead of a pointer cast.
- Linker script: ROM is 62 KB so page 31 (0x0800F800), used for calibration
  data, can never be overwritten by the program.
- NVIC priorities are set with CMSIS `NVIC_SetPriority()` (0 highest, 15
  lowest). With the reset priority grouping, the SPL `NVIC_Init()` used in
  assignments 1 and 2 shifts the pre-emption priority by 4 - 7 bits, an
  out-of-range shift that in practice writes 0, and masks the
  sub-priority to 0. So every interrupt there ran at priority 0, whatever
  the struct said, unless `NVIC_PriorityGroupConfig()` was called first.

## Other notes

- `SystemCoreClock` stays at 72 MHz because `system_stm32f30x.c` sets the PLL
  to 64 MHz without updating it. Use `board_hclk_hz()`.
- The ADC default sample time is 19.5 cycles (2.44 us at 8 MHz) instead of
  the 1.5 cycles in assignment 2. That covers the pots' source resistance
  (datasheet Table 67) and the 2.2 us VREFINT needs. Change it per call with
  `adc_read_st()` or globally with `-D ADC_DEFAULT_SAMPLE_TIME=...`.
- The project compiles and links cleanly with arm-none-eabi-gcc. It has not
  yet been run on the board, so treat the first upload as the hardware test.
# Project-F-Remote-control-of-a-satellite-antenna-position
