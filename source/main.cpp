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

#include "device.h"
#include "frackstock.h"
#include "led_ring.h"
#include "radio.h"
#include "serial.h"

#define SENDER

int main() {
    int ret;
    static absolute_time_t last_time;
    static absolute_time_t next_time;
    uint16_t cnt = 0;

    // Enable UART over USB
    SERIAL_init();

    sleep_ms(1000);

    // Print the version
    printf("Frackstock v%d.%d\n", VERSION_MAJOR, VERSION_MINOR);

    // Initialize device
    if(DEV_init()) {
        printf("Device init failed\n");
    }

    // Initialize the GPIO pin
    gpio_init(BUILTIN_LED_PIN);
    gpio_set_dir(BUILTIN_LED_PIN, GPIO_OUT);

    // Initialize the LED Ring
    if(LED_Ring_init()) {
        printf("LED Ring init failed\n");
    }

    // Initialize fractstock data
    FRACK_init();

    // Initialize the radio
    RADIO_init();

    printf("Init done\n");
    sleep_ms(500);

    while (1) {

        // Set new mode segemnts
        /*if(cnt % 500 == 0){
            activeSEG_MODE = (eSEG_MODE)((activeSEG_MODE + 1) % 5);
        }*/
        

        // Set new mode LED Ring
        if(cnt % 1000 == 100){
            //activeLED_MODE = (eLED_MODE)((activeLED_MODE + 1) % 7);
            //activeLED_MODE = LED_MODE_RAINBOW;
        }
        
        #ifdef SENDER
        // Send some data
        if(cnt % 2000 == 100){
            RADIO_send();
            activeLED_MODE = LED_MODE_BLINK;
            activeSEG_MODE = SEG_MODE_CUSTOM;
            SEG_write_number_hex(0xAA);
        }
        #endif

        if(cnt % 500 == 0){
            activeLED_MODE = LED_MODE_OFF;
            activeSEG_MODE = SEG_MODE_CUSTOM;
            SEG_write_number_hex(0xff);
        }


        // Tasks
        if(cnt % 1 == 0)
        {
            RADIO_Tick();
        }

        if(cnt % 2 == 0)
        {
            LED_Ring_Tick();
        }
        
        if(cnt % 4 == 0)
        {
           SEG_Tick();
        }
        

        if (cnt % 100 == 0)     // Heartbeat
        {
            DEV_LED_toggle();
        }
        

        SERIAL_Tick();
        

        cnt++;

        last_time = delayed_by_ms(last_time, 10); // Sleep up to 10ms
        sleep_until(last_time);

    }

    return 0;
}