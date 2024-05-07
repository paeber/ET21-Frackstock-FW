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
#include "serial.h"
#include "string.h"
#include "interrupts.h"
#include "gpio.h"


// Global variables
CC1101 radio(RADIO_SPI_CS, RADIO_GDO1, RADIO_SPI_MISO);
uint8_t syncWord[2] = {0x47, 0xB5};
bool packetWaiting = false;

uint8_t serBuffer[100];
bool new_message = false;

// Function prototypes
void messageReceived(uint gpio, uint32_t events);
void handleMessage();
int RADIO_get_rssi(uint8_t raw_rssi);
void RADIO_repeat(uint8_t *data, uint8_t length);



// Function definitions
void handleMessage(){
  CCPACKET packet;
  gpio_set_irq_enabled(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, false);
  if (radio.receiveData(&packet) > 0){
    if (!packet.crc_ok)
    {
        SERIAL_printf("[RF] crc not ok\n");
    }
    sprintf((char *)serBuffer, "lqi: %d, rssi: %d dBm", 0x3f - packet.lqi, RADIO_get_rssi(packet.rssi));
    SERIAL_printf("[RF] RX: %s\n", serBuffer);

    if (packet.crc_ok && packet.length > 0)
    {
        sprintf((char *)serBuffer, "packet: len %d", packet.length);
        SERIAL_printf("[RF] RX: %s\n", serBuffer);

        // Check if packet is from this device
        if(packet.data[PACKET_IDX_OWNER] == frackstock.id || packet.data[PACKET_IDX_REPEATER_1] == frackstock.id || packet.data[PACKET_IDX_REPEATER_2] == frackstock.id){
            SERIAL_printf("[RF] RX: packet from self\n");
            gpio_set_irq_enabled_with_callback(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, true, &handle_Interrupts);
            return;
        }

        // Check if packet is for this device
        if(packet.data[PACKET_IDX_TARGET] != BROADCAST_ADDRESS && packet.data[PACKET_IDX_TARGET] != frackstock.id){
            SERIAL_printf("[RF] RX: packet not for self\n");
            gpio_set_irq_enabled_with_callback(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, true, &handle_Interrupts);
            return;
        }

        // Check if user is not using button
        if(GPIO_Button_getStates() == 0){
            SEG_set_mode(SEG_MODE_CUSTOM);
            SEG_write_number_hex(packet.data[PACKET_IDX_OWNER]);  
            LED_Ring_set_color(packet.data[PACKET_IDX_COLOR_R], packet.data[PACKET_IDX_COLOR_G], packet.data[PACKET_IDX_COLOR_B]);
            LED_Ring_set_mode(LED_MODE_WALK);
        }

        // Check if packet needs to be repeated
        if(packet.data[PACKET_IDX_TTL] > 0) {
            RADIO_repeat(packet.data, packet.length);
        }
    
    }
  }
  gpio_set_irq_enabled_with_callback(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, true, &handle_Interrupts);
}


/**
 * @brief Initializes the radio module.
 * 
 * This function initializes the SPI interface, configures SPI pins, configures GPIOs,
 * initializes the radio, sets the sync word, carrier frequency, channel, address check,
 * transmission power amplifier, and attaches an interrupt for message reception.
 */
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
    gpio_set_irq_enabled_with_callback(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, true, &handle_Interrupts);
}


/**
 * Sends a radio packet.
 * 
 * This function prepares a radio packet with the necessary data and sends it using the radio module.
 * It sets the packet length, owner ID, target address, time-to-live (TTL), repeater addresses, and beer value.
 * After sending the packet, it enables the interrupt for receiving messages.
 */
void RADIO_send() {
    CCPACKET packet;

    gpio_set_irq_enabled(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, false);

    packet.length = 10;
    for(int i=0; i<packet.length; i++){
        packet.data[i] = 0;
    }
    packet.data[PACKET_IDX_OWNER] = frackstock.id;
    packet.data[PACKET_IDX_TARGET] = BROADCAST_ADDRESS;
    packet.data[PACKET_IDX_TTL] = RADIO_TTL;
    packet.data[PACKET_IDX_REPEATER_1] = 0;
    packet.data[PACKET_IDX_REPEATER_2] = 0;
    packet.data[PACKET_IDX_BEER] = frackstock.beer;
    packet.data[PACKET_IDX_COLOR_R] = frackstock.color[0];
    packet.data[PACKET_IDX_COLOR_G] = frackstock.color[1];
    packet.data[PACKET_IDX_COLOR_B] = frackstock.color[2];

    radio.sendData(packet);

    gpio_set_irq_enabled_with_callback(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, true, &handle_Interrupts);
}


/**
 * Repeats a radio packet with modified data.
 *
 * This function takes a data packet and its length, modifies the packet by adding the ID of the current device as a repeater,
 * decrements the TTL (Time To Live) value, and sends the modified packet using the radio module.
 *
 * @param data   Pointer to the data packet.
 * @param length Length of the data packet.
 */
void RADIO_repeat(uint8_t *data, uint8_t length) {
    CCPACKET packet;

    gpio_set_irq_enabled(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, false);

    packet.length = length;
    for(int i=0; i<packet.length; i++){
        packet.data[i] = data[i];
    }

    if(packet.data[PACKET_IDX_REPEATER_1] == 0) {
        packet.data[PACKET_IDX_REPEATER_1] = frackstock.id;
    } else if (packet.data[PACKET_IDX_REPEATER_2] == 0) {
        packet.data[PACKET_IDX_REPEATER_2] = frackstock.id;
    }

    data[PACKET_IDX_TTL]--;
    radio.sendData(packet);

    //gpio_set_irq_enabled_with_callback(RADIO_GDO1, GPIO_IRQ_EDGE_FALL, true, &handle_Interrupts); // Part of handleMessage function
}


/**
 * @brief This function is called periodically to handle incoming radio packets.
 * 
 * It checks if there is a packet waiting to be processed and calls the handleMessage() function to handle the packet.
 */
void RADIO_Tick() {
    if(packetWaiting) {
        packetWaiting = false;
        handleMessage();
        
    }
}


/**
 * @brief Converts the raw RSSI value to dBm.
 *
 * This function takes a raw RSSI value and converts it to dBm (decibel-milliwatts).
 * The raw RSSI value is expected to be in the range of 0 to 255.
 *
 * @param raw_rssi The raw RSSI value to be converted.
 * @return The RSSI value in dBm.
 */
int RADIO_get_rssi(uint8_t raw_rssi) {
    int rssi_dbm;
    if (raw_rssi >= 128) {
        rssi_dbm = ((int)(raw_rssi - 256) / 2) - 74;
    } else {
        rssi_dbm = ((int)raw_rssi / 2) - 74;
    }

    return rssi_dbm;
}
