//
// LED Ring and Segment display control
//
// Description:     LED Ring and Segment display control
// Author:          Pascal Eberhard
//

#include "main.h"
#include "led_ring.h"
#include "frackstock.h"

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#include <PicoLed.hpp>


// LED Ring
#define LED_RING_PIN        9
#define LED_RING_COUNT      12
#define LED_RING_BRIGHTNESS 64

// Segment display
#define SEG_I2C_PORT    i2c1
#define SEG_SDA_PIN     26
#define SEG_SCL_PIN     27

#define PCA_ADDR        0x60
#define PCA_AI_FLAG     0x10
#define PCA_REG_IPUT0   0x00
#define PCA_REG_IPUT1   0x01
#define PCA_REG_PSC0    0x02
#define PCA_REG_PWM0    0x03
#define PCA_REG_PSC1    0x04
#define PCA_REG_PWM1    0x05
#define PCA_REG_LS0     0x06
#define PCA_REG_LS1     0x07
#define PCA_REG_LS2     0x08
#define PCA_REG_LS3     0x09

#define PCA_OUT_LOW     0b00
#define PCA_OUT_HIGH    0b01
#define PCA_OUT_PWM0    0b10
#define PCA_OUT_PWM1    0b11

eSEG_MODE activeSEG_MODE = SEG_MODE_OFF;
eLED_MODE activeLED_MODE = LED_MODE_OFF;
PicoLed::PicoLedController ledStrip = PicoLed::addLeds<PicoLed::WS2812B>(pio0, 0, LED_RING_PIN, LED_RING_COUNT, PicoLed::FORMAT_GRB);

const uint8_t seg_order_digit[2][8] = {
    {1, 0, 6, 5, 4, 2, 3, 7},
    {13, 12, 10, 9, 8, 14, 15, 11}
    };

const uint8_t segment_digit_8[17] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b01110111, // A
    0b01111100, // b
    0b00111001, // C
    0b01011110, // d
    0b01111001, // E
    0b01110001, // F
    0b00000000 // blank
};


/**
 * Writes a number to the LED segment display.
 *
 * This function takes a single parameter `number` of type `uint8_t` and writes it to the LED segment display.
 * The number is displayed as a two-digit decimal number on the display.
 *
 * @param number The number to be displayed on the LED segment display.
 */
void SEG_write_number(uint8_t number){
    uint8_t data[5];
    int ret;
    int tens, ones;
    uint16_t digit_1_out = 0x00;
    uint16_t digit_2_out = 0x00;

    tens = (number % 100) / 10;
    ones = (number % 100) % 10;

    data[0] = PCA_REG_LS0 | PCA_AI_FLAG;

    for (int i = 0; i < 8; i++) {
        if(segment_digit_8[tens] & (1 << i) ){
            //digit_1_out &= ~(1 << (seg_order_digit_l[i] % 8));
        } else {
            digit_1_out |= (1 << (seg_order_digit[LEFT_DIGIT][i] % 8) * 2);
        }

        if(segment_digit_8[ones] & (1 << i) ){
            //digit_2_out &= ~(1 << (seg_order_digit_r[i] % 8));
        } else {
            digit_2_out |= (1 << (seg_order_digit[RIGHT_DIGIT][i] % 8) * 2);
        }
    }

    data[1] = digit_2_out & 0xFF;
    data[2] = digit_2_out >> 8;    
    data[3] = digit_1_out & 0xFF;
    data[4] = digit_1_out >> 8;

    ret = i2c_write_blocking(SEG_I2C_PORT, PCA_ADDR, data, 5, false);
}


/**
 * Writes a hexadecimal number to the LED ring display.
 *
 * This function takes a decimal number and converts it to its hexadecimal representation.
 * The converted number is then displayed on the LED ring display using two digits.
 *
 * @param number The decimal number to be converted and displayed.
 */
