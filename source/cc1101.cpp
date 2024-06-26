/**
 * Ported from mengguang/cc1101
 * 
 * Copyright (c) 2011 panStamp <contact@panstamp.com>
 * Copyright (c) 2016 Tyler Sommer <contact@tylersommer.pro>
 * 
 * This file is part of the CC1101 project.
 * 
 * CC1101 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 * 
 * CC1101 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with CC1101; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 
 * USA
 * 
 * Author: Daniel Berenguer
 * Creation date: 03/03/2011
 */

#include "main.h"
#include "radio.h"
#include "cc1101.h"

#include "pico/time.h"


void CC1101::wait_GDO0_high()
{
  while (!getGDO0state())
    ;
}

void CC1101::wait_GDO0_low()
{
  while (getGDO0state())
    ;
}

void CC1101::wait_Miso()
{
  while (gpio_get(misoPin) > 0)
    ;
}

void CC1101::cc1101_Select()
{
  gpio_put(csPin, 0);
}

void CC1101::cc1101_Deselect()
{
  gpio_put(csPin, 1);
}

int CC1101::getGDO0state()
{
  return gpio_get(gdo0Pin);
}

/**
  * PATABLE
  */
//const byte paTable[8] = {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60};

CC1101::CC1101(uint8_t _csPin, uint8_t _gdo0Pin, uint8_t _misoPin)
{
  csPin = _csPin;
  gdo0Pin = _gdo0Pin;
  misoPin = _misoPin;
  carrierFreq = CFREQ_868;
  channel = CC1101_DEFVAL_CHANNR;
  syncWord[0] = CC1101_DEFVAL_SYNC1;
  syncWord[1] = CC1101_DEFVAL_SYNC0;
  devAddress = CC1101_DEFVAL_ADDR;
}

void CC1101::init(uint8_t freq, uint8_t mode)
{
  carrierFreq = freq;
  workMode = mode;

  gpio_init(csPin);
  gpio_set_dir(csPin, GPIO_OUT);  // Make sure that the SS Pin is declared as an Output

  gpio_init(gdo0Pin);
  gpio_set_dir(gdo0Pin, GPIO_IN); // Config GDO0 as input

  reset(); // Reset CC1101

  // Configure PATABLE
  setTxPowerAmp(PA_LowPower);
}

void CC1101::wakeUp(void)
{
  cc1101_Select();   // Select CC1101
  wait_Miso();       // Wait until MISO goes low
  cc1101_Deselect(); // Deselect CC1101
}

void CC1101::writeReg(uint8_t regAddr, uint8_t value)
{
  uint8_t data[2];
  data[0] = regAddr;
  data[1] = value;

  cc1101_Select();       // Select CC1101
  wait_Miso();           // Wait until MISO goes low
  //SPI.transfer(regAddr); // Send register address
  //SPI.transfer(value);   // Send value

  spi_write_blocking(RADIO_SPI_PORT, &regAddr, 1);
  spi_write_blocking(RADIO_SPI_PORT, &value, 1);

  cc1101_Deselect();     // Deselect CC1101
}

void CC1101::writeBurstReg(uint8_t regAddr, uint8_t *buffer, uint8_t len)
{
  uint8_t addr, i;

  addr = regAddr | WRITE_BURST; // Enable burst transfer
  cc1101_Select();              // Select CC1101
  wait_Miso();                  // Wait until MISO goes low
  //SPI.transfer(addr);           // Send register address

  //for (i = 0; i < len; i++)
    //SPI.transfer(buffer[i]); // Send value
  
  spi_write_blocking(RADIO_SPI_PORT, &addr, 1);
  spi_write_blocking(RADIO_SPI_PORT, buffer, len);

  cc1101_Deselect(); // Deselect CC1101
}

void CC1101::cmdStrobe(uint8_t cmd)
{
  cc1101_Select();   // Select CC1101
  wait_Miso();       // Wait until MISO goes low
  //SPI.transfer(cmd); // Send strobe command
  spi_write_blocking(RADIO_SPI_PORT, &cmd, 1);
  cc1101_Deselect(); // Deselect CC1101
}

