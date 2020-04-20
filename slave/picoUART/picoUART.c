/* optimized half-duplex high-speed AVR serial uart
 * Ralph Doncaster 2020 open source MIT license
 *
 * picoUART is accurate to the cycle (+- 0.5 cycle error)
 * 0.64% error at 115.2k/8M and 0.4% error at 115.2k/9.6M
 *
 * define PU_BAUD_RATE before #include to change default baud rate 
 *
 * capable of single-pin operation (PU_TX = PU_RX) as follows:
 * connect MCU pin to host RX line, and a 1.5k-4.7k resistor between
 * host RX and TX line.  Note this also gives local echo.
 * 
 * 20200123 version 0.5
 * 20200123 version 0.6 - improve inline asm
 * 20200201 version 0.7 - use push/pull during tx 
 * 20200203 version 0.8 - add prints, prefix globals with PU_
 * 20200209 version 0.9 - rewrite in mostly C 
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "pu_config.h"                  // baud rate & gpio config
// #include "picoUART.h"

// use static inline functions for type safety
extern inline float PUBIT_CYCLES() {return F_CPU/(PU_BAUD_RATE*1.0);}

// delay based on cycle count of asm code + 0.5 for rounding
extern inline int PUTXWAIT() {return PUBIT_CYCLES() - 7 + 0.5;}
extern inline int PURXWAIT() {return PUBIT_CYCLES() - 5 + 0.5;}

// correct for PURXWAIT skew in PURXSTART calculation
// skew is half of 7 delay intervals between 8 bits
extern inline float PUSKEW() {
    return (PUBIT_CYCLES() - (int)(PUBIT_CYCLES() + 0.5)) * 3.5;
}
// Time from falling edge of start bit to sample 1st bit is 1.5 *
// bit-time. Subtract 2c for sbic, 1 for ldi, 1 for lsr, and PURXWAIT.
// Subtract 1.5 cycles because start bit detection is accurate to
// +-1.5 cycles.  Add 0.5 cycles for int rounding, and add skew.
extern inline int PURXSTART() {
    return (PUBIT_CYCLES()*1.5 -5.5 -PURXWAIT() + 0.5 + PUSKEW());
}

void pu_disable_irq() {
#ifdef PU_DISABLE_IRQ
    cli();
#endif
}

void pu_enable_irq() {
#ifdef PU_DISABLE_IRQ
    sei();
#endif
}

// I/O register macros
#define GBIT(r,b)       b
#define GPORT(r,b)      (PORT ## r)
#define GDDR(r,b)       (DDR ## r)
#define GPIN(r,b)       (PIN ## r)
#define get_bit(io)     GBIT(io)
#define get_port(io)    GPORT(io)
#define get_ddr(io)     GDDR(io)
#define get_pin(io)     GPIN(io)

#define PUTXBIT     get_bit(PU_TX)
#define PUTXPORT    get_port(PU_TX)
#define PUTXDDR     get_ddr(PU_TX)
#define PURXBIT     get_bit(PU_RX)
#define PURXPIN     get_pin(PU_RX)

#define _concat(a, b) (a ## b)
#define concat(a, b)  _concat(a, b)

typedef union {
    unsigned i16;
    struct { uint8_t lo8; uint8_t hi8; };
} frame;

typedef union {
    uint8_t c;
    struct {
         int b0:1; int b1:1; int b2:1; int b3:1; int b4:1; int b5:1; int b6:1; int b7:1;
    };
} bits;


void putx(uint8_t c)
{
    // pin frame to r25:24 since c will already be in r24 for ABI
    register frame f asm("r24");
    f.lo8 = c;

    PUTXPORT &= ~(1<<PUTXBIT);          // disable pullup
    pu_disable_irq();
    PUTXDDR |= (1<<PUTXBIT);            // low for start bit

    // hi8 b1 set for stop bit, b2 set for line idle state
    f.hi8 = 0x03;
    bits psave = {PUTXPORT};

    //do {
    txbit:
        __builtin_avr_delay_cycles(PUTXWAIT());
        // macro hack to set correct bit
        concat(psave.b, PUTXBIT) = f.lo8 & 0x01 ? 1 : 0;
        f.i16 >>= 1;
        PUTXPORT = psave.c; 
    // tx more bits if f.lo8 not equal to 0
    asm goto ("brne %l[txbit]" :::: txbit);
    //} while (f.lo8);
    pu_enable_irq();
    PUTXDDR &= ~(1<<PUTXBIT);            // revert to input mode
}

uint8_t purx()
{
    // wait for idle state (high) to avoid reading last bit of last frame
    // except for at very high baud rates ( <= 11 cycles per bit )
    if ( PUBIT_CYCLES() + 0.5 > 11 )
        while (! (PURXPIN & (1<<PURXBIT)) ); 

    pu_disable_irq();

    // wait for start bit
    while ( PURXPIN & (1<<PURXBIT) ); 
    uint8_t c = 0x80;                   // bit shift counter
    // wait for the middle of the start bit
    __builtin_avr_delay_cycles(PURXSTART());

    rxbit:
        __builtin_avr_delay_cycles(PURXWAIT());
        c /= 2;                         // shift right
        if ( PURXPIN & (1<<PURXBIT) )
            c |= 0x80;
    // read bits until carry set
    asm goto ("brcc %l[rxbit]" :::: rxbit);

    pu_enable_irq();
    return c;
}

void prints(const char* s)
{
    char c;
    while(c = *s++) putx(c);
}

