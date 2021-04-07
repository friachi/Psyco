#include <Psyco.h>

// psyco rotary encoder (A0 & A1), readEncoderDirection() can return:
//  0: no rotation
//  1: clockwise
// -1: anti-cloclwise
// returned data can be accumulated to created a counter/position reading

Psyco psyco;

int encoderDirection;

void setup() {
  Serial.begin(9600);
  psyco.begin();
}

void loop(){

    encoderDirection = psyco.readEncoderDirection();

    switch (encoderDirection) {
      case 1:
        Serial.println("encoder: clockwise");
        break;

      case -1:
        Serial.println("encoder: anti-clockwise");
        break;
    }

}