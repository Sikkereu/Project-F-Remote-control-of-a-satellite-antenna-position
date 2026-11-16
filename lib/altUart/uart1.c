#include "uart1.h"

void initUART1(uint32_t baudrate){

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    //Configure PA9 and PA10 for USART1 TX and RX
    GPIO_InitTypeDef GPIO_InitStructAll;

    GPIO_InitStructAll.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructAll.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructAll.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructAll.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructAll.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_Init(GPIOA, &GPIO_InitStructAll);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_7);

    USART_InitTypeDef USART_InitStructAll;

    // Configure USART1 for 8 data bits, no parity and 1 stop bit
    USART_InitStructAll.USART_BaudRate = baudrate;
    USART_InitStructAll.USART_WordLength = USART_WordLength_8b;
    USART_InitStructAll.USART_StopBits = USART_StopBits_1;
    USART_InitStructAll.USART_Parity = USART_Parity_No;
    USART_InitStructAll.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructAll.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    USART_Init(USART1, &USART_InitStructAll);
    USART_Cmd(USART1, ENABLE);
}

void uart1SendFloat(float number){
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.2f", number);
    for (int i = 0; i < strlen(buffer) ; i++) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, buffer[i]);
    }
    USART_SendData(USART1, '\n'); //Send newline character to indicate end of float
}

void uart1Send3Floats(float number1, float number2, float number3){
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.2f, %.2f, %.2f", number1, number2, number3);
    for (int i = 0; i < strlen(buffer) ; i++) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, buffer[i]);
    }
    USART_SendData(USART1, '\n'); //Send newline character to indicate end of float
}

void uart1ReceiveFloat(float *number){
    char buffer[32];
    int index = 0;
    char receivedChar;

    //Receive characters until newline or buffer is full
    while (index < sizeof(buffer) - 1) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
        receivedChar = USART_ReceiveData(USART1);
        if (receivedChar == '\n') {
            break;
        }
        buffer[index++] = receivedChar;
    }
    buffer[index] = '\0'; //Null-terminate the string

    //Convert the string to float
    *number = atof(buffer);
}