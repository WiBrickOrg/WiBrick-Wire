#include "../wibrick-wire.h"
#include "wiwire-master.h"

WiWireMaster::WiWireMaster(void)
{
  Serial.begin(WiWireBaudrate);
}

void WiWireMaster::update()
{
  bool rxAvailable = Serial.available();
  uint8_t rxByte;
  if (rxAvailable) {
    rxByte = Serial.read();
  }

  // Detect send timeout
  if (this->networkStatus == WiWireNET_WAIT && (millis() - this->lastSendMicros >= WiWireResponseTimeoutUS)) {
    this->networkStatus = WiWireNET_READY;
  } else if (this->networkStatus == WiWireNET_WAIT && rxAvailable) {
    if (rxByte == WiWireStartByte) {
      this->networkStatus = WiWireNET_RECV;
    } else { // Wrong packet start
      this->networkStatus = WiWireNET_READY;
    }
    this->lastSendMicros = micros();
    return;
  }
  
  // Receive packet
  if (this->networkStatus == WiWireNET_RECV && rxAvailable) {
    if (this->receiveBufferBytePos == 0) {
      this->receiveBuffer[this->receiveBufferPos].addr = rxByte;
    }
    if (this->receiveBufferBytePos == 1) {
      this->receiveBuffer[this->receiveBufferPos].cmd = rxByte;
    }
    if (this->receiveBufferBytePos > 1 && this->receiveBufferBytePos < WiWirePacketSize) {
      this->receiveBuffer[this->receiveBufferPos].data[this->receiveBufferBytePos - 2] = rxByte;
    }
    this->receiveBufferBytePos++;

    if (this->receiveBufferBytePos >= WiWirePacketSize) {
      uint8_t crc = rxByte;
      // if (!this->checkPacket((uint8_t *)&this->receiveBuffer[this->receiveBufferPos], crc)) { // Bad packet
        // WiWirePacket p;
        // this->receiveBuffer[this->receiveBufferPos] = p;
      // } else {
        this->onReceiveCallback(this->receiveBuffer[this->receiveBufferPos]); // Notify about new packet
        this->receiveBufferPos++;
        if (this->receiveBufferPos >= WiWireMasterReceiveBufferSize) {
          this->receiveBufferPos = 0;
        }
      // }
      this->networkStatus = WiWireNET_READY;
    }
    this->lastSendMicros = millis();
    return;
  }

  // Go to next address
  if (this->networkStatus == WiWireNET_READY) {
    if (this->sendBufferCount) {
      for (uint8_t i = 0; i < this->sendBufferCount; i++)
      {
        this->write((uint8_t *) &this->sendBuffer[i]);
      }
      this->sendBufferCount = 0;
      this->sendBufferPos = 0;
    }

    WiWirePacket p;
    p.addr = this->currentAddress;
    p.cmd = WiWire_PING;
    this->clearPacketData(p.data);
    this->write((uint8_t *) &p);

    this->networkStatus = WiWireNET_WAIT;
    this->lastSendMicros = millis();

    this->currentAddress++;
    if (this->currentAddress >= WiWireNetworkSize) {
      this->currentAddress = 0;
    }
  }
}

void WiWireMaster::write(uint8_t data[WiWirePacketSize])
{
  Serial.write(WiWireStartByte);
  uint8_t crc = 0;
  for (uint8_t i = 0; i < WiWirePacketSize; i++ ) {
    Serial.write(data[i]);
    crc += data[i];
  }
  Serial.write(crc);
}

bool WiWireMaster::checkPacket(uint8_t data[WiWirePacketSize], uint8_t rCrc)
{
  uint8_t crc = 0;
  for (uint8_t i = 0; i < WiWirePacketSize; i++ ) {
    Serial.write(data[i]);
    crc += data[i];
  }
  return crc == rCrc;
}

void WiWireMaster::send(WiWirePacket p)
{
  this->sendBuffer[this->sendBufferPos] = p;
  this->sendBufferPos++;
  if (this->sendBufferPos >= WiWireMasterSendBufferSize) {
    this->sendBufferPos = 0;
  }
  this->sendBufferCount++;
}

void WiWireMaster::send(uint8_t addr, WiCmd cmd)
{
  WiWirePacket p;
  p.addr = addr;
  p.cmd = cmd;
  this->clearPacketData(p.data);
  this->send(p);
}

void WiWireMaster::onReceive(wifptr callback)
{
  this->onReceiveCallback = callback;
}

void WiWireMaster::clearPacketData(uint8_t * data)
{
  for (uint8_t i = 0; i < WiWirePacketDataSize; i++) 
  {
    data[i] = 0;
  }
}