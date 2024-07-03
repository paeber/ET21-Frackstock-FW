//
// Device Module
//

#include "device.h"
#include "main.h"
#include "frackstock.h"

#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

#include "pico/bootrom.h"
#include "class/cdc/cdc_device.h"


// Defines
#define FLASH_TARGET_OFFSET     (uint32_t)(256 * 1024)
#define DEF_ABREV               "FRK_ET21"
 

// Global variables
const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);


// Function prototypes
void print_buf(const uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", buf[i]);
        if (i % 16 == 15)
            printf("\n");
        else
            printf(" ");
    }
}


/**
 * @brief Resets the microcontroller.
 *
 * This function performs a reset of the microcontroller by writing a specific value to a memory address.
 */
void DEV_reset_mcu() {
    // *((volatile uint32_t*)(PPB_BASE + 0x0ED0C)) = 0x5FA0004;
    while (1){}; // Wait for reset by watchdog
}


/**
 * @brief Retrieves the unique ID of the device.
 *
 * This function calls the `flash_get_unique_id` function to retrieve the unique ID of the device.
 *
 * @param unique_id Pointer to an array where the unique ID will be stored.
 */
void DEV_get_unique_id(uint8_t *unique_id) {
    flash_get_unique_id(unique_id);
}

/**
 * @brief Initializes the device.
 * 
 * This function initializes the device by performing the following steps:
 * 1. Retrieves the unique ID of the device.
 * 2. Checks if the version in the target region matches the current version.
 *    - If the versions do not match, it prepares new data based on the current version.
 * 3. Prints the new data.
 * 4. Erases the target region.
 * 5. Programs the target region with the new data.
 * 6. Checks if the programming was successful by comparing the written data with the target region contents.
 *    - If there is a mismatch, it prints "Programming failed!".
 * 
 * @return 0 indicating success.
 */
int DEV_init() {
    uint8_t write_data[FLASH_PAGE_SIZE];
    uint8_t unique_id[8];

    // necessary to make things reflashable
    tud_cdc_set_wanted_char('\0');

    DEV_get_unique_id(unique_id);

    printf("Unique ID: ");
    print_buf(unique_id, 8);
    printf("\n");

    printf("User abreviation: ");
    for(int i = 0; i < LEN_ABREV; i++) {
        printf("%c", flash_target_contents[IDX_ABREV + i]);
    }
    printf("\n");


    if(flash_target_contents[IDX_VERSION] == (VERSION_MAJOR << 4 | VERSION_MINOR)) {
        printf("Version match!\n");
    } else {
        printf("Version mismatch!\n");
        printf("Prepare new data...\n");

        if(flash_target_contents[IDX_VERSION] == 0xff) {
            printf("First time flashing...\n");
            write_data[IDX_VERSION] = VERSION_MAJOR << 4 | VERSION_MINOR;
            write_data[IDX_FRACK_ID] = unique_id[7];
            write_data[IDX_BUDDY_ID] = 0xff;
            write_data[IDX_BEER] = 0;
            write_data[IDX_FLASH_CNT] = 1;
            memcpy(write_data + IDX_ABREV, DEF_ABREV, LEN_ABREV);
            memcpy(write_data + IDX_COLOR, "\x00\xff\x00", LEN_COLOR);
            write_data[IDX_LED_MODE] = 5;
        } else {
            write_data[IDX_VERSION] = VERSION_MAJOR << 4 | VERSION_MINOR;
            write_data[IDX_FRACK_ID] = flash_target_contents[IDX_FRACK_ID];
            write_data[IDX_BUDDY_ID] = flash_target_contents[IDX_BUDDY_ID];
            write_data[IDX_BEER] = flash_target_contents[IDX_BEER];
            write_data[IDX_FLASH_CNT] = flash_target_contents[IDX_FLASH_CNT] + 1;
            memcpy(write_data + IDX_ABREV, flash_target_contents + IDX_ABREV, LEN_ABREV);
            memcpy(write_data + IDX_COLOR, flash_target_contents + IDX_COLOR, LEN_COLOR);
            if(flash_target_contents[IDX_LED_MODE]){    // Check if Custom value is set
                write_data[IDX_LED_MODE] = flash_target_contents[IDX_LED_MODE];
            } else {
                write_data[IDX_LED_MODE] = 5;
            }
        }

        printf("New data:\n");
        print_buf(write_data, FLASH_PAGE_SIZE);
    

        uint32_t interrupts = save_and_disable_interrupts();

        // Note that a whole number of sectors must be erased at a time.
        printf("\nErasing target region...\n");
        flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    
        printf("\nProgramming target region...\n");
        flash_range_program(FLASH_TARGET_OFFSET, write_data, FLASH_PAGE_SIZE);

        restore_interrupts(interrupts);

    
        bool mismatch = false;
        for (int i = 0; i < FLASH_PAGE_SIZE; ++i) {
            if (write_data[i] != flash_target_contents[i])
                mismatch = true;
        }
        if (mismatch){
            printf("Programming failed!\n");
            return 1;
        }
    } 

    return 0;
}


/**
 * @brief Retrieves frack data from the device.
 *
 * This function retrieves frack data from the device and stores it in the provided tFrackStock structure.
 *
 * @param frackstock Pointer to the tFrackStock structure where the frack data will be stored.
 */
void DEV_get_frack_data(tFrackStock *frackstock) {
    frackstock->id = flash_target_contents[IDX_FRACK_ID];
    frackstock->buddy = flash_target_contents[IDX_BUDDY_ID];
    frackstock->beer = flash_target_contents[IDX_BEER];
    memcpy(frackstock->abrev, flash_target_contents + IDX_ABREV, LEN_ABREV);
    memcpy(frackstock->color, flash_target_contents + IDX_COLOR, LEN_COLOR);
    frackstock->led_mode = flash_target_contents[IDX_LED_MODE];
}


/**
 * @brief Sets the frack data in the device.
 * 
 * This function prepares the frack data and writes it to the flash memory of the device.
 * The frack data includes the version, beer quantity, and abbreviation.
 * 
 * @param frackstock Pointer to the frackstock structure containing the frack data.
 */
void DEV_set_frack_data(tFrackStock *frackstock){
    uint8_t write_data[FLASH_PAGE_SIZE];

    for(int i = 0; i < FLASH_PAGE_SIZE; i++) {
        write_data[i] = 0x00;
    }

    // Prepare data to write
    write_data[IDX_VERSION] = VERSION_MAJOR << 4 | VERSION_MINOR;  
    write_data[IDX_FRACK_ID] = frackstock->id; 
    write_data[IDX_BUDDY_ID] = frackstock->buddy;
    write_data[IDX_BEER] = frackstock->beer;
    memcpy(write_data + IDX_ABREV, frackstock->abrev, LEN_ABREV);
    memcpy(write_data + IDX_COLOR, frackstock->color, LEN_COLOR);
    write_data[IDX_LED_MODE] = frackstock->led_mode;

    uint32_t interrupts = save_and_disable_interrupts();

    // Write data to flash
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, write_data, FLASH_PAGE_SIZE);
    
    restore_interrupts(interrupts);

}


void DEV_enter_bootloader() {
    // Set the magic value to reboot to the bootloader
    //flash_range_program(FLASH_TARGET_OFFSET + 0x10, (const uint8_t *)"\x01\x23\x45\x67\x89\xAB\xCD\xEF", 8);
    //DEV_reset_mcu();
    reset_usb_boot(0, 0);
}
