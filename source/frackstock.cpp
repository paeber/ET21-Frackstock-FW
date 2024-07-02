//
// Frackstock Module
//
// Author:      Pascal Eberhard
//

#include "frackstock.h"
#include "device.h"
#include "led_ring.h"
#include <string.h>

tFrackStock frackstock = {
    .id = 0xfe,
    .beer = 0,
    .buddy = 0,
    .color = {0, 255, 0},
    .abrev = "FRK_ET21",
    .led_mode = (uint8_t)LED_MODE_WALK
};


void FRACK_init() {
    DEV_get_frack_data(&frackstock);
    LED_Ring_set_color(frackstock.color[0], frackstock.color[1], frackstock.color[2]);
}


void FRACK_inc_beer() {
    frackstock.beer++;
    DEV_set_frack_data(&frackstock);
}


uint8_t FRACK_get_beer() {
    return frackstock.beer;
}

uint8_t FRACK_get_id() {
    return frackstock.id;
}

uint8_t FRACK_get_buddy() {
    return frackstock.buddy;
}

uint8_t FRACK_get_led_mode() {
    return frackstock.led_mode;
}

// EOF
