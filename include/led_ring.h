// 
// 

#ifndef __LED_RING_H
#define __LED_RING_H

#include <stdio.h>
#include <stdlib.h>

// Defines
#define LED_RING_PIN    9
#define LED_RING_COUNT  12

// Function prototypes
int LED_Ring_init();
int PCA9552_init();
uint8_t PCA9552_read_reg(uint8_t reg);
void SEG_clear();
void SEG_write_number(uint8_t number);
void SEG_write_number_hex(uint8_t number);
void SEG_add_dot(uint8_t digit);


#endif // __LED_RING_H
