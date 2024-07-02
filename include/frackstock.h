//
//
//

#ifndef FRACKSTOCK_H
#define FRACKSTOCK_H


#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"


// Defines


// Data structures
typedef struct {
    uint8_t id;         ///< ID of the Frackstock
    uint8_t beer;       ///< Number of beers
    uint8_t buddy;      ///< Buddy ID
    uint8_t color[3];   ///< RGB color of the Frackstock
    char abrev[9];      ///< Abbreviation of the Owner
    uint8_t led_mode;   ///< LED mode (enum eLED_MODE)
} tFrackStock;


// Global variables
extern tFrackStock frackstock;


// Function prototypes
void FRACK_init();
void FRACK_inc_beer();
uint8_t FRACK_get_beer();
uint8_t FRACK_get_id();
uint8_t FRACK_get_buddy();
uint8_t FRACK_get_led_mode();


#endif // FRACKSTOCK_H