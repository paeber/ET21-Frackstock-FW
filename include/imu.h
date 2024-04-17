
#ifndef IMU_H
#define IMU_H

/*! CPP guard */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

#include "pico/stdlib.h"

// Defines
#define IMU_SPI_PORT        spi1
#define IMU_SPI_BAUDRATE    1000000
#define IMU_SPI_CLK         10
#define IMU_SPI_MOSI        11
#define IMU_SPI_MISO        12
#define IMU_SPI_CS          13

#define IMU_INT1            14
#define IMU_INT2            15

// IMU Registers
#define BMI323_CHIP_ID              0x00
#define BMI323_ERR_REG              0x01
#define BMI323_STATUS               0x02
#define BMI323_ACC_X                0x03
#define BMI323_ACC_Y                0x04
#define BMI323_ACC_Z                0x05
#define BMI323_GYR_X                0x06
#define BMI323_GYR_Y                0x07
#define BMI323_GYR_Z                0x08
#define BMI323_TEMP                 0x09
#define BMI323_SENSOR_TIME_0        0x0A
#define BMI323_SENSOR_TIME_1        0x0B
#define BMI323_SAT_FLAGS            0x0C
#define BMI323_INT_STATUS_INT1      0x0D
#define BMI323_INT_STATUS_INT2      0x0E
#define BMI323_INT_STATUS_IBI       0x0F
#define BMI323_FEATURE_IO0          0x10
#define BMI323_FEATURE_IO1          0x11
#define BMI323_FEATURE_IO2          0x12
#define BMI323_FEATURE_IO3          0x13
#define BMI323_FEATURE_IO_STATUS    0x14
#define BMI323_FIFO_FILL_LEVEL      0x15
#define BMI323_FIFO_DATA            0x16

#define BMI323_ACC_CONF             0x20
#define BMI323_GYR_CONF             0x21



#define BMI323_CMD                  0x7E
#define BMI323_CFG_RESET            0x7F


// Functions
void BMI_set_reg(uint8_t reg_addr, uint16_t *reg_data, uint32_t length);
void BMI_get_reg(uint8_t reg_addr, uint16_t *reg_data, uint32_t length);

void IMU_init();
void IMU_Tick();


#ifdef __cplusplus
}
#endif /* End of CPP guard */

#endif // IMU_H
