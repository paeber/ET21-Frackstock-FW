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

#include "hardware/spi.h"
#include "hardware/gpio.h"

#include "led_ring.h"
#include "serial.h"
#include "frackstock.h"
#include "radio.h"
#include "interrupts.h"

// Function prototypes
void BMI323_Select();
void BMI323_Deselect();
void BMI323_wait_Miso();
void BMI323_soft_reset();
uint8_t BMI323_com_check();
void BMI323_Dummy_Read();

void IMU_INT_irq(uint gpio, uint32_t events);
void IMU_INT1_handle();
void IMU_INT2_handle();

// Global variables
bool IMU_INT1_flag = false;
bool IMU_INT2_flag = false;
IMU_State_t IMU_State = IMU_STATE_UNKNOWN;


/**
 * @brief Handles the interrupt from IMU INT1 pin.
 * 
 * This function is called when an interrupt is triggered on the IMU INT1 pin. It checks for tap detection
 * and sets the active LED mode based on the type of tap detected.
 */
void IMU_INT1_handle(){
    uint16_t interrupt, data;
    IMU_INT1_flag = false;
    BMI_get_reg(BMI323_INT_STATUS_INT1, &interrupt, 1);
    SERIAL_printf("IMU INT1: 0x%04X\n", interrupt);

    // Check for tap detection
    if(interrupt & (0x1 << 8)){
        BMI_get_reg(BMI323_FEATURE_EVENT_EXT, &data, 1);

        if(data & SINGLE_TAP_DETECT){
            SERIAL_printf("Single tap detected\n");
            LED_Ring_set_mode(LED_MODE_FADE);
            SEG_add_to_buffer(FRACK_get_beer(), SEG_NUMBER_MODE_DEC);
            SEG_set_mode(SEG_MODE_BUFFER);
        } else if(data & DOUBLE_TAP_DETECT){
            SERIAL_printf("Double tap detected\n");
            LED_Ring_set_mode(LED_MODE_FILL_CIRCLE);
            SEG_set_mode(SEG_MODE_CUSTOM);
            SEG_set_segments(LEFT_DIGIT, SEG_CHAR_t);
            SEG_set_segments(RIGHT_DIGIT, SEG_CHAR_r);
            RADIO_send(BROADCAST_ADDRESS);
        } else if(data & TRIPPLE_TAP_DETECT){
            SERIAL_printf("Tripple tap detected\n");
            LED_Ring_set_mode(LED_MODE_RGB_WALK);
            SEG_add_to_buffer(frackstock.buddy, SEG_NUMBER_MODE_HEX);
            SEG_set_mode(SEG_MODE_BUFFER);
            RADIO_send(frackstock.buddy);
        } else {
            SERIAL_printf("Unknown detected: 0x%04X\n", data);
        }
    }
}


/**
 * @brief Handles the IMU INT2 interrupt.
 * 
 * This function is called when the IMU INT2 interrupt is triggered. It reads the interrupt status
 * register of the BMI323 sensor and prints the value. It also checks for tap detection and prints a
 * message if a tap is detected.
 */
void IMU_INT2_handle(){
    uint16_t interrupt;
    IMU_INT2_flag = false;
    BMI_get_reg(BMI323_INT_STATUS_INT2, &interrupt, 1);
    SERIAL_printf("IMU INT2: 0x%04X\n", interrupt);

    // Check for tap detection
    if(interrupt & (0x1 << 8)){
        SERIAL_printf("Tap detected\n");

    }
}


// Function definitions
/**
 * @brief Initializes the IMU module.
 * 
 * This function initializes the IMU module by configuring the SPI port, INT pins, and the IMU itself.
 * It performs a soft reset of the IMU, wakes it up, reads the chip ID, error register, and status register,
 * enables the feature engine, enables the accelerometer and gyroscope, configures features such as tap detection,
 * and sets up the BMI interrupts.
 * 
 * @note This function assumes that the necessary GPIO pins and SPI port have been initialized before calling this function.
 */
