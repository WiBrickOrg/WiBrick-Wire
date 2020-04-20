#ifndef __WIBRICK_WIRE_SLAVE_H__
#define __WIBRICK_WIRE_SLAVE_H__

#include "wibrick-wire.h"

#define WiWireSlaveNET_READY 0
#define WiWireSlaveNET_ADDR 1
#define WiWireSlaveNET_RECV 2
#define WiWireSlaveNET_CRC 3

class WiWireSlave
{
public:
  uint8_t sendBuffer[WiWirePacketSizeWithCmd];
  uint8_t dataBuffer[WiWirePacketSizeWithCmd];
  bool readyToSend = false;
  uint8_t address;

  WiWireSlave();
  void update();
  void onReceive(wisfptr callback);
  void clearBuffer(uint8_t * buff);

private:
  uint8_t dataBufferState = WiWireSlaveNET_READY;
  uint8_t dataBufferPos = 0;
  wisfptr onReceiveCallback;
};

#endif