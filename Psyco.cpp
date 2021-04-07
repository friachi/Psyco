/*!
 *  @file psyco.cpp
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


#include "Psyco.h"
#include <SPI.h>

bool Psyco::begin(SPIClass *theSPI) {

  // ADC
  this->_ADC1_CS = 2;
  this->_ADC2_CS = 3;
  pinMode(this->_ADC1_CS, OUTPUT);
  pinMode(this->_ADC2_CS, OUTPUT);
  digitalWrite(this->_ADC1_CS, HIGH);
  digitalWrite(this->_ADC2_CS, HIGH);

  // DAC (i.e CVs)
  this->_DAC1_CS = 4;
  this->_DAC2_CS = 5;
  pinMode(this->_DAC1_CS, OUTPUT);
  pinMode(this->_DAC2_CS, OUTPUT);
  digitalWrite(this->_DAC1_CS, HIGH);
  digitalWrite(this->_DAC2_CS, HIGH);
  this->_channel_a = 0B0001000000000000; //ch1, buffer off, gain x2, output on
  this->_channel_b = 0B1001000000000000; //ch2, buffer off, gain x2, output on

  // GATE
  this->_GATE1 = 6;
  this->_GATE2 = 7;
  this->_GATE3 = 8;
  this->_GATE4 = 9;
  pinMode(this->_GATE1, OUTPUT);
  pinMode(this->_GATE2, OUTPUT);
  pinMode(this->_GATE3, OUTPUT);
  pinMode(this->_GATE4, OUTPUT);
  digitalWrite(this->_GATE1, LOW);
  digitalWrite(this->_GATE2, LOW);
  digitalWrite(this->_GATE3, LOW);
  digitalWrite(this->_GATE4, LOW);

  // Encoder
  this->_encoder_A = A1;
  this->_encoder_B = A0;
  pinMode(this->_encoder_A, INPUT_PULLUP);
  pinMode(this->_encoder_B, INPUT_PULLUP);
  this->_currentTime = millis();
  this->_loopTime = this->_currentTime;
  this->_encoder_A_prev = 0;
  this->_tick = 0;

  // button
  this->_encoder_SW = A2;
  pinMode(this->_encoder_SW, INPUT_PULLUP);
  this->_SW_prev = LOW;
  this->_currentTime_sw = millis();
  this->_loopTime_sw = this->_currentTime_sw;
  this->_pressedTime = 0;
  this->_releasedTime = 0;
  this->_sw_state = 0;

  // GPIO, default to INPUT
  // A3 to A5: can be changes using pinModeGPIO (to be INPUT, INPUT_PULLUP, or OUTPUT)
  // A6 to A7: can only be set to INPUT (no INPUT_PULLUP, no OUTPUT this due to limitaion on Arduino Nano)
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);

  // SPI
  _spi = theSPI;
  _spi->begin();
  

  return true;
}


unsigned int Psyco::readAnalog(byte channel){

  if ((channel < 0) || (channel > 17))
    return -1;

  if ((channel >= 0) && (channel <= 7)){
    return this->readADC(this->_ADC1_CS, channel);
  }
  else if ((channel >= 8) && (channel <= 15)){
    return this->readADC(this->_ADC2_CS, (channel-8));
  }
  else if (channel == 16){
    return ::analogRead(A6);
  }
  else if (channel == 17){
    return ::analogRead(A7);
  }
   
}

byte Psyco::mapGPIO(byte channel){

    // maps psyco GPIO channels numbering (0 to 2) to arduino numbering (A3 to A5)

    if ((channel < 0) || (channel > 3))
       return;

    byte p;
    switch (channel) {
        case 0:
          p = A3;
          break;
        case 1:
          p = A4;
          break;
        case 2:
          p = A5;
          break;
      }

      return p;
}


void Psyco::pinModeGPIO(byte channel, byte mode){

    if ((channel < 0) || (channel > 3))
       return;

    byte p = this->mapGPIO(channel);

    if ((mode == INPUT) || (mode == INPUT_PULLUP) || (mode == OUTPUT))
        ::pinMode(p, mode);
    else
        ::pinMode(p, INPUT);   // default to input

}

byte Psyco::digitalReadGPIO(byte channel){
    if ((channel < 0) || (channel > 3))
       return;

    return ::digitalRead(this->mapGPIO(channel));
}

unsigned int Psyco::analogReadGPIO(byte channel){

    if ((channel < 0) || (channel > 3))
       return;

    return ::analogRead(this->mapGPIO(channel));
}

void Psyco::digitalWriteGPIO(byte channel, byte value){

    if ((channel < 0) || (channel > 3))
       return;

    ::digitalWrite(this->mapGPIO(channel), value);
}


unsigned int Psyco::readADC(byte ADC_CS,byte channel){
  
  // returns current reading of a given 'ADC' seen on a given 'channel'
  
  if ((channel < 0) || (channel > 7))
    return -1;

  unsigned int output;  
  byte commmand = (0x01 << 7) | (channel << 4); // 0B 1CCC 0000: CCC is channel number 0 - 7
  byte b0, MSB, LSB;

  _spi->beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  digitalWrite(ADC_CS,LOW);
  
  b0 = _spi->transfer(0x01);
  MSB = _spi->transfer(commmand);
  LSB = _spi->transfer(0x00);

  digitalWrite(ADC_CS,HIGH);
  _spi->endTransaction();
  
  output = (MSB & 0x03) << 8 | LSB;

  return output;
  
}

void Psyco::writeCV(byte channel, unsigned int value){
  
  if ((channel < 0) || (channel > 3))
    return;

  if (value > 4095)
    value = 4095;

  if ((channel >= 0) && (channel <= 1)){
    this->writeDAC(this->_DAC1_CS, channel, value);
  }
  else if ((channel >= 2) && (channel <= 3)){
    this->writeDAC(this->_DAC2_CS, (channel-2), value);
  }

}

void Psyco::writeDAC(byte DAC_CS, byte channel, unsigned int value){

 if ((channel < 0) || (channel > 1))
    return;
 
 
 unsigned int chan_x;
 
 if(channel == 0) {
  chan_x = this->_channel_a;
 }
 else{
  chan_x = this->_channel_b;
 }
 
 _spi->beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
 digitalWrite(DAC_CS,LOW);   // start comm.
 _spi->transfer16(chan_x | value);
 digitalWrite(DAC_CS,HIGH);  // data trasfered
 _spi->endTransaction();

}

void Psyco::writeGate(byte channel, byte value){

  if ((channel < 0) || (channel > 3))
      return;

  switch (channel) {
    case 0:
      ::digitalWrite(this->_GATE1, value);
      break;
    case 1:
      ::digitalWrite(this->_GATE2, value);
      break;
    case 2:
      ::digitalWrite(this->_GATE3, value);
      break;
    case 3:
      ::digitalWrite(this->_GATE4, value);
      break;
  }
}

int Psyco::readEncoderDirection(){

  this->_tick = 0;
  this->_currentTime = millis();
  if (this->_currentTime < this->_loopTime) this->_loopTime = 0; // handle when millis wrap around to zero
  if(this->_currentTime >= (this->_loopTime + 3)){
    // 4ms since last check of encoder, that is  around 300Hz for a 24 step encoder
    this->_encoder_A_value = digitalRead(this->_encoder_A);
    this->_encoder_B_value = digitalRead(this->_encoder_B);
    if((!this->_encoder_A_value) && (this->_encoder_A_prev)){
      // A has gone from high to low
      if(this->_encoder_B_value) {
        // B is LOW so counter-clockwise
        this->_tick = -1;
      }
      else {
        // B is HIGH so clockwise
        this->_tick = 1;
      }

    }

    this->_encoder_A_prev = this->_encoder_A_value;
    this->_loopTime = this->_currentTime;
  }
  return this->_tick;

}

byte Psyco::readButtonState(){
  this->_sw_state = 0;
  this->_currentTime_sw = millis();
  if(this->_currentTime_sw >= (this->_loopTime_sw + 50)){
    this->_SW_value = digitalRead(this->_encoder_SW);

    // check if button is pressed
    if(this->_SW_prev == HIGH && this->_SW_value == LOW)
      this->_pressedTime = millis();

    // check button is released
    else if(this->_SW_prev == LOW && this->_SW_value == HIGH) {
      this->_releasedTime = millis();

      long _pressDuration = this->_releasedTime - this->_pressedTime;

      if( _pressDuration < this->_PRESS_TIME_REF)
        this->_sw_state = 1;  // short press
      else
      // long press
        this->_sw_state = 2;  // long press

    }

    // save the the last state
    this->_SW_prev = this->_SW_value;
    this->_loopTime_sw = this->_currentTime_sw;
  }

  return this->_sw_state;
}
