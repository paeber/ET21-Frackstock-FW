
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

#define BMI323_CHIP_ID_VAL  0x0043

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

#define BMI323_IO_INT_CTRL          0x38
#define BMI323_INT_CONF             0x39
#define BMI323_INT_MAP1             0x3A
#define BMI323_INT_MAP2             0x3B

#define BMI323_FEATURE_CTRL             0x40
#define BMI323_FEATURE_DATA_ADDR        0x41
#define BMI323_FEATURE_DATA_TX          0x42
#define BMI323_FEATURE_DATA_STATUS      0x43
#define BMI323_FEATRUE_ENGINE_STATUS    0x45
#define BMI323_FEATURE_EVENT_EXT        0x47


#define BMI323_CMD                  0x7E
#define BMI323_CFG_RESET            0x7F

#define FEATURE_ORIENT_1            0x1C
#define FEATURE_ORIENT_2            0x1D
#define FEATURE_TAP_1               0x1E
#define FEATURE_TAP_2               0x1F
#define FEATURE_TAP_3               0x20


#define SINGLE_TAP_DETECT           (0x1 << 3)
#define DOUBLE_TAP_DETECT           (0x1 << 4)
#define TRIPPLE_TAP_DETECT          (0x1 << 5)
#define SINGLE_TAP_EN               (0x1 << 12)
#define DOUBLE_TAP_EN               (0x1 << 13)
#define TRIPPLE_TAP_EN              (0x1 << 14)

// Config Value Defines
#define ACC_ODR_25                  (uint16_t)(0b0110 << 0)
#define ACC_ODR_50                  (uint16_t)(0b0111 << 0)
#define ACC_ODR_100                 (uint16_t)(0b1000 << 0)
#define ACC_ODR_200                 (uint16_t)(0b1001 << 0)
#define ACC_ODR_400                 (uint16_t)(0b1010 << 0)
#define ACC_ODR_800                 (uint16_t)(0b1011 << 0)

#define ACC_RANGE_2G                (uint16_t)(0b000 << 4)
#define ACC_RANGE_4G                (uint16_t)(0b001 << 4)
#define ACC_RANGE_8G                (uint16_t)(0b010 << 4)
#define ACC_RANGE_16G               (uint16_t)(0b011 << 4)

#define ACC_BW_ODR2                 (uint16_t)(0b1 << 7)
#define ACC_BW_ODR4                 (uint16_t)(0b1 << 7)

#define GYR_ODR_25                  (uint16_t)(0b0110 << 0)
#define GYR_ODR_50                  (uint16_t)(0b0111 << 0)
#define GYR_ODR_100                 (uint16_t)(0b1000 << 0)
#define GYR_ODR_200                 (uint16_t)(0b1001 << 0)
#define GYR_ODR_400                 (uint16_t)(0b1010 << 0)
#define GYR_ODR_800                 (uint16_t)(0b1011 << 0)

#define GYR_RANGE_125               (uint16_t)(0b000 << 4)
#define GYR_RANGE_250               (uint16_t)(0b001 << 4)
#define GYR_RANGE_500               (uint16_t)(0b010 << 4)
#define GYR_RANGE_1000              (uint16_t)(0b011 << 4)
#define GYR_RANGE_2000              (uint16_t)(0b100 << 4)

#define GYR_BW_ODR2                 (uint16_t)(0b0 << 7)
#define GYR_BW_ODR4                 (uint16_t)(0b1 << 7)

// Enums
typedef enum {
    IMU_STATE_UNKNOWN = 0,
    IMU_STATE_REBOOT,
    IMU_STATE_COM_OK,
    IMU_STATE_FEATURE_EN,
    IMU_STATE_SENSOR_EN,
    IMU_STATE_FEATURES_CONFIGURED,
    IMU_STATE_IT_EN,
    IMU_STATE_CONFIGURED,
    IMU_STATE_ERROR
} IMU_State_t;

// Functions
void BMI_set_reg(uint8_t reg_addr, uint16_t *reg_data, uint32_t length);
void BMI_get_reg(uint8_t reg_addr, uint16_t *reg_data, uint32_t length);

void IMU_init();
void IMU_Tick();
uint16_t IMU_Startup();
uint16_t IMU_Enable_Feature_Engine();
uint16_t IMU_Configure_Sensor();
uint16_t IMU_Configure_Features();
uint16_t IMU_Configure_IT();
IMU_State_t IMU_get_state();


#ifdef __cplusplus
}
#endif /* End of CPP guard */

#endif // IMU_H
