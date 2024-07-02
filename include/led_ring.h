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

#define SEG_CHAR_A (SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)  // A
#define SEG_CHAR_b (SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)          // b
#define SEG_CHAR_C (SEG_A | SEG_D | SEG_E | SEG_F)                  // C
#define SEG_CHAR_c (SEG_D | SEG_E | SEG_G)                          // c
#define SEG_CHAR_d (SEG_B | SEG_C | SEG_D | SEG_E | SEG_G)          // d
#define SEG_CHAR_E (SEG_A | SEG_D | SEG_E | SEG_F | SEG_G)          // E
#define SEG_CHAR_F (SEG_A | SEG_E | SEG_F | SEG_G)                  // F
#define SEG_CHAR_G (SEG_A | SEG_C | SEG_D | SEG_E | SEG_F)          // G
#define SEG_CHAR_H (SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)          // H
#define SEG_CHAR_I (SEG_B | SEG_C)                                  // I
#define SEG_CHAR_L (SEG_D | SEG_E | SEG_F)                          // L
#define SEG_CHAR_N (SEG_C | SEG_E | SEG_G)                          // N
#define SEG_CHAR_n (SEG_C | SEG_E | SEG_G)                          // n
#define SEG_CHAR_o (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)  // o
#define SEG_CHAR_P (SEG_A | SEG_B | SEG_E | SEG_F | SEG_G)          // P
#define SEG_CHAR_r (SEG_E | SEG_G)                                  // r
#define SEG_CHAR_S (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G)          // S
#define SEG_CHAR_t (SEG_F | SEG_E | SEG_D | SEG_G)                  // t
#define SEG_CHAR_U (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)  // U
#define SEG_CHAR_u (SEG_C | SEG_D | SEG_E | SEG_G)                  // u
#define SEG_CHAR_Y (SEG_B | SEG_C | SEG_D | SEG_F | SEG_G)          // Y

#define LED_DEFAULT_ON_TIME     80
#define SEG_DEFAULT_ON_TIME     80

// Data structures
enum eLED_MODE {
    LED_MODE_OFF = 0,
    LED_MODE_TURN_OFF,
    LED_MODE_ON,
    LED_MODE_CUSTOM,
    LED_MODE_BLINK,
    LED_MODE_FADE,
    LED_MODE_WALK,
    LED_MODE_RAINBOW,
    LED_MODE_RGB_WALK,
    LED_MODE_FILL_CIRCLE
};

enum eSEG_MODE {
    SEG_MODE_OFF = 0,
    SEG_MODE_TURN_OFF,
    SEG_MODE_ON,
    SEG_MODE_CUSTOM,
    SEG_MODE_BEER_DEC,
    SEG_MODE_BEER_HEX,
    SEG_MODE_BUFFER
};

enum eSEG_NUMBER_MODE {
    SEG_NUMBER_MODE_DEC = 0,
    SEG_NUMBER_MODE_HEX,
    SEG_NUMBER_MODE_DEC_DOT,
    SEG_NUMBER_MODE_HEX_DOT,
    SEG_NUMBER_MODE_DEC_DOT_RIGHT,
    SEG_NUMBER_MODE_HEX_DOT_RIGHT
};

// Global variables
extern eLED_MODE activeLED_MODE;
extern eSEG_MODE activeSEG_MODE;
extern PicoLed::PicoLedController ledStrip;

// Function prototypes
int LED_Ring_isPending();
int LED_Ring_init();
void LED_Ring_Tick();
void LED_Ring_set_color(uint8_t r, uint8_t g, uint8_t b);
void LED_Ring_set_mode(eLED_MODE mode);
void SEG_set_mode(eSEG_MODE mode);
void LED_Ring_set_mode_restore(eLED_MODE mode);
uint8_t LED_Ring_get_mode_restore();
int SEG_add_to_buffer(uint8_t number, eSEG_NUMBER_MODE mode);
int SEG_pop_from_buffer();
void SEG_clear_buffer();
void SEG_Tick();

int PCA9552_init();
uint8_t PCA9552_read_reg(uint8_t reg);
int SEG_clear();
void SEG_write_number(uint8_t number);
void SEG_write_number_hex(uint8_t number);
void SEG_add_dot(uint8_t digit);
void SEG_set_segments(uint8_t digit, uint8_t segments);


#endif // __LED_RING_H
