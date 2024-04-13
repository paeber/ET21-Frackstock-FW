//
// Wireless Communication Module for CC1101RGPR
//
// Author:      Pascal Eberhard
// Datasheet:   https://www.ti.com/lit/ds/symlink/cc1101.pdf
//
// Maybe use?   https://github.com/mengguang/cc1101/tree/main
//

#include "radio.h"
#include "main.h"

#include "cc1101.h"
#include "ccpacket.h"

#include "led_ring.h"

#include "string.h"

#define PRINT_MESSAGE

// Global variables
CC1101 radio(RADIO_SPI_CS, RADIO_GDO1, RADIO_SPI_MISO);
uint8_t syncWord[2] = {0x47, 0xB5};
bool packetWaiting = false;

uint8_t last_id, id,kat,r,g,b;
bool new_message = false;
bool doLights, doLightsLast = false;

// Function prototypes
void messageReceived(uint gpio, uint32_t events);
void handleMessage();

// Interrupt functions
void messageReceived(uint gpio, uint32_t events) {
    packetWaiting = true;
    activeLED_MODE = LED_MODE_BLINK;
}

// Function definitions

void handleMessage(){
  CCPACKET packet;
  gpio_set_irq_enabled(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, false);
  if (radio.receiveData(&packet) > 0){
    if (packet.crc_ok && packet.length > 0){
        if(!doLights){
            id = (uint8_t)packet.data[0];
            kat = (uint8_t)packet.data[1];
            r = (uint8_t)packet.data[2];
            g = (uint8_t)packet.data[3];
            b = (uint8_t)packet.data[4];
            #ifdef PRINT_MESSAGE
            printf("KAT: %d\n", kat);
            #endif

        }
        printf("Received packet: ");
        for(int i = 0; i < packet.length; i++) {
            printf("0x%02x ", packet.data[i]);
        }
        printf("\n");

        activeSEG_MODE = SEG_MODE_CUSTOM;
        activeLED_MODE = LED_MODE_WALK;

        SEG_write_number_hex(packet.data[1]);   
    }
  }
  gpio_set_irq_enabled_with_callback(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, true, &messageReceived);
}

void RADIO_init() {
    // Initialize the SPI interface
    spi_init(RADIO_SPI_PORT, RADIO_SPI_BAUDRATE);

    // Configure SPI pins
    gpio_set_function(RADIO_SPI_SCK, GPIO_FUNC_SPI);
    gpio_set_function(RADIO_SPI_MISO, GPIO_FUNC_SPI);
    gpio_set_function(RADIO_SPI_MOSI, GPIO_FUNC_SPI);
    //gpio_set_function(RADIO_SPI_CS, GPIO_FUNC_SPI);

    // Configure GPIOs
    gpio_init(RADIO_SPI_CS);
    gpio_set_dir(RADIO_SPI_CS, GPIO_OUT);
    gpio_put(RADIO_SPI_CS, 1);

    gpio_init(RADIO_GDO1);
    gpio_set_dir(RADIO_GDO1, GPIO_IN);
    //gpio_pull_up(RADIO_GDO1);

    gpio_init(RADIO_GDO2);
    gpio_set_dir(RADIO_GDO2, GPIO_IN);
    //gpio_pull_up(RADIO_GDO2);

    //
    sleep_ms(1000);

    // Initialize the radio
    radio.init();
    radio.setSyncWord(syncWord);
    radio.setCarrierFreq(CFREQ_868);
    radio.setChannel(0);
    radio.disableAddressCheck();
    //radio.setTxPowerAmp(PA_LongDistance);

    // Attach the interrupt
    gpio_set_irq_enabled_with_callback(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, true, &messageReceived);
    //attachInterrupt(CC1101Interrupt, messageReceived, FALLING);
}


void RADIO_send() {
    CCPACKET packet;

    gpio_set_irq_enabled(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, false);

    packet.length = 5;
    packet.data[0] = 1; //id;
    packet.data[1] = 0x2e; //kat;
    packet.data[2] = 3; //r;
    packet.data[3] = 4; //g;
    packet.data[4] = 5; //b;
    radio.sendData(packet);

    gpio_set_irq_enabled_with_callback(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, true, &messageReceived);
}


void RADIO_Tick() {
    if(packetWaiting) {
        packetWaiting = false;
        handleMessage();
        
    }
}



