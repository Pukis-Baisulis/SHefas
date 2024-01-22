#include <SoftwareSerial.h>
#include "bt.h"

// SerialBT.setRX()
// SerialBT.setTX()
//SoftwareSerial SerialBT(5, 4);

void setup() {
  SerialBT.begin(115200);
}


void loop(){
  String test=GetString();
  if(test!="") SerialBT.println(test);
}
