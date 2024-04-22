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
#include "imu.h"
#include "serial.h"



int main() {
    int ret;
    static absolute_time_t last_time;
    static absolute_time_t next_time;
    uint16_t cnt = 0;

    // Enable UART over USB
    SERIAL_init();

    // Print the version
    printf("Frackstock v%d.%d\n", VERSION_MAJOR, VERSION_MINOR);

    // Initialize device
    if(DEV_init()) {
        printf("Device init failed\n");
    }

    // Initialize the GPIO pin
    gpio_init(BUILTIN_LED_PIN);
    gpio_set_dir(BUILTIN_LED_PIN, GPIO_OUT);

    // Initialize fractstock data
    FRACK_init();

    // Initialize IMU
    IMU_init();

    // Initialize the radio
    RADIO_init();

    // Initialize the LED Ring
    if(LED_Ring_init() != 0) {
        printf("LED Ring init failed\n");
    }

    printf("Init done\n");

    while (1) {
        
        // Send some data
        if(cnt % 1500 == 100){
            RADIO_send();
            LED_Ring_set_mode(LED_MODE_FILL_CIRCLE);
            SEG_set_mode(SEG_MODE_CUSTOM);
            SEG_write_number_hex(0xAA);
        }



        // Tasks
        if(cnt % 1 == 0)
        {
            RADIO_Tick();
        }

        if(cnt % 4 == 0)
        {
            LED_Ring_Tick();
        }

        if(cnt % 10 == 0)
        {
            IMU_Tick();
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