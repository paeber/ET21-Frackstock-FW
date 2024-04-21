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
#include "frackstock.h"

#include "cc1101.h"
#include "ccpacket.h"

#include "led_ring.h"

#include "string.h"

#define PRINT_MESSAGE

// Global variables
CC1101 radio(RADIO_SPI_CS, RADIO_GDO1, RADIO_SPI_MISO);
uint8_t syncWord[2] = {0x47, 0xB5};
bool packetWaiting = false;

uint8_t rxFrom_id;
bool new_message = false;

// Function prototypes
void messageReceived(uint gpio, uint32_t events);
void handleMessage();
int RADIO_get_rssi(uint8_t raw_rssi);

// Interrupt functions
void messageReceived(uint gpio, uint32_t events) {
    packetWaiting = true;
    LED_Ring_set_mode(LED_MODE_BLINK);
}

// Function definitions
void handleMessage(){
  CCPACKET packet;
  gpio_set_irq_enabled(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, false);
  if (radio.receiveData(&packet) > 0){
    printf("Received packet...\n");
    if (!packet.crc_ok)
    {
        printf("crc not ok");
    }
    printf("lqi: %d, ", 0x3f - packet.lqi);
    printf("rssi: %d dBm\n", RADIO_get_rssi(packet.rssi));

    if (packet.crc_ok && packet.length > 0)
    {
        printf("packet: len %d, ", packet.length);
        printf("data: ");
        for(int i=0; i<packet.length; i++)
        {
            printf("%02x ", packet.data[i]);
        }
        printf("\n");

        SEG_set_mode(SEG_MODE_CUSTOM);
        LED_Ring_set_mode(LED_MODE_WALK);

        rxFrom_id = packet.data[0];
        SEG_write_number_hex(rxFrom_id);  
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
    gpio_pull_up(RADIO_GDO1);

    gpio_init(RADIO_GDO2);
    gpio_set_dir(RADIO_GDO2, GPIO_IN);
    gpio_pull_up(RADIO_GDO2);

    // Initialize the radio
    radio.init();
    radio.setSyncWord(syncWord);
    radio.setCarrierFreq(CFREQ_868);
    radio.setChannel(RADIO_CHANNEL);
    radio.disableAddressCheck();
    radio.setTxPowerAmp(PA_LongDistance);

    // Attach the interrupt
    gpio_set_irq_enabled_with_callback(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, true, &messageReceived);
}


void RADIO_send() {
    CCPACKET packet;

    gpio_set_irq_enabled(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, false);

    packet.length = 4;
    packet.data[0] = frackstock.id;
    packet.data[1] = frackstock.beer;
    packet.data[2] = 0xa2;
    packet.data[3] = 0xc3;

    radio.sendData(packet);

    gpio_set_irq_enabled_with_callback(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, true, &messageReceived);
}


void RADIO_Tick() {
    if(packetWaiting) {
        packetWaiting = false;
        handleMessage();
        
    }
}


int RADIO_get_rssi(uint8_t raw_rssi) {
    // Umwandlung des Roh-RSSI-Werts in dBm
    int rssi_dbm;
    if (raw_rssi >= 128) {
        rssi_dbm = ((int)(raw_rssi - 256) / 2) - 74;
    } else {
        rssi_dbm = ((int)raw_rssi / 2) - 74;
    }

    return rssi_dbm;
}
