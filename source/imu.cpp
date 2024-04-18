//
// IMU (Inertial Measurement Unit) class
//
// Author:      Pascal Eberhard
// Datasheet:   https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmi323-ds000.pdf
//
// Maybe use?   https://github.com/boschsensortec/BMI323_SensorAPI
//



#include "imu.h"
#include "main.h"

//#include "bmi323.h"
//#include "common.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

#include "led_ring.h"


// Function prototypes
void BMI323_Select();
void BMI323_Deselect();
void BMI323_wait_Miso();
void BMI323_soft_reset();

void IMU_INT_irq(uint gpio, uint32_t events);
void IMU_INT1_handle();
void IMU_INT2_handle();

// Global variables
bool IMU_INT1_flag = false;
bool IMU_INT2_flag = false;


// Interrupt functions
void IMU_INT_irq(uint gpio, uint32_t events) {
    if(gpio == IMU_INT1){
        IMU_INT1_flag = true;
        printf("->IMU INT1\n");
    } else if(gpio == IMU_INT2){
        IMU_INT2_flag = true;
        printf("->IMU INT2\n");
    }

}

void IMU_INT1_handle(){
    uint16_t interrupt, data;
    IMU_INT1_flag = false;
    BMI_get_reg(BMI323_INT_STATUS_INT1, &interrupt, 1);
    printf("IMU INT1: 0x%04X\n", interrupt);

    // Check for tap detection
    if(interrupt & (0x1 << 8)){
        printf("Tap detected\n");

        BMI_get_reg(BMI323_FEATURE_EVENT_EXT, &data, 1);

        if(data & SINGLE_TAP_DETECT){
            printf("Single tap detected\n");
            activeLED_MODE = LED_MODE_RAINBOW;
        } else if(data & DOUBLE_TAP_DETECT){
            printf("Double tap detected\n");
            activeLED_MODE = LED_MODE_FADE;
        } else if(data & TRIPPLE_TAP_DETECT){
            printf("Tripple tap detected\n");
            activeLED_MODE = LED_MODE_ON;
        } else {
            printf("Unknown detected: 0x%04X\n", data);
        }
    }
}

void IMU_INT2_handle(){
    uint16_t interrupt;
    IMU_INT2_flag = false;
    BMI_get_reg(BMI323_INT_STATUS_INT2, &interrupt, 1);
    printf("IMU INT2: 0x%04X\n", interrupt);

    // Check for tap detection
    if(interrupt & (0x1 << 8)){
        printf("Tap detected\n");

    }
}

