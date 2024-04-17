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
void BMI323_soft_reset();
void readAllAccel();


// Function definitions
void IMU_init(){

    // Initialize the SPI port
    spi_init(IMU_SPI_PORT, IMU_SPI_BAUDRATE);
    gpio_set_function(IMU_SPI_CLK, GPIO_FUNC_SPI);
    gpio_set_function(IMU_SPI_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(IMU_SPI_MISO, GPIO_FUNC_SPI);
    //gpio_set_function(IMU_SPI_CS, GPIO_FUNC_SPI);

    spi_set_format(IMU_SPI_PORT, 16, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

    gpio_init(IMU_SPI_CS);
    gpio_set_dir(IMU_SPI_CS, GPIO_OUT);
    gpio_put(IMU_SPI_CS, 1);

    // Initialize the INT pins
    gpio_init(IMU_INT1);
    gpio_set_dir(IMU_INT1, GPIO_IN);
    gpio_pull_up(IMU_INT1);

    gpio_init(IMU_INT2);
    gpio_set_dir(IMU_INT2, GPIO_IN);
    gpio_pull_up(IMU_INT2);

    // Soft reset the IMU
    //BMI323_soft_reset();

    // Initialize the IMU
    uint16_t data_in[16];

    // Read the chip ID
    BMI_get_reg(BMI323_CHIP_ID, data_in, 1);
    sleep_ms(100);
    BMI_get_reg(BMI323_CHIP_ID, data_in, 1);
    printf("Chip ID: 0x%02X\n", data_in[0]);

    // Read the error register
    BMI_get_reg(BMI323_ERR_REG, data_in, 1);
    printf("Error register: 0x%02X\n", data_in[0]);
    if(data_in[0] & 0x1) {
        printf("IMU Fatal Error\n");
    }
    
    //return data_in[0] & 0x01;
}

void IMU_Tick(){
    uint16_t reg_data[16];

    /*
    // Read accelerometer data
    BMI_get_reg(BMI323_ACC_X, reg_data, 3);
    printf("Acc X: %d, Y: %d, Z: %d\n", reg_data[0], reg_data[1], reg_data[2]);

    // Read gyroscope data
    BMI_get_reg(BMI323_GYR_X, reg_data, 3);
    printf("Gyr X: %d, Y: %d, Z: %d\n", reg_data[0], reg_data[1], reg_data[2]);
  */

    readAllAccel();



}

void BMI_set_reg(uint8_t reg_addr, uint16_t *reg_data, uint32_t length){
    uint8_t data_out;

    BMI323_Select();

    spi_write_blocking(IMU_SPI_PORT, &reg_addr, 1);
    for(int i = 0; i < length; i++){
        //data_out = reg_data[i] & 0xFF; // Lower byte
        //spi_write_blocking(IMU_SPI_PORT, &data_out, 1);
        //data_out = reg_data[i] >> 8; // Upper byte
        //spi_write_blocking(IMU_SPI_PORT, &data_out, 1);
        spi_write16_blocking(IMU_SPI_PORT, &reg_data[i], 1);
    }

    BMI323_Deselect();
}

void BMI_get_reg(uint8_t reg_addr, uint16_t *reg_data, uint32_t length){
    uint16_t buf;
    uint8_t dummy = reg_addr | 0x80;
    BMI323_Select();

    spi_write_blocking(IMU_SPI_PORT, &dummy, 1);
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
}

void readAllAccel() {
  uint8_t address = 0x03;
  uint8_t txBuf[1] = {address | 0x80}; // read operation, MSB is 1
  uint8_t rxBuf[20] = {0}; // initialize to 0
  //HAL_GPIO_WritePin(GPIOA, SPI_CS_Pin, GPIO_PIN_RESET); // make chip select low to enable transmission
  BMI323_Select();
  //HAL_SPI_TransmitReceive(&hspi1, txBuf, rxBuf, 1, 10); // send address and receive data
  spi_write_read_blocking(IMU_SPI_PORT, txBuf, rxBuf, 1);
  //HAL_SPI_TransmitReceive(&hspi1, &txBuf[1], &rxBuf[1], 19, 10); // receive the rest of the data
  spi_write_read_blocking(IMU_SPI_PORT, &txBuf[1], &rxBuf[1], 19);
  //HAL_GPIO_WritePin(GPIOA, SPI_CS_Pin, GPIO_PIN_SET); // make chip select high at the end of transmission
  BMI323_Deselect();

  //Offset = 2 because the 2 first bytes are dummy (useless)
  int offset = 2;
  uint16_t x, y, z, gyr_x, gyr_y, gyr_z, temperature;
  x = ((rxBuf[offset + 1] << 8 | (uint16_t)rxBuf[offset + 0])); //0x03
  y = ((rxBuf[offset + 3] << 8 | (uint16_t)rxBuf[offset + 2])); //0x04
  z = ((rxBuf[offset + 5] << 8 | (uint16_t)rxBuf[offset + 4])); //0x05
  gyr_x = (rxBuf[offset + 7] << 8 | (uint16_t)rxBuf[offset + 6]); //0x06
  gyr_y = (rxBuf[offset + 9] << 8 | (uint16_t)rxBuf[offset + 8]); //0x07
  gyr_z = (rxBuf[offset + 11] << 8 | (uint16_t)rxBuf[offset + 10]); //0x08
  temperature = (rxBuf[offset + 13] << 8 | (uint16_t)rxBuf[offset + 12]); //0x09

  printf("Accel: X: %d, Y: %d, Z: %d\n", x, y, z);
  printf("Gyro: X: %d, Y: %d, Z: %d\n", gyr_x, gyr_y, gyr_z);
  printf("Temperature: %d\n", temperature);
}