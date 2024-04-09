//
//

#ifndef DEVICE_H
#define DEVICE_H

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"

// Define
#define IDX_VERSION             0
#define IDX_FLASH_CNT           1

#define IDX_BEER                10
#define IDX_ABREV               11
#define LEN_ABREV               8

// Function prototypes
int DEV_init();
int DEV_write_flash_data(uint8_t* data, size_t length);
int DEV_read_flash_data(uint8_t* buffer, size_t length);


#endif // DEVICE_H