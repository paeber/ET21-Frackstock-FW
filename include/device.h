//
//

#ifndef DEVICE_H
#define DEVICE_H

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"

// Function prototypes
int DEV_init();
int DEV_write_flash_data(uint8_t* data, size_t length);
int DEV_read_flash_data(uint8_t* buffer, size_t length);


#endif // DEVICE_H