// Function definitions
void IMU_init(){
    uint16_t config = 0x0000;
    uint16_t data_in[16];
    uint8_t addr = 0 | 0x80;

    // Initialize the SPI port
    spi_init(IMU_SPI_PORT, IMU_SPI_BAUDRATE);
    gpio_set_function(IMU_SPI_CLK, GPIO_FUNC_SPI);
    gpio_set_function(IMU_SPI_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(IMU_SPI_MISO, GPIO_FUNC_SPI);
    //gpio_set_function(IMU_SPI_CS, GPIO_FUNC_SPI);

    //spi_set_format(IMU_SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_init(IMU_SPI_CS);
    gpio_set_dir(IMU_SPI_CS, GPIO_OUT);
    gpio_put(IMU_SPI_CS, 0);
    sleep_ms(1);
    gpio_put(IMU_SPI_CS, 1);
    sleep_ms(1);

    // Initialize the INT pins
    gpio_init(IMU_INT1);
    gpio_set_dir(IMU_INT1, GPIO_IN);
    gpio_pull_up(IMU_INT1);

    gpio_init(IMU_INT2);
    gpio_set_dir(IMU_INT2, GPIO_IN);
    gpio_pull_up(IMU_INT2);


    // Initialize the IMU
    sleep_ms(350);

    // Dummy write/read to wake up the IMU
    spi_write_blocking(IMU_SPI_PORT, &addr, 1);
    spi_read16_blocking(IMU_SPI_PORT, 0, data_in, 1);

    sleep_ms(350);

    // Read the chip ID
    BMI_get_reg(BMI323_CHIP_ID, data_in, 1);
    printf("Chip ID: 0x%04X\n", data_in[0]);

    // Read the error register
    BMI_get_reg(BMI323_ERR_REG, data_in, 1);
    printf("Error register: 0x%04X\n", data_in[0]);
    if(data_in[0] & 0x0001) {
        printf("IMU Power Error\n");
    }

    // Read the status register
    BMI_get_reg(BMI323_STATUS, data_in, 1);
    printf("Status register: 0x%02X\n", data_in[0]);
    if(data_in[0] & 0x1) {
        printf("IMU OK\n");
    }

    // 
    // Enable feature engine
    //
    config = 0x012C;
    BMI_set_reg(BMI323_FEATURE_IO2, &config, 1);
    config = 0x0001;
    BMI_set_reg(BMI323_FEATURE_IO_STATUS, &config, 1);
    BMI_get_reg(BMI323_FEATURE_CTRL, &config, 1);
    config |= 0x1;
    BMI_set_reg(BMI323_FEATURE_CTRL, &config, 1);

    config = 0x0000;
    for(int i = 0; i < 200; i++){
        BMI_get_reg(BMI323_FEATURE_IO1, &config, 1);
        sleep_ms(10);
        if((config & 0x0f) == 0x01){
            break;
        }
    }
    if((config & 0x0f) == 0x01){
        printf("\nFeature engine enabled\n");
    } else {
        printf("\nFeature engine not enabled\n");
    }

    // 3. Configure the device for low power mode
    // config = 0x3127;
    // BMI_set_reg(0x20, &config, 1);

    // BMI_get_reg(BMI323_STATUS, &config, 1);
    // if(!(config & 0x0080)){
    //     printf("IMU no data\n");
    // } else {
    //     BMI_get_reg(BMI323_ACC_X, data_in, 3);
    //     BMI_get_reg(BMI323_SAT_FLAGS, &config, 1);
    //     if(config & 0x07 == 0x0){
    //         printf("Data ok\n");
    //     } else {
    //         printf("Data saturated\n");
    //     }
    // }

    // 4. Configure the device for normal mode
    // config = 0x4027;
    // BMI_set_reg(BMI323_ACC_CONF, &config, 1);
    // config = 0x404B;
    // BMI_set_reg(BMI323_GYR_CONF, &config, 1);
    // BMI_get_reg(BMI323_ACC_X, data_in, 3);

    // Enable the accelerometer
    uint16_t acc_conf = 0x0028; // Default value
    acc_conf |= (0b100 << 12);  // Normal mode
    acc_conf |= (0b1001 << 0);  // 200 Hz
    //acc_conf |= (0b1011 << 0);  // 800 Hz
    //acc_conf |= (0b001 << 8);   // Average 2 samples
    BMI_set_reg(BMI323_ACC_CONF, &acc_conf, 1);
    BMI_get_reg(BMI323_ACC_X, data_in, 3);

    // Enable the gyroscope
    uint16_t gyr_conf = 0x0048; // Default value 
    gyr_conf |= (0b100 << 12);  // Normal mode
    gyr_conf |= (0b1001 << 0);  // 200 Hz
    //gyr_conf |= (0b1011 << 0);  // 800 Hz
    //gyr_conf |= (0b001 << 8);   // Average 2 samples
    BMI_set_reg(BMI323_GYR_CONF, &gyr_conf, 1);
    BMI_get_reg(BMI323_GYR_X, data_in, 3);

    
    //
    // Configure Features
    //

    // Configure Tab detection
    uint16_t fAddr = (uint16_t)FEATURE_TAP_1;
    config = 0x0074;    // Set x axis
    BMI_set_reg(BMI323_FEATURE_DATA_ADDR, &fAddr, 1);
    BMI_set_reg(BMI323_FEATURE_DATA_TX, &config, 1);

    sleep_ms(10);

    BMI_get_reg(BMI323_FEATURE_IO0, &config, 1);
    config |= SINGLE_TAP_EN;    // Enable single tap detection
    config |= DOUBLE_TAP_EN;    // Enable double tap detection
    config |= TRIPPLE_TAP_EN;   // Enable tripple tap detection
    BMI_set_reg(BMI323_FEATURE_IO0, &config, 1);


    //
    // Configure the BMI interrupts
    //
    config = 0x0404;    // Enable Interrupt pins 1 and 2 
    config |= 0x0202;   // Enable open drain
    BMI_set_reg(BMI323_IO_INT_CTRL, &config, 1);

    BMI_get_reg(BMI323_INT_MAP2, &config, 1);
    config = (config & (~0b11)) | (0b01 << 0); // Map tap detection to INT1
    BMI_set_reg(BMI323_INT_MAP2, &config, 1);


    // Set up the interrupt
    gpio_set_irq_enabled_with_callback(IMU_INT1, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &IMU_INT_irq);
    gpio_set_irq_enabled_with_callback(IMU_INT2, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &IMU_INT_irq);
    
    //return data_in[0] & 0x01;
}

void IMU_Tick(){
    uint16_t reg_data[16];
    uint16_t data;

    // Check for interrupts
    if(IMU_INT1_flag){
        IMU_INT1_handle();
    }

    if(IMU_INT2_flag){
        IMU_INT2_handle();
    }

    // // Read accelerometer data
    // BMI_get_reg(BMI323_ACC_X, reg_data, 3);
    // printf("Acc X: %d, Y: %d, Z: %d\n", (int16_t)reg_data[0], (int16_t)reg_data[1], (int16_t)reg_data[2]);

    // // Read gyroscope data
    // BMI_get_reg(BMI323_GYR_X, reg_data, 3);
    // printf("Gyr X: %d, Y: %d, Z: %d\n", (int16_t)reg_data[0], (int16_t)reg_data[1], (int16_t)reg_data[2]);

    // // Read temperature
    // BMI_get_reg(BMI323_TEMP, reg_data, 1);
    // float temperature_value = (float)((((float)((int16_t)reg_data[0])) / 512.0) + 23.0);
    // printf("Temperature: %.2f C\n", temperature_value);

    //BMI_get_reg(BMI323_FEATURE_IO1, &data, 1);
    //printf("TICK: IMU IO1: 0x%04X\n", data);

    BMI_get_reg(BMI323_INT_STATUS_INT1, &data, 1);
    //printf("TICK: IMU INT1: 0x%04X\n", data);

    // Check for tap detection
    if(data & (0x1 << 8)){
        printf("TICK: Tap detected\n");

        BMI_get_reg(BMI323_FEATURE_EVENT_EXT, &data, 1);

        if(data & SINGLE_TAP_DETECT){
            printf("Single tap detected\n");
            activeLED_MODE = LED_MODE_RAINBOW;
        } else if(data & DOUBLE_TAP_DETECT){
            printf("Double tap detected\n");
            activeLED_MODE = LED_MODE_FADE;
        } else if(data & TRIPPLE_TAP_DETECT){
            printf("Tripple tap detected\n");
            activeLED_MODE = LED_MODE_ON;
        } else {
            printf("Unknown detected: 0x%04X\n", data);
        }
    }

}

void BMI_set_reg(uint8_t reg_addr, uint16_t *reg_data, uint32_t length){
    uint8_t data_out;

    BMI323_Select();

    spi_write_blocking(IMU_SPI_PORT, &reg_addr, 1);
    for(int i = 0; i < length; i++){
        data_out = reg_data[i] & 0xFF; // Lower byte
        spi_write_blocking(IMU_SPI_PORT, &data_out, 1);
        data_out = reg_data[i] >> 8; // Upper byte
        spi_write_blocking(IMU_SPI_PORT, &data_out, 1);
        //spi_write16_blocking(IMU_SPI_PORT, &reg_data[i], 1);
    }

    BMI323_Deselect();
}

void BMI_get_reg(uint8_t reg_addr, uint16_t *reg_data, uint32_t length){
    uint16_t buf;
    uint8_t addr = reg_addr | 0x80;
    uint8_t dummy;
    BMI323_Select();

    spi_write_blocking(IMU_SPI_PORT, &addr, 1);
    spi_read_blocking(IMU_SPI_PORT, 0, &dummy, 1);  // Dummy read
    for(int i = 0; i < length; i++){
        spi_read16_blocking(IMU_SPI_PORT, 0, &buf, 1);
        reg_data[i] = buf & 0xFF;
        reg_data[i] |= buf << 8;
    }

    BMI323_Deselect();
}

void BMI323_Select()
{
  gpio_put(IMU_SPI_CS, 0);
  sleep_us(10);
}

void BMI323_Deselect()
{
  gpio_put(IMU_SPI_CS, 1);
}

void BMI323_wait_Miso()
{
  while (gpio_get(IMU_SPI_MISO) > 0)
    ;
}

void BMI323_soft_reset()
{
  uint16_t data = 0xDEAF;
  BMI_set_reg(BMI323_CMD, &data, 1);
  sleep_ms(100);
  BMI323_Select();
  sleep_ms(1);
  BMI323_Deselect();
}
