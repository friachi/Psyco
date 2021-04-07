/*!
 *  @file psyco.h
 *  
 *  This is a library for PSYCO programmable synth controller dev board having:
 *  - 21x analog inputs (10-bits): 0-17
 *  - 3x GPIO (INPUT,INPUT_PULLUP, OUTPUT): {0:A3, 1:A4, 2:A5}
 *  - 4x CV out (5v @ 12-bit): 0-3
 *  - 4x buffered Gate out: 0-3 {0:D6, 1:D7, 2:D8, 3:D9}
 *  - 1x unbuffered digital out: D10
 *  - MINI IN  (D0)
 *  - MIDI OUT (D1)
 *
 *  Designed specifically to work with 
 *  - Arduino Nano v3.x
 *  - 10-bit ADC: MCP3008 
 *  - 12-bit DAC: MCP-49xx
 *  - Adafruit rotary encoder (24 step incremental quadrature + switch)
 *  
 *  MIT license, all text above must be included in any redistribution
 */

#ifndef Psyco_h
#define Psyco_h

#include <Arduino.h>
#include <SPI.h>

class Psyco {
  
  public:
    bool begin(SPIClass *theSPI = &SPI);  
    unsigned int readAnalog(byte channel);
    void writeCV(byte channel, unsigned int value);
    void writeGate(byte channel, byte value);
    int readEncoderDirection();
    byte readButtonState();
    void pinModeGPIO(byte channel, byte mode);
    byte digitalReadGPIO(byte channel);
    unsigned int analogReadGPIO(byte channel);
    void digitalWriteGPIO(byte channel, byte value);
      
  private:
    SPIClass *_spi;
    byte _ADC1_CS;
    byte _ADC2_CS;

    byte _DAC1_CS;
    byte _DAC2_CS;
    unsigned int _channel_a;
    unsigned int _channel_b;

    byte _GATE1;
    byte _GATE2;
    byte _GATE3;
    byte _GATE4;

    byte _encoder_A;
    byte _encoder_B;
    byte _encoder_SW;
    unsigned char _encoder_A_value;
    unsigned char _encoder_B_value;
    unsigned char _encoder_A_prev;
    unsigned long _currentTime;
    unsigned long _loopTime;
    int _tick;
    unsigned char _SW_value;
    unsigned char _SW_prev;
    unsigned long _pressedTime;
    unsigned long _releasedTime;
    const int _PRESS_TIME_REF = 500;
    unsigned long _currentTime_sw;
    unsigned long _loopTime_sw;
    byte _sw_state;
   
    unsigned int readADC(byte ADC_CS,byte channel);
    void writeDAC(byte DAC_CS, byte channel, unsigned int value);
    byte mapGPIO(byte channel);
  
};

#endif
