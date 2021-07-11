/* Percussive Tunnable Keyboard
 * 13 piezo sensors (i.e an ocrave) are mapped to generate notes (keys layout is Isomorphic). 
 * The notes tunning is customizable and recallable during play mode
 * FSR sensor allows momentary pitch up of any note, while a GATE hold button is available to keep note playing after initial triggering
 * 
 * Usage:
 * On first use, the tunning loaded is the one provided in the code (i.e default tunning)
 * Long press on Rotary Encoder to cycle through PSYCO modes: 'Alternative tunning' (orange), 'Reference tunning' (red), and 'playing mode' (blue) .Any time a note is played it turns green.
 * While in any of the tunning modes:
 *  - The rotary encoder can be used to tune the notes up or down
 *  - Doing 5 short-clicks will reset its tunning to default tunning (leds will blink)
 *    - if a note is being tunned, only this note tunning will be reset
 *    - if no note is being tunned, all notes tunning will be reset
 *  - A long press will exist this tunning mode AND save it to long term memory
 * While in 'Playing' mode
 *  - Doing 5 short-clicks will reset Alternative & Reference tunning to default tunnings
 *  - Alternative tunning can be selectively activated/decativated for any note (toggle orange/green). 
 *  - When alternative tunnign is activated for some notes, you can use 'scale mode' button to toggle all notes on/off between reference and alternative tunning.
 * To change notes and leds layout from left to right handed, uncomment 3 arrays default_ref_tuning, default_alt_tuning, ledMap. After upload make sure to cycle at least once through all tunning modes in orde to update memeory
 * 
 * Authors: Fahd AL Riachi, Khaled Yassine
 * KHAFA - 2021 
 */



#include <Psyco.h>
#include <FastLED.h>
#include <EEPROM.h>

#define NUM_LEDS 64             // total number of leds on ledstrip
#define DATA_PIN 10             // ledstrip control pin
#define ALT_PIN 13              // (INPUT2, pin 5): Alternative note selector (Ribbon/Ladder)
#define FSR_INPUT 14            // (INPUT2, pin 6): Note momenray pitch bend (FSR)
#define GATE_HOLD_PIN 15        // (INPUT2, pin 7): Gate Hold (button)
#define SCALE_MODE_PIN 17       // (INPUT3, pin 4): Alternative notes master toggle (button)
#define NUMBER_OF_NOTES 13      // Total number of notes/inputs connected
#define ADDR_REF_TUNNING 0      // long-term memory storage location for reference tunning
#define ADDR_ALT_TUNNING 100    // long-term memory storage location for Alternative tunning
#define ADDR_EEPROM_MARKER 200  // long-term memory Flag address to verify if tunning was ever stored or not
#define MARKER_EEPROM 123       // long-term memory Flag value to verify if tunning was ever stored or not


Psyco psyco;

// inputs, frequencies, and tunner

/* position vs notes
 * 0 : do (1)
 * 1 : re
 * 2 : mi
 * 3 : sol b 
 * 4 : la b
 * 5 : si b
 * 6 : do (2)
 * 7 : re b
 * 8 : mi b
 * 9 : fa
 * 10 : sol
 * 11 : la
 * 12 : si
 */


// array used when tunning is reset using 'resetTunning()'
//left handed
int default_ref_tuning[] = {0,137,267,405,540,675,808,66,203,337,473,602,744};
int default_alt_tuning[] = {0,137,222,405,540,675,808,66,203,337,473,602,711};

//right handed
//int default_ref_tuning[] = {744,602,473,337,203,66,808,675,540,405,267,137,0};
//int default_alt_tuning[] = {711,602,473,337,203,66,808,675,540,405,222,137,0};

// array used in in live play or tunning (its values are eventually saved to EEPROM)
int ref_tuning[NUMBER_OF_NOTES];
int alt_tuning[NUMBER_OF_NOTES]; 
int input[NUMBER_OF_NOTES];
boolean tune_on[NUMBER_OF_NOTES];
boolean alt_on[NUMBER_OF_NOTES];
boolean scale_alt_on[NUMBER_OF_NOTES];
int tunnningResetCounter = 0;   // keeps track of single button clicks
int nbResetClicks = 5;
int tunerIncrement = 3;         // how many cv units to increase when turning rotary encoder
boolean SCALE_MODE = false;
int alt_buttons_debound = 120;  // debound time for alternative tunning buttons (single and master)

