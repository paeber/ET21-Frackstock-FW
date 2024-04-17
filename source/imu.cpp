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


// Function prototypes
void BMI323_Select();
void BMI323_Deselect();
void BMI323_wait_Miso();



// Function definitions
void IMU_init(){

    // Initialize the SPI port
    spi_init(IMU_SPI_PORT, IMU_SPI_BAUDRATE);
    gpio_set_function(IMU_SPI_CLK, GPIO_FUNC_SPI);
    gpio_set_function(IMU_SPI_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(IMU_SPI_MISO, GPIO_FUNC_SPI);
    gpio_set_function(IMU_SPI_CS, GPIO_FUNC_SPI);

    spi_set_format(IMU_SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // Initialize the INT pins
    gpio_init(IMU_INT1);
    gpio_set_dir(IMU_INT1, GPIO_IN);
    gpio_pull_up(IMU_INT1);

    gpio_init(IMU_INT2);
    gpio_set_dir(IMU_INT2, GPIO_IN);
    gpio_pull_up(IMU_INT2);


    // Initialize the IMU
    uint8_t data_out[16];
    uint8_t data_in[16];

    // Read the chip ID
    BMI_get_reg(BMI323_CHIP_ID, data_in, 2);
    sleep_ms(100);
    BMI_get_reg(BMI323_CHIP_ID, data_in, 1);
    printf("Chip ID: 0x%02X\n", data_in[1]);

    // Read the error register
    BMI_get_reg(BMI323_ERR_REG, data_in, 1);
    printf("Error register: 0x%02X\n", data_in[1]);
    if(data_in[1] & 0x1) {
        printf("IMU Fatal Error\n");
    }
    
    //return data_in[0] & 0x01;
}

void IMU_Tick(){
    uint8_t data_out[16];
    uint8_t data_in[16];

    // Read accelerometer data
    BMI_get_reg(BMI323_ACC_X, data_in, 6);
    printf("Acc X: %d, Y: %d, Z: %d\n", data_in[0], data_in[2], data_in[4]);

    // Read gyroscope data
    BMI_get_reg(BMI323_GYR_X, data_in, 6);
    printf("Gyr X: %d, Y: %d, Z: %d\n", data_in[0], data_in[2], data_in[4]);




}

void BMI_set_reg(uint8_t reg_addr, uint8_t *reg_data, uint32_t length){
    BMI323_Select();

    spi_write_blocking(IMU_SPI_PORT, &reg_addr, 1);
    spi_write_blocking(IMU_SPI_PORT, reg_data, length);

    BMI323_Deselect();
}

void BMI_get_reg(uint8_t reg_addr, uint8_t *reg_data, uint32_t length){
    BMI323_Select();

    spi_write_blocking(IMU_SPI_PORT, &reg_addr, 1);
    spi_read_blocking(IMU_SPI_PORT, 0, reg_data, length + 1);

    BMI323_Deselect();
}

void BMI323_Select()
{
  gpio_put(IMU_SPI_CS, 0);
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