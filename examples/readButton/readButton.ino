#include <Psyco.h>

// psyco digital switch (A2), readButtonState() can return:
// 0: open
// 1: short press
// 2: long press

Psyco psyco;

byte buttonState;

void setup() {
  Serial.begin(9600);
  psyco.begin();
}

void loop(){

    buttonState = psyco.readButtonState();

    if (buttonState != 0){
        switch (buttonState) {
          case 1:
            Serial.println("button: Short press");
            break;

          case 2:
            Serial.println("button: Long press");
            break;
        }
    }

}