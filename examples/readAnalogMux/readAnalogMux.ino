#include <Psyco.h>

Psyco psyco;

int nbOfMuxedInputs;

void setup() {
  
  Serial.begin(115200); // high serial baud rate to allow for printing values of the all muxed inputs
  
  psyco.begin();

  // this will expand the first 6 analog inputs, namely: 0, 1, 2, 3, 4, 5
  // such that each input becomes 8 inputs, thus resulting in 6*8 number of usable muxed inputs
  // it is possible to expand all psyco inputs, that is up to 18 (thus resulting in 18*8 number of usable muxed inputs)
  // tip: Psyco Exntension board has 6 max, with 8 inputs each, i.e total of 48 inputs
  
  nbOfMuxedInputs = psyco.expandInputs(6);

}

void loop(){

  for (byte i = 0; i < nbOfMuxedInputs; i++){

    // read muxed inputs
    int value = psyco.readAnalogMux(i);

    // print values (use Serial plotter for better readability)
    if (i == nbOfMuxedInputs - 1 ){
      
     Serial.print("\t");
     Serial.println(value);
    }
    else{
     Serial.print("\t");
     Serial.print(value);
    }

  }
  
}