void SEG_write_number_hex(uint8_t number){
    uint8_t data[5];
    int ret;
    int tens, ones;
    uint16_t digit_1_out = 0x00;
    uint16_t digit_2_out = 0x00;

    tens = number / 16;
    ones = number % 16;

    data[0] = PCA_REG_LS0 | PCA_AI_FLAG;

    for (int i = 0; i < 8; i++) {
        if(segment_digit_8[tens] & (1 << i) ){
            //digit_1_out &= ~(1 << (seg_order_digit_l[i] % 8));
        } else {
            digit_1_out |= (1 << (seg_order_digit[LEFT_DIGIT][i] % 8) * 2);
        }

        if(segment_digit_8[ones] & (1 << i) ){
            //digit_2_out &= ~(1 << (seg_order_digit_r[i] % 8));
        } else {
            digit_2_out |= (1 << (seg_order_digit[RIGHT_DIGIT][i] % 8) * 2);
        }
    }

    data[1] = digit_2_out & 0xFF;
    data[2] = digit_2_out >> 8;    
    data[3] = digit_1_out & 0xFF;
    data[4] = digit_1_out >> 8;

    ret = i2c_write_blocking(SEG_I2C_PORT, PCA_ADDR, data, 5, false);
}


/**
 * Adds a dot to the specified digit on the LED ring.
 *
 * @param digit The digit to add the dot to. Should be either LEFT_DIGIT or RIGHT_DIGIT.
 */
void SEG_add_dot(uint8_t digit){
    uint8_t data[5];
    int ret;

    data[0] = PCA_REG_LS0 | PCA_AI_FLAG;
    data[1] = PCA9552_read_reg(PCA_REG_LS0);
    data[2] = PCA9552_read_reg(PCA_REG_LS1);
    data[3] = PCA9552_read_reg(PCA_REG_LS2);
    data[4] = PCA9552_read_reg(PCA_REG_LS3);

    if(digit == LEFT_DIGIT){
        data[3] &= ~(0x3 << 6);
    } else if(digit == RIGHT_DIGIT){
        data[2] &= ~(0x3 << 6);
    }

    ret = i2c_write_blocking(SEG_I2C_PORT, PCA_ADDR, data, 5, false);
}


/**
 * Sets the segments of a specific digit on the LED ring.
 *
 * @param digit The digit to set the segments for (RIGHT_DIGIT or LEFT_DIGIT).
 * @param segments The segments to be displayed on the digit.
 */
void SEG_set_segments(uint8_t digit, uint8_t segments){
    uint8_t data[3];
    int ret;
    uint16_t digit_out = 0x00;

    if(digit == RIGHT_DIGIT){
        data[0] = PCA_REG_LS0 | PCA_AI_FLAG;
    } else if(digit == LEFT_DIGIT){
        data[0] = PCA_REG_LS2 | PCA_AI_FLAG;
    }

    for (int i = 0; i < 8; i++) {
        if(segments & (1 << i) ){
            //digit_out &= ~(1 << (seg_order_digit_l[i] % 8));
        } else {
            digit_out |= (1 << (seg_order_digit[digit][i] % 8) * 2);
        }
    }

    data[1] = digit_out & 0xFF;
    data[2] = digit_out >> 8;

    ret = i2c_write_blocking(SEG_I2C_PORT, PCA_ADDR, data, 3, false);
}


/**
 * Clears the Segments by sending a command to the PCA9685 LED driver.
 */
void SEG_clear(){
    uint8_t data[5];
    int ret;

    data[0] = PCA_REG_LS0 | PCA_AI_FLAG;
    data[1] = 0x55;
    data[2] = 0x55;
    data[3] = 0x55;
    data[4] = 0x55;

    ret = i2c_write_blocking(SEG_I2C_PORT, PCA_ADDR, data, 5, false);
}


/**
 * @brief Reads a register from PCA9552.
 * 
 * This function reads a register from PCA9552 using I2C communication.
 * 
 * @param reg The register address to read.
 * @return The value read from the register. If an error occurs during the I2C communication, 0xfe is returned.
 */