void IMU_init(){
    uint16_t ret = 0;

    // Initialize the SPI port
    spi_init(IMU_SPI_PORT, IMU_SPI_BAUDRATE);
    gpio_set_function(IMU_SPI_CLK, GPIO_FUNC_SPI);
    gpio_set_function(IMU_SPI_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(IMU_SPI_MISO, GPIO_FUNC_SPI);
    //gpio_set_function(IMU_SPI_CS, GPIO_FUNC_SPI);

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
    sleep_ms(150);
    BMI323_soft_reset();
    sleep_ms(150);
    
    IMU_Startup();
    IMU_Enable_Feature_Engine();
    IMU_Configure_Sensor();
    IMU_Configure_Features();
    IMU_Configure_IT();
    IMU_State = IMU_STATE_CONFIGURED;
}


/**
 * @brief Initializes the IMU and checks its status.
 * @return 0 if the IMU is functioning properly, 1 if the IMU is not detected, 2 if there is an IMU power error.
 */
uint16_t IMU_Startup(){
    uint16_t config = 0x0000;
    uint16_t data_in[16];
    uint8_t addr = 0 | 0x80;

    // Dummy write/read to wake up the IMU
    spi_write_blocking(IMU_SPI_PORT, &addr, 1);
    spi_read16_blocking(IMU_SPI_PORT, 0, data_in, 1);

    sleep_ms(350);

    // Read the chip ID
    if(BMI323_com_check()){
        SERIAL_printf("IMU not detected\n");
        BMI_get_reg(BMI323_CHIP_ID, data_in, 1);
        SERIAL_printf("Chip ID: 0x%04X\n", data_in[0]);
        return 1;
    }

    // Read the error register
    BMI_get_reg(BMI323_ERR_REG, data_in, 1);
    SERIAL_printf("Error register: 0x%04X\n", data_in[0]);
    if(data_in[0] & 0x0001) {
        SERIAL_printf("IMU Power Error\n");
        return 2;
    }

    // Read the status register
    BMI_get_reg(BMI323_STATUS, data_in, 1);
    SERIAL_printf("Status register: 0x%02X\n", data_in[0]);
    if(data_in[0] & 0x1) {
        SERIAL_printf("IMU OK\n");
    }
    return 0;
}


/**
 * @brief Enables the feature engine of the IMU.
 * 
 * This function enables the feature engine of the IMU by configuring the necessary registers.
 * It waits for the feature engine to be enabled and returns an error if it fails.
 * 
 * @return 0 if the feature engine is successfully enabled, 1 otherwise.
 */
uint16_t IMU_Enable_Feature_Engine(){
    uint16_t config = 0x0000;

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
    if((config & 0x0f) != 0x01){
        SERIAL_printf("\nFeature engine not enabled\n");
        return 1;
    }

    return 0;
}


/**
 * Configures the IMU sensor by enabling the accelerometer and gyroscope.
 * Sets the desired configuration values for each sensor.
 * Reads and prints the configuration values for verification.
 *
 * @return 0 on success.
 */
uint16_t IMU_Configure_Sensor(){
    uint16_t config = 0x0000;
    uint16_t data_in[16];

    // Enable the accelerometer
    uint16_t acc_conf = 0x0000; // Default value
    acc_conf |= (0b100 << 12);  // Normal mode
    acc_conf |= (0b010 << 8);   // Averageing 4 samples
    acc_conf |= (0b0 << 7);     // Cut-off acc_odr/2 
    acc_conf |= ACC_RANGE_2G;
    //acc_conf |= ACC_RANGE_16G;
    acc_conf |= ACC_ODR_200;
    BMI_set_reg(BMI323_ACC_CONF, &acc_conf, 1);
    BMI_get_reg(BMI323_ACC_X, data_in, 3);

    sleep_ms(10);
    BMI_get_reg(BMI323_ACC_CONF, &config, 1);
    SERIAL_printf("ACC_CONF: 0x%04X\n", config);

    // Enable the gyroscope
    uint16_t gyr_conf = 0x0000; // Default value
    gyr_conf |= (0b100 << 12);  // Normal mode
    gyr_conf |= (0b000 << 8);   // Averageing off
    gyr_conf |= GYR_BW_ODR2;
    gyr_conf |= GYR_RANGE_2000;
    gyr_conf |= GYR_ODR_200;
    BMI_set_reg(BMI323_GYR_CONF, &gyr_conf, 1);
    BMI_get_reg(BMI323_GYR_X, data_in, 3);

    sleep_ms(10);
    BMI_get_reg(BMI323_GYR_CONF, &config, 1);
    SERIAL_printf("GYR_CONF: 0x%04X\n", config);

    return 0;
}


/**
 * @brief Configures the features of the IMU.
 * 
 * This function configures the features of the IMU, such as tap detection.
 * It sets the necessary registers and applies the required settings for tap detection.
 * 
 * @return The function does not return any value.
 */
uint16_t IMU_Configure_Features(){ 
    uint16_t config = 0x0000;

    // Configure Tab detection
    uint16_t fAddr = (uint16_t)FEATURE_TAP_1;
    //config = 0x0074;    // Set x axis
    config = 0x00B4;    // Set x axis & robust mode
    BMI_set_reg(BMI323_FEATURE_DATA_ADDR, &fAddr, 1);
    BMI_set_reg(BMI323_FEATURE_DATA_TX, &config, 1);

    sleep_ms(40);

    fAddr = (uint16_t)FEATURE_TAP_2;
    config = 0;
    config |= 200;         // Tap threshold
    config |= (0b010000 << 10);     // Tap duration
    BMI_set_reg(BMI323_FEATURE_DATA_ADDR, &fAddr, 1);
    BMI_set_reg(BMI323_FEATURE_DATA_TX, &config, 1);

    sleep_ms(10);

    BMI_get_reg(BMI323_FEATURE_IO0, &config, 1);
    config |= SINGLE_TAP_EN;    // Enable single tap detection
    config |= DOUBLE_TAP_EN;    // Enable double tap detection
    config |= TRIPPLE_TAP_EN;   // Enable tripple tap detection
    BMI_set_reg(BMI323_FEATURE_IO0, &config, 1);

    config = 0x0001;
    BMI_set_reg(BMI323_FEATURE_IO_STATUS, &config, 1);  // Apply interrupt settings

    sleep_ms(10);
    BMI_get_reg(BMI323_FEATURE_IO0, &config, 1);
    SERIAL_printf("FEATURE_IO0: 0x%04X\n", config);

    return 0;
}


/**
 * @brief Configures the BMI interrupt for the IMU.
 *
 * This function configures the BMI interrupt for the IMU by setting the interrupt mapping,
 * interrupt polarity, and enabling the interrupt output. It also sets up the GPIO interrupts
 * for the IMU.
 *
 * @return 0 on success.
 */
uint16_t IMU_Configure_IT(){
    uint16_t config = 0x0000;

    BMI_get_reg(BMI323_INT_MAP2, &config, 1);
    config = (config & (uint16_t)(~0b11)) | (0b01 << 0); // Map tap detection to INT2
    BMI_set_reg(BMI323_INT_MAP2, &config, 1);

    BMI_get_reg(BMI323_INT_MAP2, &config, 1);
    SERIAL_printf("INT_MAP2: 0x%04X\n", config);

    config = 0x0000;
    config |= (0b0 << 0);   // INT1 active low
    config |= (0b1 << 1);   // INT1 open drain
    config |= (0b1 << 2);   // INT1 enable output
    config |= (0b0 << 8);   // INT2 active low
    config |= (0b1 << 9);   // INT2 open drain
    config |= (0b1 << 10);  // INT2 enable output
    BMI_set_reg(BMI323_IO_INT_CTRL, &config, 1);

    BMI_get_reg(BMI323_IO_INT_CTRL, &config, 1);
    SERIAL_printf("IO_INT_CTRL: 0x%04X\n", config);

    // Set up the interrupt
    gpio_set_irq_enabled_with_callback(IMU_INT1, GPIO_IRQ_EDGE_FALL, true, &handle_Interrupts);
    gpio_set_irq_enabled_with_callback(IMU_INT2, GPIO_IRQ_EDGE_FALL, true, &handle_Interrupts);

    return 0;
}


/**
 * @brief Handles the tick event for the IMU (Inertial Measurement Unit).
 * 
 * This function is called periodically to check for interrupts and process tap detection events.
 * It checks for interrupts on IMU_INT1 and IMU_INT2 pins and calls the corresponding interrupt handlers.
 * It also checks for tap detection events and sets the active LED mode based on the detected tap type.
 * 
 * @note This function assumes that the BMI323 sensor is used for tap detection and the necessary registers are configured.
 */
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

    /*
    switch(IMU_State){
        case IMU_STATE_UNKNOWN:
            BMI323_soft_reset();
            IMU_State = IMU_STATE_REBOOT;
            break;

        case IMU_STATE_REBOOT:
            if(IMU_Startup()){
                IMU_State = IMU_STATE_ERROR;
            } else {
                IMU_State = IMU_STATE_COM_OK;
            }
            break;

        case IMU_STATE_COM_OK:
            if(IMU_Enable_Feature_Engine()){
                IMU_State = IMU_STATE_ERROR;
            } else {
                IMU_State = IMU_STATE_FEATURE_EN;
            }
            break;

        case IMU_STATE_FEATURE_EN:
            if(IMU_Configure_Sensor()){
                IMU_State = IMU_STATE_ERROR;
            } else {
                IMU_State = IMU_STATE_SENSOR_EN;
            }
            break;

        case IMU_STATE_SENSOR_EN:
            if(IMU_Configure_Features()){
                IMU_State = IMU_STATE_ERROR;
            } else {
                IMU_State = IMU_STATE_FEATURES_CONFIGURED;
            }
            break;
        
        case IMU_STATE_FEATURES_CONFIGURED:
            if(IMU_Configure_IT()){
                IMU_State = IMU_STATE_ERROR;
            } else {
                IMU_State = IMU_STATE_IT_EN;
            }
            break;

        case IMU_STATE_IT_EN:
            IMU_State = IMU_STATE_CONFIGURED;
            break;

        case IMU_STATE_ERROR:
            SERIAL_printf("IMU error\n");
            IMU_State = IMU_STATE_UNKNOWN;
            break;

    }
    */


    /*BMI_get_reg(BMI323_FEATURE_IO1, &data, 1);
    if(data & 0x0f == 0){
        SERIAL_printf("Feature engine not ready\n");
        IMU_State = IMU_STATE_UNKNOWN;
        return;
    }*/

}


