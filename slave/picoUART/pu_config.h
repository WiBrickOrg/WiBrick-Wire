// picoUART configuration

#ifndef PU_BAUD_RATE
// #define PU_BAUD_RATE 115200L            // default baud rate
#define PU_BAUD_RATE 9600L            // default baud rate
#endif

// port and bit for Tx and Rx - can be same
#ifndef PU_TX
#define PU_TX B,2
#define PU_RX B,2
#endif

// disable interrupts during Tx and Rx
#define PU_DISABLE_IRQ 1

