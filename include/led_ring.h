// 
// 
//

// Segments Layout
//    A
//  F   B
//    G
//  E   C
//    D   DP

#ifndef __LED_RING_H
#define __LED_RING_H

#include <stdio.h>
#include <stdlib.h>

#include <PicoLed.hpp>

// Defines
#define LED_RING_PIN    9   ///< GPIO pin for the LED ring
#define LED_RING_COUNT  12  ///< Number of LEDs in the ring

#define RIGHT_DIGIT     0   ///< Right digit on the pcb
#define LEFT_DIGIT      1   ///< Left digit on the pcb

#define SEG_A   0x01        
#define SEG_B   0x02
#define SEG_C   0x04
#define SEG_D   0x08
#define SEG_E   0x10
#define SEG_F   0x20
#define SEG_G   0x40
#define SEG_DP  0x80

#define LED_DEFAULT_ON_TIME     100
#define SEG_DEFAULT_ON_TIME     100

// Data structures
enum eLED_MODE {
    LED_MODE_OFF = 0,
    LED_MODE_ON,
    LED_MODE_CUSTOM,
    LED_MODE_BLINK,
    LED_MODE_FADE,
    LED_MODE_WALK,
    LED_MODE_RAINBOW,
    LED_MODE_FILL_CIRCLE
};

enum eSEG_MODE {
    SEG_MODE_OFF = 0,
    SEG_MODE_ON,
    SEG_MODE_CUSTOM,
    SEG_MODE_BEER_DEC,
    SEG_MODE_BEER_HEX,
};

// Global variables
extern eLED_MODE activeLED_MODE;
extern eSEG_MODE activeSEG_MODE;
extern PicoLed::PicoLedController ledStrip;

// Function prototypes
int LED_Ring_init();
void LED_Ring_Tick();

void LED_Ring_set_mode(eLED_MODE mode);
void SEG_set_mode(eSEG_MODE mode);

void SEG_Tick();

int PCA9552_init();
uint8_t PCA9552_read_reg(uint8_t reg);
void SEG_clear();
void SEG_write_number(uint8_t number);
void SEG_write_number_hex(uint8_t number);
void SEG_add_dot(uint8_t digit);
void SEG_set_segments(uint8_t digit, uint8_t segments);


#endif // __LED_RING_H
