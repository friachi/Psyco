#include <MIDI.h>

/*
 * This example is derived from examples from Arduino MIDI library with some changes to have both MIDI input and output at same time. 
 * Nothing special related to Psyco, except some precision info:
 *  - MIDI TX/RX are connected to the Arduino's Nano hardware serial TX1/RX0 ports, (pin0/pin1) respectively. Accordingly 'Serial monitor' can't be used since the latter uses same hardware serial port over USB. 
 *      - Alternatively, a software serial port can be used for MIDI, freeing up the usb for Serial Monitor. Check MIDI library for example 'AltPinSerial'.
 *  - When uploading Code to the board, make sure thet MIDI switch is in OFF position, this to avoid interfering with code upload over USB
 * 
 * The example:
 * For Input: when MIDI noteON message is received on MIDI IN port, LED 13 on the board will be ON, and OFF when noteOFF MIDI message is recieved
 * For output: a note ON/OFF will be sent each 1 sec to MIDI OUT
 * 
 */

MIDI_CREATE_DEFAULT_INSTANCE();

int noteState = LOW;
unsigned long previousMillis = 0; 
const long interval = 1000;

void setup()
{
    MIDI.setHandleNoteOn(handleNoteOn);  
    MIDI.setHandleNoteOff(handleNoteOff);

    // Listen to all channels (alternatively, pass an integer channel number)
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.turnThruOff(); // prevent input midi messages from being repeated on the midi output
    
    pinMode(13,OUTPUT);
}

void loop()
{
    // handle MIDI IN
    
    // Call MIDI.read the fastest you can for real-time performance.
    // when message is received, the registerd callbacks will be automatically called,
    // in our example: handleNoteOn() and/or handleNoteOff()
    MIDI.read();


    //handle MIDI OUT
    // below is sending a note on/off each 1sec without using a delay function
    // this to avoid blocking the loop which contains the time-critical MIDI.read()
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
      
      // save the last time you set the note ON
      previousMillis = currentMillis;
  
      // if the NOTE is off turn it on and vice-versa:
      if (noteState == LOW) {
        noteState = HIGH;
        MIDI.sendNoteOn(42, 127, 1);
      } else {
        noteState = LOW;
        MIDI.sendNoteOff(42, 0, 1);
      }
  }
   
}

//-------------- MIDI IN callbacks -------------//

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
    // Do something when the note is pressed
    digitalWrite(13,HIGH);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
    // Do something when the note is released.
    digitalWrite(13,LOW);
}