// gate variables
int threshold = 100;
const int gateLength = 30;    //default gate open time in milliseconds
boolean HOLDING = false;
boolean GATE_ON = false;
long lastChangeTime = 0;

// state machine variables
byte buttonState;
int encoderDirection;
enum mode {REF_TUNING, ALT_TUNING, PLAYING};
mode psyco_mode = PLAYING;

// led variables
int brightness = 100;
CHSV alt_color = CHSV( 35, 255, brightness);   // orange hue
CHSV ref_color = CHSV( 0, 255, brightness);    // red hue
CHSV play_color = CHSV( 130, 255, brightness); // blue hue
CHSV note_select_color = CHSV( 96, 255, brightness); // green hue
CRGB leds[NUM_LEDS];
int ledMap[NUMBER_OF_NOTES] = {17,19,21,41,43,45,47,10,12,32,34,36,38};  // left handed
//int ledMap[NUMBER_OF_NOTES] = {22,20,18,46,44,42,40,13,11,39,37,35,33};  // right handed

void setup() {
  
  Serial.begin(9600);
  psyco.begin();
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.clear();        // clear led display
  ledScene(psyco_mode);   // set led display to current default mode (i.e PLAYING)
  tune_off(-1);           // make sure none of the notes are in tunning mode
  alt_off(-1);            // make sure all alternative notes are turned off
  initializeMemory();     // Load stored tuning (if available in EEPROM)
  
  Serial.println(F("[ PSYCO has started ]"));
  printTuningMode();
  
}

void loop()
{

// reading rotary button state
buttonState = psyco.readButtonState();

// reading note/inputs values
for (int i=0; i<NUMBER_OF_NOTES;i++){
  input[i] = psyco.readAnalog(i);
}

// check if reset tunning is requested (10 single clicks on rotarty encoder)
if (buttonState == 1){
  Serial.print(F("- Remaining short-click to reset tunning to default: "));
  Serial.println((nbResetClicks-tunnningResetCounter));
  if (tunnningResetCounter>=nbResetClicks){
    resetTunning(psyco_mode);  
  }
  tunnningResetCounter++;
}

//////////////////////////
////// STATE MACHINE /////
//////////////////////////

if (buttonState == 2){ 
    if (psyco_mode == PLAYING){
      psyco_mode = ALT_TUNING;
    }
    else if (psyco_mode == ALT_TUNING){
      psyco_mode = REF_TUNING;
      saveTuning();
    }
    else if (psyco_mode == REF_TUNING){
      psyco_mode = PLAYING;
      saveTuning();
    }

    // any time a state is changed then: 
    // turn off gate, stop tuning, and change led according to state
    psyco.writeGate(0,LOW);
    tune_off(-1);
    ledScene(psyco_mode);
    printTuningMode();
    tunnningResetCounter = 0;
    
}

/////////////////////////////////
////// STATE Implementation /////
/////////////////////////////////

/////////////////////////////////
////// Alternative tuning ///////

if(psyco_mode == ALT_TUNING){
  
  //detect which note to tune
  for (int i=0; i<NUMBER_OF_NOTES;i++){
    if (input[i] > threshold){
      
      // if same note is hit again, then turn off tuning of this note
      if (tune_on[i] == true){
        psyco.writeGate(0,LOW);
        tune_off(-1);
        tune_on[i] = false;
        setLed(i, alt_color);
      }
      
      // when current note was not being tuned, then activate its tuning now
      else
      {
        psyco.writeGate(0,HIGH);
        tune_off(i);
        tune_on[i] = true;
        setLed(i, note_select_color);
        printNote(i,alt_tuning[i]);
      }
    }
  }
  
  // for the note being tuned, read rotary encoder and change its tuning
  // and keep the frequencies arrays uptodate
  for (int i=0; i<NUMBER_OF_NOTES;i++){
    if (tune_on[i] == true){
      int freq = alt_tuning[i];
      encoderDirection = psyco.readEncoderDirection();
      freq = freq + (encoderDirection * tunerIncrement);
      freq = limitCV(freq, 0, 4095);
      psyco.writeCV(0, freq);
      alt_tuning[i] = freq;
      if (encoderDirection !=0){
        printNote(i,freq);
      }
    }
  }

} // end alt mode

///////////////////////////////
////// Reference tuning ///////

if(psyco_mode == REF_TUNING){
  
  //detect which note to tune
  for (int i=0; i<NUMBER_OF_NOTES;i++){
    if (input[i] > threshold){

      // if same note is hit again, then turn off tuning of this note
      if (tune_on[i] == true){
        psyco.writeGate(0,LOW);
        tune_off(-1);
        tune_on[i] = false;
        setLed(i, ref_color);
      }

      // when current note was not being tuned, then activate its tuning now
      else
      { 
        psyco.writeGate(0,HIGH);
        tune_off(i);
        tune_on[i] = true;
        setLed(i, note_select_color);
        printNote(i,ref_tuning[i]);
      }
      
    }
  }
  
  // for the note being tuned, read rotary encoder and change its tuning
  // and keep the frequencies arrays uptodate
  for (int i=0; i<NUMBER_OF_NOTES;i++){
    if (tune_on[i] == true){
      int freq = ref_tuning[i];
      encoderDirection = psyco.readEncoderDirection();
      freq = freq + (encoderDirection * tunerIncrement);
      freq = limitCV(freq, 0, 4095);
      psyco.writeCV(0, freq);
      ref_tuning[i] = freq;
      if (encoderDirection !=0){
        printNote(i,freq);
      }    
    }
  }

} // end tuning mode

///////////////////////////////
///////// Play mode ///////////

else if (psyco_mode == PLAYING){

  long now = millis();
  
  checkAlternative(ALT_PIN);

  checkScaleMode(SCALE_MODE_PIN);
  
  checkFsr(FSR_INPUT);
  
  checkButton(GATE_HOLD_PIN);
  
  // detect pressed inputs and set cv values
  for (int j=0; j<NUMBER_OF_NOTES;j++){
    if (input[j] > threshold){
      if (alt_on[j] == true){
        psyco.writeCV(0, alt_tuning[j]);
        printNote(j,alt_tuning[j]);
      }
      else {
        psyco.writeCV(0, ref_tuning[j]);
        printNote(j,ref_tuning[j]);
      }
      psyco.writeGate(0,HIGH);
      GATE_ON = true;
      setLed(j, note_select_color);
      
    }
  }
  
  // decide if gate should be closed quickly (i.e when GATE button is not pressed)
  if (GATE_ON && now > lastChangeTime + gateLength && !HOLDING){
  
   GATE_ON = false;
   psyco.writeGate(0,LOW);
   lastChangeTime = now;
   
  }

  // remove green color if note is off
  if (GATE_ON == false && now > lastChangeTime + 120){
    ledScene(psyco_mode); 
  } 
  
} // end play mode


}  /// end of loop