uint8_t CC1101::readReg(uint8_t regAddr, uint8_t regType)
{
  uint8_t addr, val;

  addr = regAddr | regType;
  cc1101_Select();          // Select CC1101
  wait_Miso();              // Wait until MISO goes low
  //SPI.transfer(addr);       // Send register address
  //val = SPI.transfer(0x00); // Read result
  spi_write_blocking(RADIO_SPI_PORT, &addr, 1);
  spi_read_blocking(RADIO_SPI_PORT, 0, &val, 1);
  cc1101_Deselect();        // Deselect CC1101

  return val;
}

void CC1101::readBurstReg(uint8_t *buffer, uint8_t regAddr, uint8_t len)
{
  uint8_t addr, i;

  addr = regAddr | READ_BURST;
  cc1101_Select();    // Select CC1101
  wait_Miso();        // Wait until MISO goes low
  //SPI.transfer(addr); // Send register address
  spi_write_blocking(RADIO_SPI_PORT, &addr, 1);

  for (i = 0; i < len; i++)
    //buffer[i] = SPI.transfer(0x00); // Read result byte by byte
    spi_read_blocking(RADIO_SPI_PORT, 0, &buffer[i], 1);

  cc1101_Deselect();                // Deselect CC1101
}

void CC1101::reset(void)
{
  uint8_t data[1] = {CC1101_SRES};

  cc1101_Deselect(); // Deselect CC1101
  sleep_us(5);
  cc1101_Select(); // Select CC1101
  sleep_us(10);
  cc1101_Deselect(); // Deselect CC1101
  sleep_us(41);
  cc1101_Select(); // Select CC1101

  wait_Miso();               // Wait until MISO goes low
  //SPI.transfer(CC1101_SRES); // Send reset command strobe
  spi_write_blocking(RADIO_SPI_PORT, data, 1);
  wait_Miso();               // Wait until MISO goes low

  cc1101_Deselect(); // Deselect CC1101

  setCCregs(); // Reconfigure CC1101
}

