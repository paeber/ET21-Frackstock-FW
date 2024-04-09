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
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#include <PicoLed.hpp>
#include "led_ring.h"



int main() {
    int ret;
    uint16_t cnt = 0;

    // Enable UART over USB
    stdio_init_all();

    // Initialize the GPIO pin
    gpio_init(BUILTIN_LED_PIN);
    gpio_set_dir(BUILTIN_LED_PIN, GPIO_OUT);

    // Initialize the LED Ring
    if(LED_Ring_init()) {
        printf("LED Ring init failed\n");
    }


    // 1. Initialize LED strip
    printf("0. Initialize LED strip\n");
    auto ledStrip = PicoLed::addLeds<PicoLed::WS2812B>(pio0, 0, LED_RING_PIN, LED_RING_COUNT, PicoLed::FORMAT_GRB);
    ledStrip.setBrightness(64);

    // 2. Set all LEDs to green!
    ledStrip.fill( PicoLed::RGB(0, 255, 0) );
    ledStrip.show();
    sleep_ms(500);

    // 3. Set all LEDs to blue!
    ledStrip.fill( PicoLed::RGB(0, 0, 255) );
    ledStrip.show();
    sleep_ms(500);

    // 4. Set all LEDs off!
    ledStrip.fill( PicoLed::RGB(0, 0, 0) );
    ledStrip.show();
    sleep_ms(500);


    printf("Init done\n");
    sleep_ms(500);

    while (1) {

        // Toggle LED
        gpio_get(BUILTIN_LED_PIN) ? gpio_put(BUILTIN_LED_PIN, 0) : gpio_put(BUILTIN_LED_PIN, 1);


        SEG_write_number_hex(cnt / 10);

        ledStrip.fillRainbow(0 + cnt, 255 / LED_RING_COUNT);
        ledStrip.show();

        cnt++;

        sleep_ms(10);
    }

    return 0;
}