/**
 * @brief Sets the register values of the BMI sensor.
 * 
 * This function is used to set the register values of the BMI sensor
 * using SPI communication. It takes the register address, an array of
 * register data, and the length of the array as parameters. The function
 * selects the BMI sensor, writes the register address, and then writes
 * the register data to the sensor. Finally, it deselects the BMI sensor.
 * 
 * @param reg_addr The register address to write to.
 * @param reg_data An array of register data to write.
 * @param length The length of the reg_data array.
 */
void BMI_set_reg(uint8_t reg_addr, uint16_t *reg_data, uint32_t length){
    uint8_t data_out;

    BMI323_Select();

    spi_write_blocking(IMU_SPI_PORT, &reg_addr, 1);
    for(int i = 0; i < length; i++){
        data_out = reg_data[i] & 0xFF; // Lower byte
        spi_write_blocking(IMU_SPI_PORT, &data_out, 1);
        data_out = reg_data[i] >> 8; // Upper byte
        spi_write_blocking(IMU_SPI_PORT, &data_out, 1);
    }

    BMI323_Deselect();
}


/**
 * @brief Reads data from a register in the BMI sensor.
 * 
 * This function reads data from a specified register in the BMI sensor using SPI communication.
 * The register address, data, and length of data to be read are provided as parameters.
 * The function first selects the BMI sensor, then sends the register address to read from.
 * It performs a dummy read to clear the receive buffer, and then reads the data from the register.
 * The data is stored in the provided array of 16-bit integers.
 * 
 * @param reg_addr The address of the register to read from.
 * @param reg_data Pointer to the array where the read data will be stored.
 * @param length The number of data elements to read.
 */