/////////////////////////////////
////// Utitlity functions ///////
/////////////////////////////////

// check state of GATE HOLDING button
void checkButton(int pin){
  unsigned int buttonState = psyco.readAnalog(pin);  
  if (buttonState > 20){ 
      HOLDING = true;
    }
    else {
      HOLDING = false;
    }  
}

// handle master alternative tunning toggle in PLAYING MODE

void checkScaleMode(int pin){
  unsigned int value = psyco.readAnalog(pin);
  if (value>200){
    delay(alt_buttons_debound);
    value = psyco.readAnalog(pin);
    if (value > 200){
      SCALE_MODE = !SCALE_MODE; // toggle mode
      
      if (SCALE_MODE == true){
        for (int i = 0; i < NUMBER_OF_NOTES; i++){
          if (scale_alt_on[i] == true){
            alt_on[i] = true;
            setLed(i,alt_color);
          }
          else {
            alt_on[i] = false;
            setLed(i,play_color);
          }
        }
        Serial.println("- SCALE mode is ON");
      }
      else{
        for (int i = 0; i < NUMBER_OF_NOTES; i++){
          scale_alt_on[i] = alt_on[i];  // save current selection to scale storage
          alt_on[i] = false;
          setLed(i,play_color);
        }
        Serial.println("- SCALE mode is OFF");
      }
    
    }    
  }
  
}




