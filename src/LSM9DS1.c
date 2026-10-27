#include "30010_io.h"
#include "LSM9DS1.h"

void init_SPI3_9DOF(){
	 GPIO_InitTypeDef GPIO_InitStructAll;
	 SPI_InitTypeDef SPI_InitStructAll;

	 //enable clocks
	 RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	 RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	 RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);

	 //init SPI related pins
	 //PC10 - SCLK
	 //PC11 - MISO
	 //PC12 - MOSI
	 GPIO_StructInit(&GPIO_InitStructAll);
	 GPIO_InitStructAll.GPIO_Mode = GPIO_Mode_AF;
	 GPIO_InitStructAll.GPIO_Speed = GPIO_Speed_10MHz;
	 GPIO_InitStructAll.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
	 GPIO_InitStructAll.GPIO_OType = GPIO_OType_PP;
	 GPIO_Init(GPIOC, &GPIO_InitStructAll);

	 //connect pins to AF
	 GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_6);
	 GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_6);
	 GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_6);

	 //init SPI
	 SPI_I2S_DeInit(SPI3);
	 SPI_InitStructAll.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	 SPI_InitStructAll.SPI_Mode = SPI_Mode_Master;
	 SPI_InitStructAll.SPI_DataSize = SPI_DataSize_8b;
	 SPI_InitStructAll.SPI_CPOL = SPI_CPOL_High;
	 SPI_InitStructAll.SPI_CPHA = SPI_CPHA_2Edge;
	 SPI_InitStructAll.SPI_NSS = SPI_NSS_Soft;
	 SPI_InitStructAll.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	 SPI_InitStructAll.SPI_FirstBit = SPI_FirstBit_MSB;
	 SPI_InitStructAll.SPI_CRCPolynomial = 7;
	 SPI_Init(SPI3, &SPI_InitStructAll);
     // Set RX FIFO threshold to 8-bit
     SPI_RxFIFOThresholdConfig(SPI3, SPI_RxFIFOThreshold_QF);    
	 SPI_Cmd(SPI3, ENABLE);

	 //init CS_ pins
	 //Gyro and accelerometer with CS bound to PD2
	 //Magnetometer with CS bound to PC8
	 //Changed: Added setting the CS pins high BEFORE they become outputs, so they go straight to "not selected" instead of pulsing low for a moment
	 GPIO_SetBits(GPIOD, GPIO_Pin_2); // CS = 1
	 GPIO_SetBits(GPIOC, GPIO_Pin_8); // CS = 1
	 GPIO_StructInit(&GPIO_InitStructAll);
	 GPIO_InitStructAll.GPIO_Mode = GPIO_Mode_OUT;
	 GPIO_InitStructAll.GPIO_Speed = GPIO_Speed_10MHz;
	 GPIO_InitStructAll.GPIO_Pin = GPIO_Pin_2;
	 GPIO_InitStructAll.GPIO_OType = GPIO_OType_PP;
	 GPIO_InitStructAll.GPIO_PuPd = GPIO_PuPd_NOPULL;
	 GPIO_Init(GPIOD, &GPIO_InitStructAll);

	 GPIO_StructInit(&GPIO_InitStructAll);
	 GPIO_InitStructAll.GPIO_Mode = GPIO_Mode_OUT;
	 GPIO_InitStructAll.GPIO_Speed = GPIO_Speed_10MHz;
	 GPIO_InitStructAll.GPIO_Pin = GPIO_Pin_8;
	 GPIO_InitStructAll.GPIO_OType = GPIO_OType_PP;
	 GPIO_InitStructAll.GPIO_PuPd = GPIO_PuPd_NOPULL;
	 GPIO_Init(GPIOC, &GPIO_InitStructAll);
	 //Set CS pins to idle
	 GPIO_SetBits(GPIOD, GPIO_Pin_2); // CS = 1
	 GPIO_SetBits(GPIOC, GPIO_Pin_8); // CS = 1
}

