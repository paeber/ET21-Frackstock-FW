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
#include "led_ring.h"




int main() {
    int ret;
    uint16_t cnt = 0;

    // Enable UART over USB
    stdio_init_all();

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


    printf("Init done\n");
    sleep_ms(500);

    while (1) {

        // Toggle LED
        gpio_get(BUILTIN_LED_PIN) ? gpio_put(BUILTIN_LED_PIN, 0) : gpio_put(BUILTIN_LED_PIN, 1);


        SEG_Tick();

        LED_Ring_Tick();

        cnt++;

        sleep_ms(10);
    }

    return 0;
}