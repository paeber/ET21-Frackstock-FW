//
// Serial Communication Interface
//
// Author:      Pascal Eberhard
// 
//



#include "serial.h"
#include "main.h"
#include "frackstock.h"
#include "device.h"

#include <stdio.h>
#include <string.h>



// Defines

// Global variables
uint8_t userInput = 0;
uint8_t buffer[256];

// Function prototypes
void SERIAL_handle();

// Function definitions
void SERIAL_init() {
    stdio_init_all();
}


void SERIAL_Tick() {
    int c = getchar_timeout_us(0);
    if(c != PICO_ERROR_TIMEOUT) {
        if(c == '\n') {
            buffer[userInput] = '\0';
            userInput = 0;
            printf("Received: %s\n", buffer);
            SERIAL_handle();
        } else if(c == '\r') {
            // Ignore
        } else {
            buffer[userInput] = c;
            userInput++;
        }
    }
}

void SERIAL_handle(){
    if(strcmp((char *)buffer, "help") == 0) {
        printf("Available commands:\n");
        printf("help - Display this help\n");
        printf("status - Display the current status\n");
        printf("reset - Reset the device\n");
        printf("set abrev - Set a new abreviation\n");
        printf("set beer - Set the amount of beer\n");
        printf("reset beer - Reset the amount of beer\n");
        printf("set color - Set the color of the LED Ring\n");
        printf("apply - Apply the changes\n");
    } else if(strcmp((char *)buffer, "status") == 0) {
        printf("Status:\n");
        printf("Version: %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
        printf("ID: 0x%02X\n", frackstock.id);
        printf("Beer: %d\n", frackstock.beer);
        printf("Color: %d, %d, %d\n", frackstock.color[0], frackstock.color[1], frackstock.color[2]);
        printf("Abrev: ");
        for(int i = 0; i < LEN_ABREV; i++) {
            printf("%c", frackstock.abrev[i]);
        }
    } else if (strncmp((char *)buffer, "set abrev ", 10) == 0) {
        for(int i = 0; i < LEN_ABREV; i++) {
            frackstock.abrev[i] = buffer[i+10];
        }
        printf("New abreviation set\n");
    } else if(strncmp((char *)buffer, "set beer ", 9) == 0) {
        frackstock.beer = atoi((char *)buffer+9);
        printf("New beer amount set\n");
    } else if(strcmp((char *)buffer, "reset beer ") == 0) {
        frackstock.beer = 0;
        printf("New beer amount set\n");
    } else if(strncmp((char *)buffer, "set color ", 10) == 0) {
        // Parse the color by reading 3 uint8_t numbers
        frackstock.color[0] = atoi((char *)buffer+10);
        frackstock.color[1] = atoi((char *)buffer+13);
        frackstock.color[2] = atoi((char *)buffer+16);
        printf("New color set\n");
    } else if(strcmp((char *)buffer, "apply") == 0){
        DEV_set_frack_data(&frackstock);
        printf("Changes applied\n");
    } else if(strcmp((char *)buffer, "reset") == 0){
        DEV_reset_mcu();
    } else {
        printf("Unknown command\n");
    }
}

