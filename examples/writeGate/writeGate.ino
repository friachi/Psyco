#include <Psyco.h>

// psyco digital write: buffered GATE
// GATE (1-4): 0 - 3
// values can be LOW or HIGH (i.e O to 5v)

Psyco psyco;

void setup() {
  psyco.begin();
}

void loop(){

  // GATE 0, 1, 2, 3
  psyco.writeGate(0,HIGH);
  psyco.writeGate(1,LOW);
  psyco.writeGate(2,HIGH);
  psyco.writeGate(3,LOW);
  
}
