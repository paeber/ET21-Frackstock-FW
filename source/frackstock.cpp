//
// Frackstock Module
//
// Author:      Pascal Eberhard
//

#include "frackstock.h"
#include "device.h"

#include <string.h>

tFrackStock frackstock = {
    .id = 0xfe,
    .beer = 0,
    .abrev = "FRK_ET21"
};


void FRACK_init() {
    DEV_get_frack_data(&frackstock);
}


void FRACK_inc_beer() {
    frackstock.beer++;
}


uint8_t FRACK_get_beer() {
    return frackstock.beer;
}