void CC1101::setCCregs(void)
{
  writeReg(CC1101_IOCFG2, CC1101_DEFVAL_IOCFG2);
  writeReg(CC1101_IOCFG1, CC1101_DEFVAL_IOCFG1);
  writeReg(CC1101_IOCFG0, CC1101_DEFVAL_IOCFG0);
  writeReg(CC1101_FIFOTHR, CC1101_DEFVAL_FIFOTHR);
  writeReg(CC1101_PKTLEN, CC1101_DEFVAL_PKTLEN);
  writeReg(CC1101_PKTCTRL1, CC1101_DEFVAL_PKTCTRL1);
  writeReg(CC1101_PKTCTRL0, CC1101_DEFVAL_PKTCTRL0);

  // Set default synchronization word
  setSyncWord(syncWord);

  // Set default device address
  setDevAddress(devAddress);

  // Set default frequency channel
  setChannel(channel);

  writeReg(CC1101_FSCTRL1, CC1101_DEFVAL_FSCTRL1);
  writeReg(CC1101_FSCTRL0, CC1101_DEFVAL_FSCTRL0);

  // Set default carrier frequency = 868 MHz
  setCarrierFreq(carrierFreq);

  // RF speed
  if (workMode == MODE_LOW_SPEED)
    writeReg(CC1101_MDMCFG4, CC1101_DEFVAL_MDMCFG4_4800);
  else
    writeReg(CC1101_MDMCFG4, CC1101_DEFVAL_MDMCFG4_38400);

  writeReg(CC1101_MDMCFG3, CC1101_DEFVAL_MDMCFG3);
  writeReg(CC1101_MDMCFG2, CC1101_DEFVAL_MDMCFG2);
  writeReg(CC1101_MDMCFG1, CC1101_DEFVAL_MDMCFG1);
  writeReg(CC1101_MDMCFG0, CC1101_DEFVAL_MDMCFG0);
  writeReg(CC1101_DEVIATN, CC1101_DEFVAL_DEVIATN);
  writeReg(CC1101_MCSM2, CC1101_DEFVAL_MCSM2);
  writeReg(CC1101_MCSM1, CC1101_DEFVAL_MCSM1);
  writeReg(CC1101_MCSM0, CC1101_DEFVAL_MCSM0);
  writeReg(CC1101_FOCCFG, CC1101_DEFVAL_FOCCFG);
  writeReg(CC1101_BSCFG, CC1101_DEFVAL_BSCFG);
  writeReg(CC1101_AGCCTRL2, CC1101_DEFVAL_AGCCTRL2);
  writeReg(CC1101_AGCCTRL1, CC1101_DEFVAL_AGCCTRL1);
  writeReg(CC1101_AGCCTRL0, CC1101_DEFVAL_AGCCTRL0);
  writeReg(CC1101_WOREVT1, CC1101_DEFVAL_WOREVT1);
  writeReg(CC1101_WOREVT0, CC1101_DEFVAL_WOREVT0);
  writeReg(CC1101_WORCTRL, CC1101_DEFVAL_WORCTRL);
  writeReg(CC1101_FREND1, CC1101_DEFVAL_FREND1);
  writeReg(CC1101_FREND0, CC1101_DEFVAL_FREND0);
  writeReg(CC1101_FSCAL3, CC1101_DEFVAL_FSCAL3);
  writeReg(CC1101_FSCAL2, CC1101_DEFVAL_FSCAL2);
  writeReg(CC1101_FSCAL1, CC1101_DEFVAL_FSCAL1);
  writeReg(CC1101_FSCAL0, CC1101_DEFVAL_FSCAL0);
  writeReg(CC1101_RCCTRL1, CC1101_DEFVAL_RCCTRL1);
  writeReg(CC1101_RCCTRL0, CC1101_DEFVAL_RCCTRL0);
  writeReg(CC1101_FSTEST, CC1101_DEFVAL_FSTEST);
  writeReg(CC1101_PTEST, CC1101_DEFVAL_PTEST);
  writeReg(CC1101_AGCTEST, CC1101_DEFVAL_AGCTEST);
  writeReg(CC1101_TEST2, CC1101_DEFVAL_TEST2);
  writeReg(CC1101_TEST1, CC1101_DEFVAL_TEST1);
  writeReg(CC1101_TEST0, CC1101_DEFVAL_TEST0);

  // Send empty packet
  CCPACKET packet;
  packet.length = 0;
  sendData(packet);
}

void CC1101::setSyncWord(uint8_t syncH, uint8_t syncL)
{
  writeReg(CC1101_SYNC1, syncH);
  writeReg(CC1101_SYNC0, syncL);
  syncWord[0] = syncH;
  syncWord[1] = syncL;
}

void CC1101::setSyncWord(uint8_t *sync)
{
  CC1101::setSyncWord(sync[0], sync[1]);
}

void CC1101::setDevAddress(uint8_t addr)
{
  writeReg(CC1101_ADDR, addr);
  devAddress = addr;
}

void CC1101::setChannel(uint8_t chnl)
{
  writeReg(CC1101_CHANNR, chnl);
  channel = chnl;
}

void CC1101::setCarrierFreq(uint8_t freq)
{
  switch (freq)
  {
  case CFREQ_915:
    writeReg(CC1101_FREQ2, CC1101_DEFVAL_FREQ2_915);
    writeReg(CC1101_FREQ1, CC1101_DEFVAL_FREQ1_915);
    writeReg(CC1101_FREQ0, CC1101_DEFVAL_FREQ0_915);
    break;
  case CFREQ_433:
    writeReg(CC1101_FREQ2, CC1101_DEFVAL_FREQ2_433);
    writeReg(CC1101_FREQ1, CC1101_DEFVAL_FREQ1_433);
    writeReg(CC1101_FREQ0, CC1101_DEFVAL_FREQ0_433);
    break;
  case CFREQ_918:
    writeReg(CC1101_FREQ2, CC1101_DEFVAL_FREQ2_918);
    writeReg(CC1101_FREQ1, CC1101_DEFVAL_FREQ1_918);
    writeReg(CC1101_FREQ0, CC1101_DEFVAL_FREQ0_918);
    break;
  default:
    writeReg(CC1101_FREQ2, CC1101_DEFVAL_FREQ2_868);
    writeReg(CC1101_FREQ1, CC1101_DEFVAL_FREQ1_868);
    writeReg(CC1101_FREQ0, CC1101_DEFVAL_FREQ0_868);
    break;
  }

  carrierFreq = freq;
}

