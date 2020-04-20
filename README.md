# WiBrick-Wire

WiBrick Wire lib. Master/Slave implementation for device bus.

This is simple 1-Wire protocol based on half duplex UART interface.

## Basics
Master always send `WiWire_PING` requests to bus for every address (from 0 up to `WiWireNetworkSize`). 
If Slave device receive message with his address it parse request and can send answer ony in next `WiWire_PING` request.
If Slave not respond in `WiWireResponseTimeoutUS` Master send `WiWire_PING` to next address

### Documentation and implementation still under development
### Contributors welcome! :)
