#include "IRremote.h"

#define IR_RECEIVE_PIN 1

void setup() {
  Serial.begin(9600);
  //IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);  
}

void loop() {
  int code = 0;
  while(code==0){
    IrReceiver.resume();
    delay(10);
    if(IrReceiver.decode()){
      code = (IrReceiver.decodedIRData.command == NEC);
      break;
    }
  } 
}