// handle alternative tunning selection in PLAYING mode
void checkAlternative(int pin){
  unsigned int value = psyco.readAnalog(pin);
  
  int index;

  if (value > 0){
    //Serial.print(value);
    if (value >= 75 && value <= 80)
      index = 0;
    else if (value >= 210 && value <= 215)
      index = 1;
    else if (value >= 335 && value <= 340)
      index = 2;
    else if (value >= 465 && value <= 470)
      index = 3;
    else if (value >= 615 && value <= 620)
      index = 4;
    else if (value >= 805 && value <= 810)
      index = 5;
    else if (value >= 1015 && value <= 1020)
      index = 6;
    else if (value >= 145 && value <= 150)
      index = 7;
    else if (value >= 270 && value <= 280)
      index = 8;
    else if (value >= 395 && value <= 405)
      index = 9;
    else if (value >= 535 && value <= 545)
      index = 10;
    else if (value >= 700 && value <= 710)
      index = 11;
    else if (value >= 925 && value <= 935)
      index = 12;
    else
      index = -1;

    if (index != -1) {
      delay(alt_buttons_debound);
      value = psyco.readAnalog(pin);
      if (value>=75){
        alt_on[index] = !alt_on[index];
        if (alt_on[index] == true){
          setLed(index,alt_color);
          SCALE_MODE = true;    // any alt note ON will cause SCALE_MODE to be true (needed for SCALE_MODE master toggle)
        }
        else
          setLed(index,play_color);
      }
    }
  }
}

// set LED display color for each mode 
void ledScene(mode m){

  // set colors in play mode and managing the note's alt (orange) or play (green) colors
  if (m == PLAYING){
    for (int i = 0; i < NUMBER_OF_NOTES; i++){
      if (alt_on[i] == true)
        leds[ledMap[i]] = alt_color;
      else
        leds[ledMap[i]] = play_color;
    }
  }
  
  else if (m == REF_TUNING){
    for (int i = 0; i < NUMBER_OF_NOTES; i++){
        leds[ledMap[i]] = ref_color;
    }
  }
  
  else if (m == ALT_TUNING){
    for (int i = 0; i < NUMBER_OF_NOTES; i++){
        leds[ledMap[i]] = alt_color;
    }
  }

  FastLED.show();
 

}


// set led color provided a note index between 0 and NUMBER_OF_NOTES-1
void setLed(int note, CHSV color){

  // clear all leds while still showing current mode color(s) 
  ledScene(psyco_mode);

  if (note < NUMBER_OF_NOTES)
    leds[ledMap[note]] = color;
  else
    ledScene(psyco_mode);   //if out of range, then reset all leds

  FastLED.show();
}


// blink led using current colors
void blinkLED(int note){
  if (note == -1) {
    FastLED.clear();
    FastLED.show();
    delay(500);
    ledScene(psyco_mode);
    delay(500);
    FastLED.clear();
    FastLED.show();
    delay(500);
    ledScene(psyco_mode);
  }
  else{
    leds[ledMap[note]] = CRGB::Black;
    FastLED.show();
    delay(500);
    ledScene(psyco_mode);
    delay(500);
    leds[ledMap[note]] = CRGB::Black;
    FastLED.show();
    delay(500);
    ledScene(psyco_mode);
  }
}

// turn off tuning flqg for all input EXCEPT for the skipNote
void tune_off(int skipNote){
  // if passed value outside array size (ex. -1), then all array will be set to false (i.e not tuning)
  for (int i=0; i<NUMBER_OF_NOTES; i++){
    if (i != skipNote){
      tune_on[i] = false;
      }
  }

}

// Alter note off flng flqg for all input EXCEPT for the skipNote
void alt_off(int skipNote){
  // if passed value outside array size (ex. -1), then all array will be set to false (i.e not alternative notes)
  for (int i=0; i<NUMBER_OF_NOTES; i++){
    if (i != skipNote){
      alt_on[i] = false;
      }
  }

}

// makes sure a cv note is within mn and mx range (typical range 0-4095)
int limitCV(int note, int mn, int mx){
  if (note > mx)
    note = mx;
  if (note < mn)
    note = mn;
  return note;
  
}

