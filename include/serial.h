//
// Serial Communication Interface
//
// Author:      Pascal Eberhard
// 
//

#ifndef SERIAL_H
#define SERIAL_H

#include "pico/stdlib.h"
#include "hardware/uart.h"

// Defines
#define SERIAL_MESSAGES_ENABLED 1
#define SERIAL_MESSAGES_DISABLED 0

// Function prototypes
void SERIAL_init();
void SERIAL_Tick();
void SERIAL_EnableMessages(uint8_t enable);
void SERIAL_printf(const char *format, ...);





#endif // SERIAL_H