// pinout 
  #define RX_PIN 5// serialBluetooth
  #define TX_PIN 4//115200 BaudRate 

  #define SCL0_PIN 21 // for vl53l5cx
  #define SDA0_PIN 20
  #define I2C_0_CLOCK 1000000 //1Mhz

  #define SCL1_PIN 11 // for mpu-6050
  #define SDA1_PIN 10
  #define I2C_1_CLOCK 400000 //400khz

  #define SRX_PIN 12 // SD(spi)
  #define STX_PIN 15
  #define CS_PIN 13
  #define CLK_PIN 14

  #define IR_PIN 1

  #define SERVO_PIN 6
  #define IN_A_PIN 8
  #define IN_B_PIN 9
  #define PWM_PIN 7
  #define PWM_RES 10 //0 -> 2^PWM_RES
  #define PWM_FREQ 1000 // hz
  #define XSHUT_A_PIN 17
  #define XSHUT_B_PIN 18 
  #define XSHUT_C_PIN 19

  #define LED_PIN 25


// libraries 
  //plz just copy the libraries from the project folder and not use downloaded ones(exeptions: Wire.h, Servo.h)!
  //may include modified versions of the libraries, copy both from project folder to the libraries folder 
  #include <Servo.h>
  #include <IRremote.hpp>
  #include <Wire.h> 
  #include <SPI.h>
  #include <SD.h>
  #include <SoftwareSerial.h>
  #include "bt.h"
  #include "SparkFun_VL53L5CX_Library.h"
  #include "MPU6050_light.h"
  #include "MPU6050A.h"
// objects and configs
  File logFile;

  SparkFun_VL53L5CX vlx[3];
  VL53L5CX_ResultsData vlxData[3];
  const int xShut[3] = {XSHUT_A_PIN, XSHUT_B_PIN, XSHUT_C_PIN};
  int sensorAddress = 0x46;

  MPU6050 mpuB(Wire1);
  MPU6050A mpuA(Wire1); 

  Servo servo;

  #define SERVO_MIN 0// to be determined expermentaly !!!
  #define SERVO_MAX 160
  #define SERVO_MIDPOINT 80

  #define MAX_VLX_DIST_SIDES 500
  #define MAX_VLX_DIST_FRONT 2000 // front sensor distance limit, impacts speed and acceleration 
  #define MIN_FRONT_DIST 500 // starts ignoring obstacles beyond this distance
  #define USE_SPADS_FRONT 48 //disregard some of the top distances (2 layers)

  #define ALLOWED_DELTA_SIDES 100


  #define MIN_SPEED_DISTANCE 300
  #define MIN_NORMAL_SPEED 180 // to be determined expermentaly !!! 
  #define MAX_NORMAL_SPEED 200
  #define MIN_START_SPEED 90
  #define MIN_SPEED 80

// variables
  bool setupDone = false;
  bool go=false;
  int targetPos = 0; 
  int targetDist = 0; 
  int avgDistFront = 0;
  bool targetIgnore = false;
  int distA[4]={0,0,0,0};
  bool maxTagA[4]={false,false,false,false};
  int tagsA = 0;
  int distB[4]={0,0,0,0};
  bool maxTagB[4]={false,false,false,false};
  int tagsB = 0;
  bool goStraight = false;
  int sideMod[4]={35,40,25,0};//if everything works correctly should add up to 100
  int distModA[4]={0,0,0,0};
  int distModB[4]={0,0,0,0};

  
  int target = 0;
  int lastErr = 0;
  long long I = 0;
  long long lastT=0;


  bool drvEn = true;
  int speed = 0;
  double kP = 0.3;
  double kI = 0;
  double kD = 2; // later divided by 1000

#define APPVALUES_COUNT 3
double AppValues[APPVALUES_COUNT] = {kP, kI, kD};
char AppIndexes[APPVALUES_COUNT] = {'P', 'I', 'D'};

// prototypes
  void drive(int spd, int dir);
  void sensors();
  int  PID();
