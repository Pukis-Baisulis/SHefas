#include <SoftwareSerial.h>

// Formatas -> /string
// Pvz. -> /P200 -> nustato P reiksme i 200

SoftwareSerial SerialBT(5, 4);
char bt_byte;
String bt_text;
int breakCounter[2]={0, 0};

String GetString(){
  while(SerialBT.available()){
    char bt_byte = SerialBT.read();
    // SerialBT.print(bt_byte);
    // SerialBT.print("|");
    // SerialBT.println((int)bt_byte);

    if(bt_byte=='/') bt_text="";
    else if(bt_byte==13) return bt_text;
    else bt_text+=bt_byte;
  }
  return "";
}

