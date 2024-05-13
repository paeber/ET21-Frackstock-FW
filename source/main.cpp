//
// Frackstock Pico Firmware
//
// Author:      Pascal Eberhard
//

#include "main.h"

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/binary_info.h"

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/watchdog.h"

#include "device.h"
#include "frackstock.h"
#include "led_ring.h"
#include "radio.h"
#include "imu.h"
#include "serial.h"
#include "interrupts.h"
#include "gpio.h"

#define RESET_SLEEP_TIMER 1000

int main() {
    int ret;
    static absolute_time_t last_time;
    static absolute_time_t next_time;
    uint16_t cnt = 0;
    static uint16_t sleep_timer = RESET_SLEEP_TIMER;

    // Enable UART over USB
    SERIAL_init();

    if (watchdog_caused_reboot()) {
        SERIAL_printf("Rebooted by Watchdog!\n");
    } else {
        SERIAL_printf("Clean boot\n");
    }

    // Print the version
    SERIAL_printf("Frackstock v%d.%d\n", VERSION_MAJOR, VERSION_MINOR);

    // Initialize device
    if(DEV_init()) {
        SERIAL_printf("Device init failed\n");
    }

    // Initialize the GPIO pin
    GPIO_init();

    // Initialize fractstock data
    FRACK_init();

    // Initialize IMU
    IMU_init();

    // Initialize the radio
    RADIO_init();

    // Initialize the LED Ring
    if(LED_Ring_init() != 0) {
        SERIAL_printf("LED Ring init failed\n");
    }

    SERIAL_printf("Init done\n");
    SERIAL_EnableMessages(SERIAL_MESSAGES_ENABLED);

    //SEG_add_to_buffer(VERSION_MAJOR << 4 | VERSION_MINOR, SEG_NUMBER_MODE_HEX_DOT);
    SEG_add_to_buffer(frackstock.id, SEG_NUMBER_MODE_HEX);
    SEG_set_mode(SEG_MODE_BUFFER);

    // Enable the watchdog, requiring the watchdog to be updated every 1200ms or the chip will reboot
    watchdog_enable(1200, 1);

    while (1) {
        
        /*
        // Send some data
        if(cnt % 2000 == 1000){
            RADIO_send(BROADCAST_ADDRESS);
            if(!GPIO_Button_getStates()){
                LED_Ring_set_color(frackstock.color[0], frackstock.color[1], frackstock.color[2]);
                LED_Ring_set_mode(LED_MODE_FILL_CIRCLE);
                SEG_set_mode(SEG_MODE_CUSTOM);
                SEG_set_segments(LEFT_DIGIT,  SEG_CHAR_t);
                SEG_set_segments(RIGHT_DIGIT, SEG_CHAR_r);
            }
        }
        */

        #ifdef MAX_POWER_TEST
        if (cnt % 4 == 0) 
        {
            LED_Ring_set_color(255, 255, 255);
            LED_Ring_set_mode(LED_MODE_ON);
            SEG_set_mode(SEG_MODE_ON);
        }
        #endif

        // Check for events
        if(packetWaiting || IMU_INT1_flag || IMU_INT2_flag || GPIO_INT_FLAG || LED_Ring_isPending()){
            sleep_timer = RESET_SLEEP_TIMER;
        } else {
            if(sleep_timer > 0){
                sleep_timer--;
            }
        }

        // Tasks
        if(1){
            if(cnt % 2000 == 100 || packetWaiting){
                RADIO_Tick();
            }

            if(cnt % 2000 == 0 || IMU_INT1_flag || IMU_INT2_flag) {
                IMU_Tick();
            }

            if(LED_Ring_isPending()){
                if(cnt % 4 == 2)
                {
                    LED_Ring_Tick();
                }
                
                if(cnt % 4 == 3)
                {
                SEG_Tick();
                }
            }
            

            if (cnt % 200 == 0)     // Heartbeat
            {
                GPIO_LED_set(1);
            }
            else if (cnt % 200 == 10)
            {
                GPIO_LED_set(0);
            }
            
            if (cnt % 30 == 0){
                GPIO_Button_Tick();
            }


            SERIAL_Tick();

            cnt++;
            last_time = delayed_by_ms(last_time, 10); // Sleep up to 10ms
            sleep_until(last_time);
        } else {
            // Sleep for 1 second
            last_time = get_absolute_time();
            next_time = delayed_by_ms(last_time, 10);
            sleep_until(next_time);

        }
        watchdog_update();


    }

    return 0;
}