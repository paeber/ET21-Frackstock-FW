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
void RADIO_repeat(uint8_t *data, uint8_t length);

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

        rxFrom_id = packet.data[PACKET_IDX_OWNER];
        SEG_write_number_hex(rxFrom_id);  

        // Check if packet needs to be repeated
        if(packet.data[PACKET_IDX_TTL] > 0) {
            if(packet.data[PACKET_IDX_REPEATER_1] != frackstock.id && packet.data[PACKET_IDX_REPEATER_2] != frackstock.id){
                RADIO_repeat(packet.data, packet.length);
            }
        }
    
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

    packet.length = 10;
    packet.data[PACKET_IDX_OWNER] = frackstock.id;
    packet.data[PACKET_IDX_TARGET] = BROADCAST_ADDRESS;
    packet.data[PACKET_IDX_TTL] = RADIO_TTL;
    packet.data[PACKET_IDX_REPEATER_1] = 0;
    packet.data[PACKET_IDX_REPEATER_2] = 0;
    packet.data[PACKET_IDX_BEER] = frackstock.beer;

    radio.sendData(packet);

    gpio_set_irq_enabled_with_callback(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, true, &messageReceived);
}


void RADIO_repeat(uint8_t *data, uint8_t length) {
    CCPACKET packet;

    gpio_set_irq_enabled(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, false);

    packet.length = length;
    memcpy(packet.data, data, length);

    if(packet.data[PACKET_IDX_REPEATER_1] == 0) {
        packet.data[PACKET_IDX_REPEATER_1] = frackstock.id;
    } else if (packet.data[PACKET_IDX_REPEATER_2] == 0) {
        packet.data[PACKET_IDX_REPEATER_2] = frackstock.id;
    }

    data[PACKET_IDX_TTL]--;
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