uint8_t SPI3_read_byte(uint8_t data, uint8_t selector){
    int output;
    uint8_t dummy;
	switch (selector){
		case 0: //Gyro and accelerometer with CS bound to PD2
			GPIO_SetBits(GPIOC, GPIO_Pin_8);       // Make sure Magnetometer is deselected
			GPIO_ResetBits(GPIOD, GPIO_Pin_2); // CS = 0 - Start Transmission
			while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) != SET) { }
			SPI_SendData8(SPI3, data);
            while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET){}
            dummy = SPI_ReceiveData8(SPI3);
			while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) != SET) { }
			SPI_SendData8(SPI3, 0x00); //Dummy data to keep clock running for read action
			while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET) {}
			output = SPI_ReceiveData8(SPI3);
			while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET){}
			while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY) == SET){}
			GPIO_SetBits(GPIOD, GPIO_Pin_2); // CS = 1 - End Transmission
			break;

		case 1: //Magnetometer with CS bound to PC8
			GPIO_SetBits(GPIOD, GPIO_Pin_2);       // Make sure Gyro/Acc is deselected
			GPIO_ResetBits(GPIOC, GPIO_Pin_8); // CS = 0 - Start Transmission
			while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) != SET) { }
    		SPI_SendData8(SPI3, data);
            while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET) { }
			dummy = SPI_ReceiveData8(SPI3);
			while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) != SET) { }
			SPI_SendData8(SPI3, 0x00); //Dummy data to keep clock running for read action
			while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET){}			
			output = SPI_ReceiveData8(SPI3);
			while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET){}
			while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY) == SET){}
			GPIO_SetBits(GPIOC, GPIO_Pin_8); // CS = 1 - End Transmission
			break;


	}
	return output;
}

//Changed: Added SPI3_write_reg(), same transfer as SPI3_read_byte() but with RW = 0 and the value as second byte
void SPI3_write_reg(uint8_t reg, uint8_t value, uint8_t selector){
    uint8_t dummy;
	if (selector == GyroAccTemp_SEL){
		GPIO_SetBits(GPIOC, GPIO_Pin_8);       // Make sure Magnetometer is deselected
		GPIO_ResetBits(GPIOD, GPIO_Pin_2);     // CS = 0 - Start Transmission
	} else {
		GPIO_SetBits(GPIOD, GPIO_Pin_2);       // Make sure Gyro/Acc is deselected
		GPIO_ResetBits(GPIOC, GPIO_Pin_8);     // CS = 0 - Start Transmission
	}
	while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) != SET) { }
	SPI_SendData8(SPI3, WRITE | reg);
	while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET) { }
	dummy = SPI_ReceiveData8(SPI3);            // empty the RX FIFO
	while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) != SET) { }
	SPI_SendData8(SPI3, value);
	while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET) { }
	dummy = SPI_ReceiveData8(SPI3);            // empty the RX FIFO
	(void)dummy;
	while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET){}
	while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY) == SET){}
	if (selector == GyroAccTemp_SEL){
		GPIO_SetBits(GPIOD, GPIO_Pin_2);       // CS = 1 - End Transmission
	} else {
		GPIO_SetBits(GPIOC, GPIO_Pin_8);       // CS = 1 - End Transmission
	}
}

void readGyroData(int16_t *out_x, int16_t *out_y, int16_t *out_z){
	*out_x = SPI3_read_byte(READ | 0x19, GyroAccTemp_SEL)<<8;
	*out_x |= SPI3_read_byte(READ | 0x18, GyroAccTemp_SEL);

	*out_y = SPI3_read_byte(READ | 0x1B, GyroAccTemp_SEL)<<8;
	*out_y |= SPI3_read_byte(READ | 0x1A, GyroAccTemp_SEL);

	*out_z = SPI3_read_byte(READ | 0x1D, GyroAccTemp_SEL)<<8;
	*out_z |= SPI3_read_byte(READ | 0x1C, GyroAccTemp_SEL);
}

void readAccData(int16_t *out_x, int16_t *out_y, int16_t *out_z){
	*out_x = SPI3_read_byte(READ | 0x29, GyroAccTemp_SEL)<<8;
	*out_x |= SPI3_read_byte(READ | 0x28, GyroAccTemp_SEL);

	*out_y = SPI3_read_byte(READ | 0x2B, GyroAccTemp_SEL)<<8;
	*out_y |= SPI3_read_byte(READ | 0x2A, GyroAccTemp_SEL);

	*out_z = SPI3_read_byte(READ | 0x2D, GyroAccTemp_SEL)<<8;
	*out_z |= SPI3_read_byte(READ | 0x2C, GyroAccTemp_SEL);
}

void readTempData(int16_t *out_temp){
	*out_temp = SPI3_read_byte(READ | 0x16, GyroAccTemp_SEL)<<8;
	*out_temp |= SPI3_read_byte(READ | 0x15, GyroAccTemp_SEL);

}

void readMagData(int16_t *out_x, int16_t *out_y, int16_t *out_z){
	*out_x = SPI3_read_byte(READ | 0x29, Magnetometer_SEL)<<8;
	*out_x |= SPI3_read_byte(READ | 0x28, Magnetometer_SEL);

	*out_y = SPI3_read_byte(READ | 0x2B, Magnetometer_SEL)<<8;
	*out_y |= SPI3_read_byte(READ | 0x2A, Magnetometer_SEL);

	*out_z = SPI3_read_byte(READ | 0x2D, Magnetometer_SEL)<<8;
	*out_z |= SPI3_read_byte(READ | 0x2C, Magnetometer_SEL);
}