void BMI_get_reg(uint8_t reg_addr, uint16_t *reg_data, uint32_t length){
    uint8_t addr = reg_addr | 0x80;
    uint8_t dummy;
    uint8_t data = 0;
    BMI323_Select();

    spi_write_blocking(IMU_SPI_PORT, &addr, 1);
    spi_read_blocking(IMU_SPI_PORT, 0, &dummy, 1);  // Dummy read
    for(int i = 0; i < length; i++){
        spi_read_blocking(IMU_SPI_PORT, 0, &data, 1);
        reg_data[i] = 0x00ff & data;
        spi_read_blocking(IMU_SPI_PORT, 0, &data, 1);
        reg_data[i] |= (data << 8) & 0xff00;
    }

    BMI323_Deselect();
}


/**
 * @brief Selects the BMI323 sensor.
 * 
 * This function sets the IMU_SPI_CS pin to low, effectively selecting the BMI323 sensor.
 * It also introduces a delay of 6 microseconds to ensure proper communication.
 */
void BMI323_Select()
{
  gpio_put(IMU_SPI_CS, 0);
  sleep_us(6);
}


/**
 * @brief Deselects the BMI323 sensor.
 * 
 * This function deselects the BMI323 sensor by setting the IMU_SPI_CS pin to high.
 * It also introduces a delay of 6 microseconds using the `sleep_us` function.
 */
