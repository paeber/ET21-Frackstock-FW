//
// Wireless Communication Module for CC1101RGPR
//
// Author:      Pascal Eberhard
// Datasheet:   https://www.ti.com/lit/ds/symlink/cc1101.pdf
//

#ifndef RADIO_H
#define RADIO_H


#include "pico/stdlib.h"
#include "hardware/spi.h"


// Defines
#define RADIO_SPI_PORT      spi0
#define RADIO_SPI_BAUDRATE  1000000
#define RADIO_SPI_CS        17
#define RADIO_SPI_SCK       18
#define RADIO_SPI_MISO      16
#define RADIO_SPI_MOSI      19
#define RADIO_GDO1          20
#define RADIO_GDO2          21

#define RADIO_CHANNEL       0x00

#define BROADCAST_ADDRESS   0xFF
#define RADIO_TTL           2

#define PACKET_IDX_OWNER        0
#define PACKET_IDX_TARGET       1
#define PACKET_IDX_TTL          2
#define PACKET_IDX_REPEATER_1   3
#define PACKET_IDX_REPEATER_2   4
#define PACKET_IDX_BEER         5
#define PACKET_IDX_COLOR_R      6
#define PACKET_IDX_COLOR_G      7
#define PACKET_IDX_COLOR_B      8
#define PACKET_IDX_LED_MODE     9



// Function prototypes
void RADIO_init();
void RADIO_Tick();
void RADIO_send(uint8_t target);

#endif // RADIO_H