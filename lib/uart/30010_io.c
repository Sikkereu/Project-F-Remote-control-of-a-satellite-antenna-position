/*
 * 30010_io.c  Course-provided USART2 driver (printf over the ST-Link virtual
 * COM port). Changes for the standard project, marked in the code:
 *   - USART2_IRQHandler clears an overrun (ORE), which otherwise hangs
 *   - uart_get_char() masks interrupts around the buffer update
 *   - uart_getc(), a blocking read (assignment 3, appendix 2)
 */
#include "30010_io.h"

/****************************/
/*** USB Serial Functions ***/
/****************************/
volatile uint8_t UART_BUFFER[UART_BUFFER_LENGTH] = {0};
volatile uint8_t UART_END_IDX = 0;
volatile uint8_t UART_START_IDX = 0;
volatile uint8_t UART_COUNT = 0;

uint8_t uart_get_char(){
    uint8_t val = 0;
    // Standard project: interrupts masked around the read, because
    // UART_COUNT-- is a read-modify-write that the RX interrupt (which
    // increments it) could otherwise land in the middle of, losing a byte.
    __disable_irq();
    if (UART_COUNT > 0) {
        val = UART_BUFFER[UART_START_IDX++];
        UART_COUNT--;
    }
    __enable_irq();
    return val;
}

uint8_t uart_getc(void){
    while (UART_COUNT == 0) { }
    return uart_get_char();
}

void uart_put_char(uint8_t c) {
    USART_SendData(USART2, (uint8_t)c);
    while(USART_GetFlagStatus(USART2, USART_FLAG_TXE)  == RESET){}
}

int _write_r(struct _reent *r, int file, char *ptr, int len) {
    int n;
    (void)r;      // every stream (stdout, stderr) goes to USART2
    (void)file;

    for (n = 0; n < len; n++) {
        if (ptr[n] == '\n') {
            uart_put_char('\r');
        }
        uart_put_char(ptr[n] & (uint16_t)0x01FF);
    }

    return len;
}

void USART2_IRQHandler(void)
{
    // Standard project: an overrun (a byte arriving before the previous one
    // was read, e.g. while interrupts were masked) sets ORE, which also
    // raises this interrupt when RXNEIE = 1 ([RM] USART_ISR.ORE). Without
    // clearing it the handler is re-entered forever and the program hangs.
    if (USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET)
    {
        USART_ClearFlag(USART2, USART_FLAG_ORE);
    }
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        UART_BUFFER[UART_END_IDX++] = (uint8_t)(USART2->RDR & 0xFF);
        if (UART_COUNT == UART_BUFFER_LENGTH-1){
            UART_START_IDX++;
        } else {
            UART_COUNT++;
        }
    }
}

void uart_clear(){
    UART_START_IDX = 0;
    UART_END_IDX = 0;
    UART_COUNT = 0;
}

uint8_t uart_get_count(){
    return UART_COUNT;
}

void uart_init(uint32_t baud) {
    setbuf(stdout, NULL); // Set stdout to disable line buffering
    setbuf(stdin,  NULL); // Set stdin  to disable line buffering

    // Enable Clocks
    RCC->AHBENR  |= RCC_AHBPeriph_GPIOA;    // Enable Clock for GPIO Bank A
    RCC->APB1ENR |= RCC_APB1Periph_USART2;  // Enable Clock for USART2

    // Connect pins to USART2
    GPIOA->AFR[2 >> 0x03] &= ~(0x0000000F << ((2 & 0x00000007) * 4)); // Clear alternate function for PA2
    GPIOA->AFR[2 >> 0x03] |=  (0x00000007 << ((2 & 0x00000007) * 4)); // Set alternate 7 function for PA2
    GPIOA->AFR[3 >> 0x03] &= ~(0x0000000F << ((3 & 0x00000007) * 4)); // Clear alternate function for PA3
    GPIOA->AFR[3 >> 0x03] |=  (0x00000007 << ((3 & 0x00000007) * 4)); // Set alternate 7 function for PA3

    // Configure pins PA2 and PA3 for 10 MHz alternate function
    GPIOA->OSPEEDR &= ~(0x00000003 << (2 * 2) | 0x00000003 << (3 * 2));    // Clear speed register
    GPIOA->OSPEEDR |=  (0x00000001 << (2 * 2) | 0x00000001 << (3 * 2));    // set speed register (0x01 - 10 MHz, 0x02 - 2 MHz, 0x03 - 50 MHz)
    GPIOA->OTYPER  &= ~(0x0001     << (2)     | 0x0001     << (3));        // Clear output type register
    GPIOA->OTYPER  |=  (0x0000     << (2)     | 0x0000     << (3));        // Set output type register (0x00 - Push pull, 0x01 - Open drain)
    GPIOA->MODER   &= ~(0x00000003 << (2 * 2) | 0x00000003 << (3 * 2));    // Clear mode register
    GPIOA->MODER   |=  (0x00000002 << (2 * 2) | 0x00000002 << (3 * 2));    // Set mode register (0x00 - Input, 0x01 - Output, 0x02 - Alternate Function, 0x03 - Analog in/out)
    GPIOA->PUPDR   &= ~(0x00000003 << (2 * 2) | 0x00000003 << (3 * 2));    // Clear push/pull register
    GPIOA->PUPDR   |=  (0x00000001 << (2 * 2) | 0x00000001 << (3 * 2));    // Set push/pull register (0x00 - No pull, 0x01 - Pull-up, 0x02 - Pull-down)

    //Configure USART2
    USART2->CR1 &= ~0x00000001; // Disable USART2
    USART2->CR2 &= ~0x00003000; // Clear CR2 Configuration
    USART2->CR2 |=  0x00000000; // Set 1 stop bits
    USART2->CR1 &= ~(0x00001000 | 0x00000400 | 0x00000200 | 0x00000008 | 0x00000004); // Clear CR1 Configuration
    USART2->CR1 |=  0x00000000; // Set word length to 8 bits
    USART2->CR1 |=  0x00000000; // Set parity bits to none
    USART2->CR1 |=  0x00000004 | 0x00000008; // Set mode to RX and TX
    USART2->CR3 &= ~(0x00000100 | 0x00000200); // Clear CR3 Configuration
    USART2->CR3 |=  0x00000000; // Set hardware flow control to none

    uint32_t divider = 0, apbclock = 0, tmpreg = 0;
    RCC_ClocksTypeDef RCC_ClocksStatus;
    RCC_GetClocksFreq(&RCC_ClocksStatus); // Get USART2 Clock frequency
    apbclock = RCC_ClocksStatus.USART2CLK_Frequency;

    if ((USART2->CR1 & 0x00008000) != 0) {
      // (divider * 10) computing in case Oversampling mode is 8 Samples
      divider = (2 * apbclock) / baud;
      tmpreg  = (2 * apbclock) % baud;
    } else {
      // (divider * 10) computing in case Oversampling mode is 16 Samples
      divider = apbclock / baud;
      tmpreg  = apbclock % baud;
    }

    if (tmpreg >=  baud / 2) {
        divider++;
    }

    if ((USART2->CR1 & 0x00008000) != 0) {
        // get the LSB of divider and shift it to the right by 1 bit
        tmpreg = (divider & (uint16_t)0x000F) >> 1;
        // update the divider value
        divider = (divider & (uint16_t)0xFFF0) | tmpreg;
    }

    USART2->BRR = (uint16_t)divider; // Configure baud rate
    USART2->CR1 |= 0x00000001; // Enable USART2

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    NVIC_EnableIRQ(USART2_IRQn);
}
