#include <SoftwareSerial.h>

// Formatas -> /string
// Pvz. -> /P200 -> nustato P reiksme i 200

//SoftwareSerial SerialBT(5, 4);
SoftwareSerial SerialBT(0, 1); // nano

char bt_byte;
String bt_text;
int symCounter;

String GetString(){
  while(SerialBT.available()){
    char bt_byte = SerialBT.read();
    // SerialBT.print(bt_byte);
    // SerialBT.print("|");
    // SerialBT.println((int)bt_byte);
    symCounter++;

    if(bt_byte=='/'){
      bt_text="";
      symCounter=0;
      } 
    // else if(bt_byte==13) return bt_text;
    else if(bt_byte==10){
      return symCounter+bt_text;
    } 
    else bt_text+=(char)bt_byte;
  }
  return "";
}

