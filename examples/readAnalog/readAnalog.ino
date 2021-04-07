#include <Psyco.h>

// psyco analog inputs 0 - 17 (10bits resolution), such that
// INPUT1 (0-7): 0 - 7
// INPUT2 (0-7): 8 - 15
// INPUT3 (3-4): 16 - 17

Psyco psyco;
unsigned int var;

void setup() {
  
  Serial.begin(9600);
  psyco.begin();
  
}

void loop(){
  // inputs: 0 to 17
  for (int chan=0; chan<=17; chan++) {
     
     var = psyco.readAnalog(chan);
     
     Serial.print(var); 
     Serial.print("\t");
  }

  Serial.println();
  
}
