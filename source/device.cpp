//
//
//

#include "device.h"

#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "pico/stdlib.h"
#include "hardware/flash.h"



#define FLASH_TARGET_OFFSET (256 * 1024) 

const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);


int DEV_init() {
    uint8_t* flash_data = (uint8_t*) malloc(16);

    DEV_read_flash_data(flash_data, 16);

    printf("Flash data: ");
    for(int i = 0; i < 16; i++) {
        printf("%d ", flash_data[i]);
    }
    printf("\n");

    return 0;
}

int DEV_write_flash_data(uint8_t* data, size_t length) {
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, data, length);

    // Check if write was successful
    uint8_t* check_data = (uint8_t*) malloc(length);
    memcpy(check_data, (void*)(XIP_BASE + FLASH_TARGET_OFFSET), length);
    int success = memcmp(data, check_data, length);
    free(check_data);

    return success;
}


int DEV_read_flash_data(uint8_t* buffer, size_t length) {
    memcpy(buffer, (void*)(XIP_BASE + FLASH_TARGET_OFFSET), length);

    // Check if read was successful
    int success = memcmp(buffer, (void*)(XIP_BASE + FLASH_TARGET_OFFSET), length);

    return success;
}