// main functions 
  void setup() {
    // SPI config
      SPI1.setRX(SRX_PIN);
      SPI1.setCS(CS_PIN);
      SPI1.setSCK(CLK_PIN);
      SPI1.setTX(STX_PIN);
    // debug and datalog to file with next number
      Serial.begin(115200);
      SerialBT.begin(115200);
      SD.begin(CS_PIN, SPI1);
      logFile = SD.open("logs.txt", FILE_WRITE);
      logFile.println("N New log");
    //pinModes
      servo.attach(SERVO_PIN, 900, 2300);// servo attachment 
      pinMode(PWM_PIN,     OUTPUT); // pinModes
      pinMode(IN_A_PIN,    OUTPUT);
      pinMode(IN_B_PIN,    OUTPUT);
      pinMode(XSHUT_A_PIN, OUTPUT);
      pinMode(XSHUT_B_PIN, OUTPUT);
      pinMode(XSHUT_C_PIN, OUTPUT);
      pinMode(LED_PIN,     OUTPUT);

      servo.write(SERVO_MIDPOINT); // set all output pins LOW 
      digitalWrite(PWM_PIN,     LOW);
      digitalWrite(IN_A_PIN,    LOW);
      digitalWrite(IN_B_PIN,    LOW);
      digitalWrite(XSHUT_A_PIN, HIGH);
      digitalWrite(XSHUT_B_PIN, HIGH);
      digitalWrite(XSHUT_C_PIN, HIGH);
      digitalWrite(LED_PIN,     LOW);

    //pwm config
      analogWriteResolution(PWM_RES);//pwm config
      analogWriteFreq(PWM_FREQ);

    //vlx bootup
      Wire.setSDA(SDA0_PIN);//setup i2c 0 interface
      Wire.setSCL(SCL0_PIN);//used for vl53l5cx
      Wire.setClock(I2C_0_CLOCK);
      Wire.begin();

      digitalWrite(XSHUT_A_PIN, LOW);//for some fucking bloody reason this is the oposite from the library
      digitalWrite(XSHUT_B_PIN, LOW);//HIGH enables the sensor, LOW disables 
      digitalWrite(XSHUT_C_PIN, LOW);
      delay(100);
      for(int i =0; i < 3; i++){
        digitalWrite(xShut[i], HIGH);//turn on next sensor
        delay(100);//wait for boot 
        //vlx[i].setWireMaxPacketSize(128);//not tested yet, may increase speed
        vlx[i].begin();//init 
        vlx[i].setAddress(sensorAddress+i);//change default i2c address
      }
      vlx[0].setResolution(4 * 4);//set resolution 
      vlx[0].setRangingFrequency(60); //set refresh rate
      vlx[0].setRangingMode(SF_VL53L5CX_RANGING_MODE::CONTINUOUS);
      vlx[0].startRanging();//start ranging
      

      vlx[1].setResolution(4 * 4);//set resolution 
      vlx[1].setRangingFrequency(60); //set refresh rate
      vlx[1].setRangingMode(SF_VL53L5CX_RANGING_MODE::CONTINUOUS);
      vlx[1].startRanging();//start ranging
      
      vlx[2].setResolution(4 * 4);//set resolution 
      vlx[2].setRangingFrequency(60); //set refresh rate
      vlx[2].setRangingMode(SF_VL53L5CX_RANGING_MODE::CONTINUOUS);
      vlx[2].startRanging();//start ranging
      
    // mpus bootup
      Wire1.setSDA(SDA1_PIN);//setup i2c 1 interface
      Wire1.setSCL(SCL1_PIN);
      Wire1.setClock(I2C_1_CLOCK);
      Wire1.begin();
      mpuA.begin();
      mpuB.begin(); 


    //setup end credits
      digitalWrite(LED_PIN, HIGH);
      servo.write(SERVO_MIDPOINT);
      setupDone=true;
      while(!motEn && !go){}
  }

  void setup1(){
    IrReceiver.begin(1, ENABLE_LED_FEEDBACK);


    while(!setupDone){}
    go=true;
  }

  void loop() {
    sensors();
    int steer = PID();
    drive(speed, steer);
  }

  void loop1(){
    // bluetooth
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
    //IR
      // if (IrReceiver.decode()) {
      //   IrReceiver.printIRResultShort(&Serial);
      //   IrReceiver.printIRSendUsage(&Serial);
      //   Serial.println();

      //   IrReceiver.resume();

      //   switch(IrReceiver.decodedIRData.command == 0x10{

      //   }
      // }
  }

//addon functions

