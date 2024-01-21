#define SRX_PIN 12 // SD(spi)
#define STX_PIN 15
#define CS_PIN 13
#define CLK_PIN 14

#define LED_PIN 25

#include <SPI.h>
#include <SD.h>

File logFile;

void setup() {
  
    SPI1.setRX(SRX_PIN);
    SPI1.setCS(CS_PIN);
    SPI1.setSCK(CLK_PIN);
    SPI1.setTX(STX_PIN);
    SPI1.begin();
  // debug and datalog to file with next number
    pinMode(LED_PIN,OUTPUT);
    String data  = "A 45875847458757 x890 y879 7987 2345 6534 5678 4839 3487 9203 3489";
    SD.begin(CS_PIN, SPI1);
    logFile = SD.open("logs.txt", FILE_WRITE);
    long timeStart = micros();
    logFile.println(data);
    logFile.flush();
    logFile.println(micros()-timeStart);
    logFile.close();
    

    digitalWrite(LED_PIN, HIGH);
    

}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}
