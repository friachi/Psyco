#include <Psyco.h>

// psyco gpio INPUT3 (0-2): 0-2
// can be set to either INPUT, INPUT_PULLUP or OUTPUT


Psyco psyco;

int digital_value;
int analog_value;

void setup() {

  Serial.begin(9600);
  psyco.begin();

  psyco.pinModeGPIO(0,INPUT);
  psyco.pinModeGPIO(1,INPUT_PULLUP);
  psyco.pinModeGPIO(2,OUTPUT);

}

void loop(){

  digital_value = psyco.digitalReadGPIO(0);
  Serial.print(digital_value);

  analog_value = psyco.analogReadGPIO(1);
  Serial.println(analog_value);

  psyco.digitalWriteGPIO(2, HIGH);
  delay(20);

  psyco.digitalWriteGPIO(2, LOW);
  delay(20);

}