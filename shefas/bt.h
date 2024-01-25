#include <SoftwareSerial.h>

// Formatas -> /string
// Pvz. -> /P200 -> nustato P reiksme i 200

SoftwareSerial SerialBT(4, 5);
//SoftwareSerial SerialBT(0, 1); // nano

char bt_byte;
String bt_text;
int symCounter;

String GetString(){
  while(SerialBT.available()){
    char bt_byte = (char)SerialBT.read();
    // Serial.print((char)bt_byte);
    // Serial.print("|");
    // Serial.println((int)bt_byte);
    // Serial.println(bt_text);

    //if(bt_byte!=0){
      if(bt_byte=='/'){
        bt_text="";
        symCounter=0;
      }
      else if(bt_byte==13){
        return symCounter+bt_text;
      }
      else bt_text+=(char)bt_byte;
      symCounter++;
    //}
  }
  return "";
}

