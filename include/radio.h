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
#define RADIO_SPI_PORT  spi0
#define RADIO_SPI_BAUDRATE 1000000
#define RADIO_SPI_CS    17
#define RADIO_SPI_SCK   18
#define RADIO_SPI_MISO  16
#define RADIO_SPI_MOSI  19
#define RADIO_GDO1      20
#define RADIO_GDO2      21


// Function prototypes
void RADIO_init();
void RADIO_Tick();
void RADIO_send();

#endif // RADIO_H