int PID(){
  long dA = ((distA[0]*sideMod[0])+(distA[1]*sideMod[1])+(distA[2]*sideMod[2])+(distA[3]*sideMod[3]))/100;
  long dB = ((distB[0]*sideMod[3])+(distB[1]*sideMod[2])+(distB[2]*sideMod[1])+(distB[3]*sideMod[0]))/100;
  long pos = dB - dA;
  long err = target - pos;
  I += err;
  double pid = constrain(((err*kP) + (I*kI) + (((err-lastErr)/(micros()-lastT))*kD)),-45,45);
  lastT = micros();
  lastErr = err;
  if(goStraight){
    I=0;
    return 0;
  }
  else 
    return pid;
}

void sensors(){

  //Left VLX
    if (vlx[0].isDataReady() == true) 
    {
      tagsA=0;
      vlx[0].getRangingData(&vlxData[0]);
      //vlxData[1].
      for(int i = 0; i < 4; i++){//use 4 vertical arrays  ||||
        int sum = vlxData[0].distance_mm[i+8] + vlxData[0].distance_mm[i+4];
        if(abs((sum/2)-vlxData[0].distance_mm[i])<ALLOWED_DELTA_SIDES){
          sum += vlxData[0].distance_mm[i];
          distA[i] = sum/3;
        }
        else{
          distA[i] = sum/2;
        }
        maxTagA[i]=false;
        if(distA[i] >= MAX_VLX_DIST_SIDES){
          maxTagA[i] = true;
          tagsA++;
        }
      }
    }
  /*/Front VLX
    if (vlx[1].isDataReady() == true) 
    {
      vlx[1].getRangingData(&vlxData[1]);
      int minVal    = 9969;
      int minValPos = -1;
      int maxVal    = -1;
      long avg = 0;
      for(int i = 0; i < USE_SPADS_FRONT; i++){
        int tmp = vlxData[1].distance_mm[i];
        if(tmp<MAX_VLX_DIST_FRONT)
          tmp = vlxData[1].distance_mm[i];

        if(tmp > maxVal){
          maxVal = tmp;
        }
        if(tmp < minVal){
          minValPos=i;
          minVal = tmp;
        }
        avg+=tmp;
      }
      avgDistFront = avg/USE_SPADS_FRONT;
      targetDist = minVal;
      targetPos = (minValPos%8)-4;
      if(targetPos>=0)
        targetPos++;

      targetIgnore = false;
      if(minValPos <= 7 || minVal >=MIN_FRONT_DIST){
        targetPos = 0;
        targetIgnore = true;
      }
    }//*/
  //Right VLX
    if (vlx[2].isDataReady() == true)
    {
      tagsB = 0;
      vlx[2].getRangingData(&vlxData[2]);
      //vlx[2].
      for(int i = 0; i < 4; i++){//use 4 vertical arrays  ||||
        int sum = vlxData[2].distance_mm[i+8] + vlxData[2].distance_mm[i+4];
        if(abs((sum/2)-vlxData[2].distance_mm[i])<ALLOWED_DELTA_SIDES){
          sum += vlxData[2].distance_mm[i];
          distB[i] = sum/3;
        }
        else{
          distB[i] = sum/2;
        }
        maxTagB[i]=false;
        if(distB[i] >= MAX_VLX_DIST_SIDES){
          maxTagB[i] = true;
          tagsB++;
        }
      }
    }
  //calculations
    //mode
      goStraight = false;
      if(tagsA >=3 && tagsB >=3){
        goStraight = true;
      }


}

void drive(int spd, int dir){  
  int dirN = map(dir, -45, 45, SERVO_MIN, SERVO_MAX);
  dirN = constrain(dirN, SERVO_MIN, SERVO_MAX);
  servo.write(dirN);
  if(drvEn){
    analogWrite(PWM_PIN, abs(spd));
    if(spd > 0){
      digitalWrite(IN_A_PIN, HIGH);
      digitalWrite(IN_B_PIN,  LOW);
    }
    else{
      digitalWrite(IN_A_PIN,  LOW);
      digitalWrite(IN_B_PIN, HIGH);
    }
  }
  else{
    digitalWrite(IN_A_PIN, HIGH);
    digitalWrite(IN_B_PIN, HIGH);
  }
}
