//
// Frackstock Module
//
// Author:      Pascal Eberhard
//

#include "frackstock.h"

#include <string.h>

tFrackStock frackstock;


void FRACK_init() {
    frackstock.id = 0;
    frackstock.beer = 0;
    strcpy(frackstock.abrev, "FRACK_21");
}


void FRACK_inc_beer() {
    frackstock.beer++;
}


uint8_t FRACK_get_beer() {
    return frackstock.beer;
}



