#ifndef __WIBRICK_WIRE_MAKESTER_H__
#define __WIBRICK_WIRE_MAKESTER_H__

#include "../wibrick-wire.h"

#define WiWireMasterSendBufferSize 4
#define WiWireMasterReceiveBufferSize 4

#define WiWireNET_READY 0
#define WiWireNET_WAIT 1
#define WiWireNET_RECV 2

class WiWireMaster
{
public:
  WiWireMaster();
  void scan();
  void update();
  void send(uint8_t addr, WiCmd cmd);
  void send(WiWirePacket p);
  void write(uint8_t data[WiWirePacketSize]);
  void onReceive(wifptr callback);

private:
  WiWirePacket sendBuffer[WiWireMasterSendBufferSize];
  WiWirePacket receiveBuffer[WiWireMasterReceiveBufferSize];
  uint8_t sendBufferPos = 0;
  uint8_t sendBufferCount = 0;
  uint8_t receiveBufferPos = 0;
  uint8_t receiveBufferBytePos = 0;
  uint8_t currentAddress = 0;
  uint8_t networkStatus = WiWireNET_READY;
  uint32_t lastSendMicros = 0;
  wifptr onReceiveCallback;

  bool checkPacket(uint8_t data[WiWirePacketSize], uint8_t rCrc);
  void clearPacketData(uint8_t * data);
};

#endif