void CC1101::setPowerDownState()
{
  // Comming from RX state, we need to enter the IDLE state first
  cmdStrobe(CC1101_SIDLE);
  // Enter Power-down state
  cmdStrobe(CC1101_SPWD);
}

bool CC1101::sendData(CCPACKET packet)
{
  uint8_t marcState;
  bool res = false;

  // Declare to be in Tx state. This will avoid receiving packets whilst
  // transmitting
  rfState = RFSTATE_TX;

  // Enter RX state
  setRxState();

  int tries = 0;
  // Check that the RX state has been entered
  while (tries++ < 1000 && ((marcState = readStatusReg(CC1101_MARCSTATE)) & 0x1F) != 0x0D)
  {
    if (marcState == 0x11) // RX_OVERFLOW
      flushRxFifo();       // flush receive queue
  }
  if (tries >= 1000)
  {
    // TODO: MarcState sometimes never enters the expected state; this is a hack workaround.
    return false;
  }

  sleep_us(500);

  if (packet.length > 0)
  {
    // Set data length at the first position of the TX FIFO
    writeReg(CC1101_TXFIFO, packet.length);
    // Write data into the TX FIFO
    writeBurstReg(CC1101_TXFIFO, packet.data, packet.length);

    // CCA enabled: will enter TX state only if the channel is clear
    setTxState();
  }

  // Check that TX state is being entered (state = RXTX_SETTLING)
  marcState = readStatusReg(CC1101_MARCSTATE) & 0x1F;
  if ((marcState != 0x13) && (marcState != 0x14) && (marcState != 0x15))
  {
    setIdleState(); // Enter IDLE state
    flushTxFifo();  // Flush Tx FIFO
    setRxState();   // Back to RX state

    // Declare to be in Rx state
    rfState = RFSTATE_RX;
    return false;
  }

  // Wait for the sync word to be transmitted
  wait_GDO0_high();

  // Wait until the end of the packet transmission
  wait_GDO0_low();

  // Check that the TX FIFO is empty
  if ((readStatusReg(CC1101_TXBYTES) & 0x7F) == 0)
    res = true;

  setIdleState(); // Enter IDLE state
  flushTxFifo();  // Flush Tx FIFO

  // Enter back into RX state
  setRxState();

  // Declare to be in Rx state
  rfState = RFSTATE_RX;

  return res;
}

uint8_t CC1101::receiveData(CCPACKET *packet)
{
  uint8_t val;
  uint8_t rxBytes = readStatusReg(CC1101_RXBYTES);

  // Any byte waiting to be read and no overflow?
  if (rxBytes & 0x7F && !(rxBytes & 0x80))
  {
    // Read data length
    packet->length = readConfigReg(CC1101_RXFIFO);
    // If packet is too long
    if (packet->length > CCPACKET_DATA_LEN)
      packet->length = 0; // Discard packet
    else
    {
      // Read data packet
      readBurstReg(packet->data, CC1101_RXFIFO, packet->length);
      // Read RSSI
      packet->rssi = readConfigReg(CC1101_RXFIFO);
      // Read LQI and CRC_OK
      val = readConfigReg(CC1101_RXFIFO);
      packet->lqi = val & 0x7F;
      packet->crc_ok =  (val >> 7) & 0x01;  //bitRead(val, 7);
    }
  }
  else
    packet->length = 0;

  setIdleState(); // Enter IDLE state
  flushRxFifo();  // Flush Rx FIFO
  //cmdStrobe(CC1101_SCAL);

  // Back to RX state
  setRxState();

  return packet->length;
}

void CC1101::setRxState(void)
{
  cmdStrobe(CC1101_SRX);
  rfState = RFSTATE_RX;
}

void CC1101::setTxState(void)
{
  cmdStrobe(CC1101_STX);
  rfState = RFSTATE_TX;
}