uint8_t PCA9552_read_reg(uint8_t reg){
    int ret;
    uint8_t buf[1];

    ret = i2c_write_blocking(SEG_I2C_PORT, PCA_ADDR, &reg, 1, true);
    if(ret < 0){
        return 0xfe;
    }
    ret = i2c_read_blocking(SEG_I2C_PORT, PCA_ADDR, buf, 1, false);
    if(ret < 0){
        return 0xfe;
    }

    return buf[0];
}


/**
 * @brief Initializes the PCA9552.
 * 
 * This function initializes the PCA9552 by clearing the segments on the LED ring, 
 * displaying a sequence of segments on the left and right digits, writing the 
 * version number in hexadecimal format, and adding a dot to the left digit. 
 * 
 * @return 0 on success.
 */
int PCA9552_init(){
    uint8_t data[5];
    int ret;

    // SEG_clear();

    // for(int i=0; i<8; i++){
    //     SEG_set_segments(LEFT_DIGIT, SEG_A << i);
    //     sleep_ms(100);
    // }
    // for(int i=0; i<8; i++){
    //     SEG_set_segments(RIGHT_DIGIT, SEG_A << i);
    //     sleep_ms(100);
    // }

    SEG_clear();
    SEG_write_number_hex((uint8_t)(VERSION_MAJOR << 4 | VERSION_MINOR));
    SEG_add_dot(LEFT_DIGIT);
    
    printf("PCA9552 init: %d\n", ret);

    return 0;
}


/**
 * @brief Initializes the LED ring.
 * 
 * This function initializes the I2C bus and the PCA9552 chip used for controlling the LED ring.
 * 
 * @return 0 if initialization is successful, otherwise an error code.
 */
int LED_Ring_init(){
    // Initialize the LED ring
    ledStrip.setBrightness(LED_RING_BRIGHTNESS);
    ledStrip.fill( PicoLed::RGB(0, 0, 0) );
    ledStrip.show();

    // Initialize the I2C bus
    i2c_init(SEG_I2C_PORT, 100 * 1000);
    gpio_set_function(SEG_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SEG_SCL_PIN, GPIO_FUNC_I2C);

    // Initialize the PCA9552
    PCA9552_init();

    return 0;
}


void LED_Ring_set_mode(eLED_MODE mode){
    activeLED_MODE = mode;
}


void LED_Ring_Tick(){
    static uint8_t tick = 0;


    switch(activeLED_MODE){
        case LED_MODE_CUSTOM:
            break;

        case LED_MODE_OFF:
            ledStrip.fill( PicoLed::RGB(0, 0, 0) );
            break;

        case LED_MODE_ON:
            ledStrip.fill( PicoLed::RGB(0, 255, 255) );
            break;

        case LED_MODE_BLINK:
            if(tick % 4 < 2){
                ledStrip.fill( PicoLed::RGB(0, 255, 255) );
            } else {
                ledStrip.fill( PicoLed::RGB(0, 0, 0) );
            }
            break;

        case LED_MODE_FADE:
            ledStrip.fill( PicoLed::HSV(tick, 255, 255) );
            break;

        case LED_MODE_WALK:
            ledStrip.fill( PicoLed::RGB(0, 0, 0) );
            ledStrip.setPixelColor(tick % LED_RING_COUNT, PicoLed::RGB(255, 255, 255));
            break;

        case LED_MODE_RAINBOW:
            ledStrip.fillRainbow(tick, 255 / LED_RING_COUNT);
            break;
        
    }

    ledStrip.show();

    tick++;
}

void SEG_Tick(){
    static uint16_t number = 0;

    switch (activeSEG_MODE)
    {
        case SEG_MODE_OFF:
            SEG_clear();
            break;
        
        case SEG_MODE_ON:
            SEG_write_number(number / 10);
            break;
        
        case SEG_MODE_CUSTOM:
            break;

        case SEG_MODE_BEER_DEC:
            SEG_write_number(FRACK_get_beer());
            break;

        case SEG_MODE_BEER_HEX:
            SEG_write_number_hex(FRACK_get_beer());
            break;

    }
    
    number++;
    number %= 1000;

}