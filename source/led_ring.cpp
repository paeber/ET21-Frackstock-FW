//
// Description: LED Ring and Segment display control
//

#include "main.h"
#include "led_ring.h"

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#include <PicoLed.hpp>


// LED Ring
#define LED_RING_PIN    9
#define LED_RING_COUNT  12

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

const uint8_t seg_order_digit_r[8] = {1, 0, 6, 5, 4, 2, 3, 7};
const uint8_t seg_order_digit_l[8] = {13, 12, 10, 9, 8, 14, 15, 11};

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
            digit_1_out |= (1 << (seg_order_digit_l[i] % 8) * 2);
        }

        if(segment_digit_8[ones] & (1 << i) ){
            //digit_2_out &= ~(1 << (seg_order_digit_r[i] % 8));
        } else {
            digit_2_out |= (1 << (seg_order_digit_r[i] % 8) * 2);
        }
    }

    data[1] = digit_2_out & 0xFF;
    data[2] = digit_2_out >> 8;    
    data[3] = digit_1_out & 0xFF;
    data[4] = digit_1_out >> 8;

    ret = i2c_write_blocking(SEG_I2C_PORT, PCA_ADDR, data, 5, false);
}


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
            digit_1_out |= (1 << (seg_order_digit_l[i] % 8) * 2);
        }

        if(segment_digit_8[ones] & (1 << i) ){
            //digit_2_out &= ~(1 << (seg_order_digit_r[i] % 8));
        } else {
            digit_2_out |= (1 << (seg_order_digit_r[i] % 8) * 2);
        }
    }

    data[1] = digit_2_out & 0xFF;
    data[2] = digit_2_out >> 8;    
    data[3] = digit_1_out & 0xFF;
    data[4] = digit_1_out >> 8;

    ret = i2c_write_blocking(SEG_I2C_PORT, PCA_ADDR, data, 5, false);
}


void SEG_add_dot(uint8_t digit){
    uint8_t data[5];
    int ret;

    data[0] = PCA_REG_LS0 | PCA_AI_FLAG;
    data[1] = PCA9552_read_reg(PCA_REG_LS0);
    data[2] = PCA9552_read_reg(PCA_REG_LS1);
    data[3] = PCA9552_read_reg(PCA_REG_LS2);
    data[4] = PCA9552_read_reg(PCA_REG_LS3);

    if(digit == 2){
        data[2] &= ~(0x3 << 6);
    } else if(digit == 1){
        data[3] &= ~(0x3 << 6);
    }

    ret = i2c_write_blocking(SEG_I2C_PORT, PCA_ADDR, data, 5, false);
}


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

int PCA9552_init(){
    uint8_t data[5];
    int ret;

    data[0] = PCA_REG_LS0 | PCA_AI_FLAG;
    data[1] = 0x00;
    data[2] = 0x55;
    data[3] = 0x00;
    data[4] = 0x55;

    SEG_write_number_hex((uint8_t)(VERSION_MAJOR << 4 | VERSION_MINOR));
    SEG_add_dot(1);
    
    //ret = i2c_write_blocking(SEG_I2C_PORT, PCA_ADDR, data, 5, false);
    printf("PCA9552 init: %d\n", ret);

    return 0;
}

int LED_Ring_init(){
    // Initialize the I2C bus
    i2c_init(SEG_I2C_PORT, 100 * 1000);
    gpio_set_function(SEG_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SEG_SCL_PIN, GPIO_FUNC_I2C);

    // Initialize the PCA9552
    PCA9552_init();

    return 0;
}