// reset to default tunning provided the current psyco mode
void resetTunning(int mode){
  tunnningResetCounter = 0;
  int tNote = getTunningNote(); // check if a note being tunned (inorder to reset it only instead of all)
  switch (mode) {
  case ALT_TUNING:
    for (int i = 0; i < NUMBER_OF_NOTES; i++)
    {
      if (tNote == -1)
        alt_tuning[i] = default_alt_tuning[i];
      else {
        alt_tuning[tNote] = default_alt_tuning[tNote];
        Serial.println(F("- ALTERNATIVE tunning of one note is reset to default"));
        blinkLED(tNote);
        break;
      }
    }
    if (tNote == -1) {
      Serial.println(F("- All ALTERNATIVE tunning is reset to default"));
      blinkLED(-1);
    }
    break;
  
  case REF_TUNING:
    for (int i = 0; i < NUMBER_OF_NOTES; i++)
    {   
      if (tNote == -1)
        ref_tuning[i] = default_ref_tuning[i];
      else {
        ref_tuning[tNote] = default_ref_tuning[tNote];
        Serial.println(F("- REFERENCE tunning of one note is reset to default"));
        blinkLED(tNote);
        break;
      }
    }
    if (tNote == -1) {
      Serial.println(F("- All REFERENCE tunning is reset to default"));
      blinkLED(-1);
    }
    break;
  
  case PLAYING:
    for (int i = 0; i < NUMBER_OF_NOTES; i++)
    {
      ref_tuning[i] = default_ref_tuning[i];
      alt_tuning[i] = default_alt_tuning[i];
    }
    Serial.println(F("- ALTERNATIVE & REFERENCE tunning is reset to default"));
    blinkLED(-1);
    break;
  }
   
}


// check FSR input and control CV 1 accordingly
void checkFsr(int pin){
  int fsr = psyco.readAnalog(pin);
  int val = 0;
  
  val = map(fsr,0,1023,0,1000);
  
  psyco.writeCV(1, val);
}
// get note being tunned, if none, it returns -1
int getTunningNote(){
  int index =  -1;
  for(int i = 0; i < NUMBER_OF_NOTES;i++){
    if (tune_on[i] == true){
      index = i;
      break;
    }
  }
  return index;
}

// print tunning mode to serial
void printTuningMode(){
  switch (psyco_mode) {
  case 0:
    Serial.println(F("==== Mode: ALTERNATIVE tunning ===="));
    break;
  case 1:
    Serial.println(F("==== Mode: REFERENCE tunning ===="));
    break;
  case 2:
    Serial.println(F("==== Mode: PLAY ===="));
    break;
  }
}

// print note index and cv to serial
void printNote(int i,int freq){
  Serial.print(F("- Note:"));
  Serial.print(i);
  Serial.print(F(", CV:"));
  Serial.println(freq);
}

// write an array of integers (i.e 2 bytes each read) to EEPROM starting at the provided 'address'
void writeIntArrayIntoEEPROM(int address, int numbers[], int arraySize)
{
  int addressIndex = address;
  for (int i = 0; i < arraySize; i++) 
  {
    //note by using update() instead of write() then memory is updated only if incoming data doesn't equal stored data (i.e longer EEPROM life)
    EEPROM.update(addressIndex, numbers[i] >> 8);
    EEPROM.update(addressIndex + 1, numbers[i] & 0xFF);
    addressIndex += 2;
  }
  EEPROM.update(ADDR_EEPROM_MARKER, MARKER_EEPROM);
}

// read an array of integers (i.e 2 bytes each read) from EEPROM starting at the provided 'address'
void readIntArrayFromEEPROM(int address, int numbers[], int arraySize)
{
  int addressIndex = address;
  for (int i = 0; i < arraySize; i++)
  {
    numbers[i] = (EEPROM.read(addressIndex) << 8) + EEPROM.read(addressIndex + 1);
    addressIndex += 2;
  }
}


// save tunning to long term memory
void saveTuning(){
  writeIntArrayIntoEEPROM(ADDR_REF_TUNNING, ref_tuning, NUMBER_OF_NOTES);
  writeIntArrayIntoEEPROM(ADDR_ALT_TUNNING, alt_tuning, NUMBER_OF_NOTES);
  Serial.println(F("- Tunning is saved to memory"));
}

// save tunning to long term memory
void loadTuning(){
  readIntArrayFromEEPROM(ADDR_REF_TUNNING, ref_tuning, NUMBER_OF_NOTES);
  readIntArrayFromEEPROM(ADDR_ALT_TUNNING, alt_tuning, NUMBER_OF_NOTES);
  Serial.println(F("- Tunning is loaded from memory"));
}

void initializeMemory()
{
  //copy tunning defined in default tunning arrays to live tuning arrays
  for (int i = 0; i < NUMBER_OF_NOTES; i++)
  {
    ref_tuning[i] = default_ref_tuning[i];
    alt_tuning[i] = default_alt_tuning[i];
  }
  
  byte marker = EEPROM.read(ADDR_EEPROM_MARKER);
  if (marker == MARKER_EEPROM)
  {
    loadTuning();
  }
  else 
    Serial.println(F("- No tunning is stored yet. Default tunning will be used"));
}
