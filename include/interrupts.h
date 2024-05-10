#ifndef INTERRUPTS_H
#define INTERRUPTS_H

/*! CPP guard */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

#include "pico/stdlib.h"

// Global variables
extern bool packetWaiting;
extern bool IMU_INT1_flag;
extern bool IMU_INT2_flag;

// Function prototypes
void handle_Interrupts(uint gpio, uint32_t events);

#ifdef __cplusplus
}
#endif /* End of CPP guard */

#endif // INTERRUPTS_H