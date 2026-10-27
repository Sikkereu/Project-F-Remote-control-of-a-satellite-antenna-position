#include "stm32f30x_conf.h"
#include "30010_io.h"
#include <string.h>
#include "LSM9DS1.h"
#include <math.h>

int16_t meanArray(int16_t arr[]){
	int n = sizeof(arr) / sizeof(arr[0]);
	int32_t sum = 0;
	for (uint8_t i = 0 ; i < n ; i++){
		sum += arr[i];
	}
	int32_t mean = sum / n;
	return (int16_t)mean;
}

int main(void)
{

    uint8_t whoAmI; //Who am i Mag
	int16_t gyroX;
	int16_t gyroY;
	int16_t gyroZ;
    int16_t temperature;
    int16_t magX;
    int16_t magY;
    int16_t magZ;
    int16_t accX;
    int16_t accY;
    int16_t accZ;

	// uint8_t data = 0;
    uint8_t output;//Who am i Gyro
    uint8_t dummy = 0;
	uint8_t data = READ | 0x0F;
    uint8_t gyroadr = WRITE | 0x10; // CTRL_REG1_G  
    uint8_t gyro_on = 0x20;
    uint8_t magadr = WRITE | 0x20;
    uint8_t mag_on = 0xF8;

    // Initialize UART for printf()
    uart_init(9600);

	init_SPI3_9DOF();
        GPIO_SetBits(GPIOD, GPIO_Pin_2); // Gyro/Acc/Temp OFF
        GPIO_SetBits(GPIOC, GPIO_Pin_8); // Magnetometer OFF

    // SPI Gyro Init:

        GPIO_SetBits(GPIOC, GPIO_Pin_8);      // Make sure MAG is deselected
        GPIO_ResetBits(GPIOD, GPIO_Pin_2);    // Select Gyro/Acc, CS = 0 - Start Transmission
        
            while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) != SET) {}
            SPI_SendData8(SPI3, gyroadr);    // CTRL_REG1_G   

            while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET){}
            dummy = SPI_ReceiveData8(SPI3);

			while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) != SET) { }
			SPI_SendData8(SPI3, gyro_on); //Dummy data to keep clock running for read action

			while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET) {}
			output = SPI_ReceiveData8(SPI3);

            while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET){}
			while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY) == SET){}
			GPIO_SetBits(GPIOD, GPIO_Pin_2); // CS = 1 - End Transmission

    // SPI Mag Init:

        GPIO_SetBits(GPIOD, GPIO_Pin_2);       // Make sure Gyro/Acc is deselected
        GPIO_ResetBits(GPIOC, GPIO_Pin_8);     // Select Magnetometer // CS = 0 - Start Transmission

			while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) != SET) { }
			SPI_SendData8(SPI3, magadr);

            while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET){}
            dummy = SPI_ReceiveData8(SPI3);

			while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) != SET) { }
			SPI_SendData8(SPI3, mag_on); //Dummy data to keep clock running for read action

			while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET) {}
			whoAmI = SPI_ReceiveData8(SPI3);

            while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET){}
			while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY) == SET){}

			GPIO_SetBits(GPIOC, GPIO_Pin_8); // CS = 1 - End Transmission

        // ---------------- Magnetometer CTRL_REG3_M ----------------
        // 0x80:
        // I2C disabled
        // SIM = 0 -> 4-wire SPI
        // MD = 00 -> continuous conversion

        GPIO_ResetBits(GPIOC, GPIO_Pin_8); // CS = 0 - Start Transmission
            while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) != SET) {}
            SPI_SendData8(SPI3, WRITE | 0x22);
            while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET) {}
            dummy = SPI_ReceiveData8(SPI3);

            while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) != SET) {}
            SPI_SendData8(SPI3, 0x80);
            while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET) {}
            dummy = SPI_ReceiveData8(SPI3);

            while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET){}
            while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY) == SET) {}

            GPIO_SetBits(GPIOC, GPIO_Pin_8); // CS = 1 - End Transmission



    whoAmI = SPI3_read_byte(data, Magnetometer_SEL); // Read WHO_AM_I from magnetometer
    output = SPI3_read_byte(data, GyroAccTemp_SEL); // Read WHO_AM_I from gyro/accelerometer
            printf("\nWHO_AM_I Gyro = 0x%02X\n\r", output);

            printf("WHO_AM_I Mag = 0x%02X\n\r", whoAmI);
    int16_t xAvg[1000];
    int16_t yAvg[1000];
    int16_t zAvg[1000];
	while (1)
    {
        readAccData(&accX, &accY, &accZ);

        //readTempData(&temperature);

        for(int i = 0 ; i < 1000 ; i++){
            xAvg[i] = accX;
            yAvg[i] = accY;
            zAvg[i] = accZ;
            if(i == 999){
                // Calculate mean of the arrays
                int16_t xMean = meanArray(xAvg);
                int16_t yMean = meanArray(yAvg);
                int16_t zMean = meanArray(zAvg);
                //Convert to m/s^2 and print
                float xMPS2 = (float)xMean * 0.061 / 1000 * 9.82;
                float yMPS2 = (float)yMean * 0.061 / 1000 * 9.82;
                float zMPS2 = (float)zMean * 0.061 / 1000 * 9.82;
                printf("Acc X: %6.2f | Y: %6.2f | Z: %6.2f |\n\r",//| Temp: %6d | Mag X: %6d | Y: %6d | Z: %6d |\n\r",
                xMPS2, yMPS2, zMPS2);
                //Calculate roll and pitch from the accelerometer data and print
                float roll = atan2(yMPS2, zMPS2) * 180 / M_PI;
                float pitch = atan2(-xMPS2, sqrt(pow(yMPS2, 2) + pow(zMPS2, 2))) * 180 / M_PI;
                printf("Roll: %6.2f | Pitch: %6.2f |\n\r", roll, pitch);
                //(int)temperature, magX, magY, magZ);
            }
        }
        for(int i = 0 ; i < 10000000; i++){} // Delay

        //readMagData(&magX, &magY, &magZ);

        

    }
    }

