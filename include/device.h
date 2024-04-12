//
//

#ifndef DEVICE_H
#define DEVICE_H

#include "frackstock.h"

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
void DEV_LED_toggle();
void DEV_get_frack_data(tFrackStock *frackstock);
void DEV_set_frack_data(tFrackStock *frackstock);

#endif // DEVICE_H