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

// Function prototypes
int LED_Ring_init();
void LED_Ring_Tick();

int PCA9552_init();
uint8_t PCA9552_read_reg(uint8_t reg);
void SEG_clear();
void SEG_write_number(uint8_t number);
void SEG_write_number_hex(uint8_t number);
void SEG_add_dot(uint8_t digit);
void SEG_set_segments(uint8_t digit, uint8_t segments);


#endif // __LED_RING_H
