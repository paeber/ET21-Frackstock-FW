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
#include "radio.h"
#include "led_ring.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// Defines

// Global variables
uint8_t messagesEnabled = SERIAL_MESSAGES_ENABLED;
uint8_t userInput = 0;
uint8_t buffer[256];

// Function prototypes
void SERIAL_handle();

// Function definitions
void SERIAL_init() {
    stdio_init_all();
}


void SERIAL_EnableMessages(uint8_t enable) {
    messagesEnabled = enable;
}

void SERIAL_printf(const char *format, ...) {
    if(messagesEnabled == SERIAL_MESSAGES_ENABLED){
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

void SERIAL_Tick() {
    int c = getchar_timeout_us(0);
    if(c != PICO_ERROR_TIMEOUT) {
        if(c == '\n') {
            buffer[userInput] = '\0';
            userInput = 0;
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
        printf("unique - Display the unique ID\n");
        printf("reset - Reset the device\n");
        printf("serial - Enable/Disable serial messages\n");
        printf("set abrev - Set a new abreviation\n");
        printf("set beer - Set the amount of beer\n");
        printf("set buddy - Set the buddy ID\n");
        printf("reset beer - Reset the amount of beer\n");
        printf("set color - Set the (r,g,b) of the LED Ring\n");
        printf("set led_restore_mode - Set the LED restore mode\n");
        printf("send to - Send a message to a specific ID\n");
        printf("apply - Apply the changes\n");
        printf("bootloader - Enter the bootloader (disabled)\n");
    } else if(strcmp((char *)buffer, "status") == 0) {
        sprintf((char *)buffer, "{\"v\": %d.%d, \"id\": %d, \"buddy\":%d, \"beer\": %d, \"color\": [%d, %d, %d], \"abrev\": \"%s\"}", VERSION_MAJOR, VERSION_MINOR, frackstock.id, frackstock.buddy, frackstock.beer, frackstock.color[0], frackstock.color[1], frackstock.color[2], frackstock.abrev);
        printf("%s\n", buffer);
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
    } else if(strncmp((char *)buffer, "set buddy ", 10) == 0) {
        frackstock.buddy = atoi((char *)buffer+10);
        printf("New buddy ID set\n");
    } else if(strncmp((char *)buffer, "set color ", 10) == 0) {
        // Parse the color by reading (r,g,b)
        int r = 0;
        int g = 0;
        int b = 0;
        sscanf((char *)buffer+10, "(%d,%d,%d)", &r, &g, &b);
        frackstock.color[0] = r;
        frackstock.color[1] = g;
        frackstock.color[2] = b;
        printf("New color set\n");
    } else if(strcmp((char *)buffer, "apply") == 0){
        DEV_set_frack_data(&frackstock);
        printf("Changes applied\n");
    } else if(strcmp((char *)buffer, "reset") == 0){
        DEV_reset_mcu();
    } else if(strcmp((char *)buffer, "unique") == 0){
        uint8_t unique_id[8];
        DEV_get_unique_id(unique_id);
        printf("%02X %02X %02X %02X %02X %02X %02X %02X\n", unique_id[0], unique_id[1], unique_id[2], unique_id[3], unique_id[4], unique_id[5], unique_id[6], unique_id[7]);
    } else if(strncmp((char *)buffer, "set id ", 7) == 0){
        frackstock.id = atoi((char *)buffer+7);
        printf("New ID set\n");
    } else if(strcmp((char *)buffer, "bootloader") == 0){
        DEV_enter_bootloader();
    } else if(strncmp((char *)buffer, "serial", 6) == 0){
        uint8_t enable = atoi((char *)buffer+7);
        if(enable == 0){
            SERIAL_EnableMessages(SERIAL_MESSAGES_DISABLED);
            printf("Serial messages disabled\n");
        } else {
            SERIAL_EnableMessages(SERIAL_MESSAGES_ENABLED);
            printf("Serial messages enabled\n");
        }
    } else if (strncmp((char *)buffer, "send to ", 8) == 0) {
        // Send a message to a specific ID
        uint8_t id = atoi((char *)buffer+8);
        printf("Sending message to ID %d\n", id);
        RADIO_send(id);
        LED_Ring_set_color(frackstock.color[0], frackstock.color[1], frackstock.color[2]);
        LED_Ring_set_mode(LED_MODE_FILL_CIRCLE);
    } else if (strncmp((char *)buffer, "set led_restore_mode ", 21) == 0) {
        uint8_t mode = atoi((char *)buffer+21);
        frackstock.led_mode = mode;
        printf("New LED restore mode set\n");
    } else {
        printf("Unknown command\n");
    }
}

