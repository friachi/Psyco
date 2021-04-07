#include <Psyco.h>

// psyco analog write: buffered CV
// CV (1-4): 0 - 3
// values can be set between 0 & 4095 (i.e O to 5v)

Psyco psyco;

void setup() {
  psyco.begin();
}

void loop(){

  // CV 0, 1, 2, 3
  psyco.writeCV(0, 0);
  psyco.writeCV(1, 1000);
  psyco.writeCV(2, 2500);
  psyco.writeCV(3, 4095);
  
}
