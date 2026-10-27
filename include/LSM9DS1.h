#ifndef LSM9DS1_H_
#define LSM9DS1_H_

#include "stm32f30x_conf.h" // STM32 config
#include "30010_io.h" 		// Input/output library for this course

#define READ 0b10000000
#define WRITE 0b00000000
#define GyroAccTemp_SEL 0
#define Magnetometer_SEL 1

//Changed: Added register addresses that were missing (from Assignment4_Simple/lib/lsm9ds1/lsm9ds1.h)
// Accel/gyro registers
#define CTRL_REG1_G     0x10    // gyro data rate and full scale
#define CTRL_REG6_XL    0x20    // accel data rate and full scale
#define CTRL_REG8       0x22    // BDU and address auto-increment
#define CTRL_REG9       0x23    // I2C_DISABLE
// Magnetometer registers
#define CTRL_REG1_M     0x20    // X/Y performance mode and data rate
#define CTRL_REG2_M     0x21    // full scale
#define CTRL_REG3_M     0x22    // I2C_DISABLE and operating mode
#define CTRL_REG4_M     0x23    // Z performance mode
#define CTRL_REG5_M     0x24    // BDU

//Changed: Added expected WHO_AM_I values, so main() can check that the sensor answers
#define WHO_AM_I_AG_VALUE   0x68
#define WHO_AM_I_M_VALUE    0x3D

//Changed: Added datasheet sensitivities (DS Table 3 and 5) as integer fractions,
// so the raw counts can be printed in milli-units without float printf
// acc  +-2 g      -> 0.061 mg per count     = 61 / 1000
// gyro +-245 dps  -> 8.75 mdps per count    = 875 / 100
// mag  +-4 gauss  -> 0.14 mgauss per count  = 14 / 100
// temp 16 counts per degC, 0 at 25 degC
#define ACC_MG_PER_LSB_NUM      61
#define ACC_MG_PER_LSB_DEN      1000
#define GYRO_MDPS_PER_LSB_NUM   875
#define GYRO_MDPS_PER_LSB_DEN   100
#define MAG_MGAUSS_PER_LSB_NUM  14
#define MAG_MGAUSS_PER_LSB_DEN  100
#define TEMP_LSB_PER_DEGC       16

void init_SPI3_9DOF();
uint8_t SPI3_read_byte(uint8_t data, uint8_t selector);
void SPI3_write_reg(uint8_t reg, uint8_t value, uint8_t selector); //Changed: Added write function for register set-up
void readGyroData(int16_t *out_x, int16_t *out_y, int16_t *out_z);
void readAccData(int16_t *out_x, int16_t *out_y, int16_t *out_z);
void readTempData(int16_t *out_temp);
void readMagData(int16_t *out_x, int16_t *out_y, int16_t *out_z);

#endif /* LSM9DS1_H_ */