void BMI323_Deselect()
{
  sleep_us(6);
  gpio_put(IMU_SPI_CS, 1);
}


/**
 * @brief Performs a soft reset on the BMI323 sensor.
 * 
 * This function sends a command to the BMI323 sensor to perform a soft reset.
 * After sending the command, it waits for 100 milliseconds before selecting and
 * deselecting the sensor to complete the reset process.
 */
void BMI323_soft_reset()
{
  uint16_t data = 0xDEAF;
  BMI_set_reg(BMI323_CMD, &data, 1);
  sleep_ms(100);
  BMI323_Select();
  sleep_ms(1);
  BMI323_Deselect();
}


/**
 * @brief Checks the communication with the BMI323 chip.
 * 
 * This function reads the chip ID register of the BMI323 chip and compares it with the expected value.
 * If the chip ID matches the expected value, it indicates successful communication with the chip.
 * 
 * @return 0 if the communication is successful, 1 otherwise.
 */
uint8_t BMI323_com_check(){
    uint16_t data = 0;
    BMI_get_reg(BMI323_CHIP_ID, &data, 1);
    if((data & 0x00ff) == BMI323_CHIP_ID_VAL){
        return (0u);
    }
    return (1u);
}


/**
 * @brief Performs a dummy write/read operation to wake up the IMU.
 *
 * This function sends a dummy write command followed by a read command to the IMU in order to wake it up.
 * It uses the SPI interface to communicate with the IMU.
 */
void BMI323_Dummy_Read(){
    uint16_t data_in[16];
    uint8_t addr = 0 | 0x80;

    // Dummy write/read to wake up the IMU
    spi_write_blocking(IMU_SPI_PORT, &addr, 1);
    spi_read16_blocking(IMU_SPI_PORT, 0, data_in, 1);
}


/**
 * @brief Enum representing the state of the IMU (Inertial Measurement Unit).
 */
IMU_State_t IMU_get_state(){
    return IMU_State;
}