#include "wibrick-wire.h"
#include "wiwire-slave.h"

// #define DEBUG_WISLAVE

#define USE_PU SIGNATURE_0 == 0x1E && SIGNATURE_1 == 0x90 && SIGNATURE_2 == 0x07 // ATTiny13
#if USE_PU
#include "picoUART/picoUART.h"
#endif

WiWireSlave::WiWireSlave(void)
{
#ifdef DEBUG_WISLAVE
  putx(0xF1);
#endif
}

void WiWireSlave::update()
{
#if USE_PU
  uint8_t rxByte = purx();
#else
  uint8_t rxByte = Serial.read();
#endif

  // If Receiver ready and we receive start byte
  if (rxByte == WiWireStartByte && this->dataBufferState == WiWireSlaveNET_READY)
  {
    this->dataBufferState = WiWireSlaveNET_ADDR;
    this->dataBufferPos = 0;
    return;
  }

  // Check address
  if (this->dataBufferState == WiWireSlaveNET_ADDR && rxByte == this->address)
  { // for me
    this->dataBufferState = WiWireSlaveNET_RECV;
#ifdef DEBUG_WISLAVE
    putx(0xD1);
#endif
    return;
  }
  else if (this->dataBufferState == WiWireSlaveNET_ADDR)
  { // Not for me
    this->dataBufferState = WiWireSlaveNET_READY;
#ifdef DEBUG_WISLAVE
    putx(0xD2);
#endif
    return;
  }

  // Receive data
  if (this->dataBufferState == WiWireSlaveNET_RECV)
  {
    dataBuffer[dataBufferPos] = rxByte;
    dataBufferPos++;
#ifdef DEBUG_WISLAVE
    putx(0xD3);
#endif
    if (dataBufferPos >= WiWirePacketSizeWithCmd)
    {
#ifdef DEBUG_WISLAVE
      putx(0xD4);
#endif
      this->dataBufferState = WiWireSlaveNET_CRC;
      return;
    }
  }

  // Check CRC
  if (this->dataBufferState == WiWireSlaveNET_CRC)
  {
#ifdef DEBUG_WISLAVE
    putx(0xD5);
#endif
    uint8_t crc = this->address;
    for (uint8_t i = 0; i < WiWirePacketSizeWithCmd; i++)
    {
      crc = crc + this->dataBuffer[i];
    }
#ifdef DEBUG_WISLAVE
    putx(0xD6);
#endif

    if (crc != rxByte) // Wrong crc
    {
#ifdef DEBUG_WISLAVE
      putx(0xD7);
#endif
      this->dataBufferState = WiWireSlaveNET_READY;
      this->clearBuffer(this->dataBuffer);
      return;
    }

#ifdef DEBUG_WISLAVE
    putx(0xDA);
#endif
    if (this->dataBuffer[0] != WiWire_PING)
    {
      this->onReceiveCallback();
      this->clearBuffer(this->dataBuffer);
      this->dataBufferState = WiWireSlaveNET_READY;
      return;
    }
#ifdef DEBUG_WISLAVE
    putx(0xD8);
#endif

    if (this->dataBuffer[0] == WiWire_PING && this->readyToSend)
    {
#ifdef DEBUG_WISLAVE
      putx(0xD9);
#endif

#if USE_PU
      putx(WiWireStartByte);
      putx(this->address);
      uint8_t crc = this->address;
      for (uint8_t i = 0; i < WiWirePacketDataSize; i++)
      {
        putx(this->sendBuffer[i]);
        crc += this->sendBuffer[i];
      }
      putx(crc);
#else
      Serial.write(WiWireStartByte);
      Serial.write(WiWireStartByte);
      Serial.write(this->address);
      uint8_t crc = this->address;
      for (uint8_t i = 0; i < WiWirePacketDataSize; i++)
      {
        Serial.write(this->sendBuffer[i]);
        crc += this->sendBuffer[i];
      }
      Serial.write(crc);
#endif
    }
    this->readyToSend = false;
    this->clearBuffer(this->sendBuffer);
    this->clearBuffer(this->dataBuffer);
    this->dataBufferState = WiWireSlaveNET_READY;
    return;
  }
}

void WiWireSlave::onReceive(wisfptr callback)
{
  this->onReceiveCallback = callback;
}

void WiWireSlave::clearBuffer(uint8_t *buff)
{
  for (uint8_t i = 0; i < WiWirePacketSizeWithCmd; i++)
  {
    buff[i] = 0;
  }
}
