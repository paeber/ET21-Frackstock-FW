#include "interrupts.h"
#include "main.h"
#include "radio.h"
#include "led_ring.h"
#include "hardware/gpio.h"
#include "imu.h"
#include "serial.h"
#include "gpio.h"

/**
 * @brief Handle interrupts from different GPIOs.
 * 
 * This is the callback for all GPIO driven interrupts. It sets the flags for the respective interrupt source.
 * 
 * @note Multiple callbacks did not work, so all interrupts are handled in this function. 
 */
void handle_Interrupts(uint gpio, uint32_t events)
{
    //SEG_set_mode(SEG_MODE_CUSTOM);
    //SEG_write_number_hex(0x17);     // Show that interrupt was triggered 17 = IT, for degug purposes
    if(gpio == IMU_INT1){
        IMU_INT1_flag = true;
        SERIAL_printf("->IMU INT1\n");
    } else if(gpio == IMU_INT2){
        IMU_INT2_flag = true;
        SERIAL_printf("->IMU INT2\n");
    } else if (gpio == RADIO_GDO1){
        packetWaiting = true;
        SERIAL_printf("->RADIO INT\n");
    } else if (gpio == BUTTON_PIN){
        GPIO_INT_FLAG = true;
        SERIAL_printf("->BUTTON INT\n");
    }
    
}
