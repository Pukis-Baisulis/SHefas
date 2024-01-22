#include <SoftwareSerial.h>
#include "bt.h"

// SerialBT.setRX()
// SerialBT.setTX()
//SoftwareSerial SerialBT(5, 4);

#define APPVALUES_COUNT 3

long int AppValues[APPVALUES_COUNT] = {0, 0, 0};
char AppIndexes[APPVALUES_COUNT] = {'P', 'I', 'D'};


void setup() {
  SerialBT.begin(115200);
  // SerialBT.begin(9600); // nano
  SerialBT.println("");
  SerialBT.println("Ir as esu bobot BeepBoop BeepBoop ");
  SerialBT.println("ir prarysiu tave BoopBeep BoopBeep");
  SerialBT.println("");
}


void loop(){
  String result=GetString();
  String value="";
  if(result!="") SerialBT.println(result);

  for(int i=2; i<result[0]; i++){
    value+=result[i];
  }

  for(int i=0; i<APPVALUES_COUNT; i++){
    if(result[1]==AppIndexes[i]){
      AppValues[i]=value.toInt();
      SerialBT.print("*");
    }
    if(result!=""){
      SerialBT.print(AppIndexes[i]); SerialBT.print(" "); SerialBT.println(AppValues[i]);
    }
  }
}
