#ifndef __WIBRICK_WIRE_H__
#define __WIBRICK_WIRE_H__

#include <Arduino.h>

#define WiWireBaudrate 9600
#define WiWirePacketDataSize 6
#define WiWirePacketSizeWithCmd WiWirePacketDataSize + 1
#define WiWirePacketSize WiWirePacketDataSize + 2
#define WiWireResponseTimeoutUS 200
#define WiWireStartByte 0xA0
#define WiWireNetworkSize 40

/**
 * Known devices
 */
#define DEVICE_TYPE_MOTOR_SG90 0x01
#define DEVICE_TYPE_MOTOR_N20 0x02
#define DEVICE_TYPE_MOTOR_SG90_inifinity 0x03
//////

enum WiCmd {
  WiWire_NOP, //0
  WiWire_PING, //1
  WiWire_SCAN, //2
  WiWire_SET_ADDRESS, //3
  WiWire_GET_SETTINGS, //4
  WiWire_SET_SETTINGS, //5
  WiWire_STATUS, //6
  WiWire_ACTION //7
};

enum WiCmdMotors {
  WiWire_MOTOR_STOP, //0
  WiWire_MOTOR_FORWARD, //1
  WiWire_MOTOR_BACKWARD //2
};

struct WiWirePacket {
  uint8_t addr;
  uint8_t cmd;
  uint8_t data[WiWirePacketDataSize];
};

typedef void (*wifptr)(WiWirePacket p);
typedef void (*wisfptr)();

#endif