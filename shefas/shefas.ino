#define RX_PIN 5// serialBluetooth
#define TX_PIN 4//115200 BaudRate 
//hello
#define SCL0_PIN 21 // for vl53l5cx
#define SDA0_PIN 20
#define I2C_0_CLOCK 1000000 //1Mhz

#define SCL1_PIN 15 // for mpu-6050
#define SDA1_PIN 14
#define I2C_1_CLOCK 400000 //400khz

#define MOSI_PIN // SD(spi)
#define MISO_PIN
#define CS_PIN 13
#define CLK_PIN 10

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

//plz just copy the libraries from the project folder and not use downloaded ones(exeptions: Wire.h, Servo.h)!
//may include modified versions of the libraries, copy both from project folder to the libraries folder 
#include <Servo.h>
#include <Wire.h> 
#include "SparkFun_VL53L5CX_Library.h"
#include "MPU6050_light.h"
#include "MPU6050A.h"

SparkFun_VL53L5CX vlx[3];
VL53L5CX_ResultsData vlxData[3];
const int xShut[3] = {XSHUT_A_PIN, XSHUT_B_PIN, XSHUT_C_PIN};
int sensorAddress = 0x46;

MPU6050 mpuB(Wire1);
MPU6050A mpuA(Wire1); 

Servo servo;
#define SERVO_MIN 0// to be determined expermentaly !!!
#define SERVO_MAX 180
#define SERVO_MIDPOINT 93

#define MAX_VLX_DIST_FRONT 2000 // front sensor distance limit, impacts speed and acceleration 
#define MIN_FRONT_DIST 500 // starts ignoring obstacles beyond this distance
#define USE_SPADS_FRONT 48 //disregard some of the top distances (2 layers)

#define ALLOWED_DELTA_SIDES 100


#define MIN_SPEED_DISTANCE 300
#define MIN_NORMAL_SPEED 100 // to be determined expermentaly !!! 
#define MAX_NORMAL_SPEED 255
#define MIN_START_SPEED 90
#define MIN_SPEED 80


int targetPos = 0; 
int targetDist = 0; 
int avgDistFront = 0;
bool targetIgnore = false;
int distA[4]={0,0,0,0};
int distB[4]={0,0,0,0};
long sideMod[4]={1,0,0,0};//if everything works correctly should add up to 1
int distModA[4]={0,0,0,0};
int distModB[4]={0,0,0,0};
int speed = 0;

void drive(int spd, int dir);
void sensors();
void PID();

void setup() {
  Serial.begin(115200);
  //pinModes
    servo.attach(SERVO_PIN);// servo attachment 
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
    

    vlx[1].setResolution(8 * 8);//set resolution 
    vlx[1].setRangingFrequency(15); //set refresh rate
    vlx[1].setSharpenerPercent(20);
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

}

void loop() {
  sensors();
}

void PID(){
  
}

void sensors(){
  //Left VLX
    if (vlx[0].isDataReady() == true) 
    {
      vlx[0].getRangingData(&vlxData[0]);vlxData[1].
      
      for(int i = 0; i < 4; i++){//use 4 vertical arrays  ||||
        int sum = vlxData[0].distance_mm[i*4+1] + vlxData[0].distance_mm[i*4+2];
        if(abs((sum/2)-vlxData[0].distance_mm[i*4])<ALLOWED_DELTA_SIDES){
          sum += vlxData[0].distance_mm[i*4];
          distA[i] = sum/3;
        }
        else{
          distA[i] = sum/2;
        }
      }
    }
  //Front VLX
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
    }
  //Right VLX
    if (vlx[2].isDataReady() == true)
    {
      vlx[2].getRangingData(&vlxData[2]);
      //vlx[2].
      
      for(int i = 0; i < 4; i++){//use 4 vertical arrays  ||||
        int sum = vlxData[2].distance_mm[i*4+1] + vlxData[2].distance_mm[i*4+2];
        if(abs((sum/2)-vlxData[2].distance_mm[i*4])<ALLOWED_DELTA_SIDES){
          sum += vlxData[2].distance_mm[i*4];
          distB[i] = sum/3;
        }
        else{
          distB[i] = sum/2;
        }
      }
    }
  //calculations
    //speed
      speed = map((avgDistFront+targetDist)/2, MIN_SPEED_DISTANCE, MAX_VLX_DIST_FRONT, MIN_NORMAL_SPEED, MAX_NORMAL_SPEED);
      speed = constrain(speed, MIN_NORMAL_SPEED, MAX_NORMAL_SPEED);
    //side sensor modifiers and create modified side sensor distances
      int alfa = map(speed, MIN_NORMAL_SPEED, MAX_NORMAL_SPEED, -57, 57);
  //*print data
    for(int i = 0; i < 4; i++){
      Serial.print(distA[i]);
      Serial.print(" ");
    }
    for(int i = 0; i < 4; i++){
      Serial.print(distB[i]);
      Serial.print(" ");
    }
    for(int i = 0; i < 4; i++){
      Serial.print(sideMod[i]);
      Serial.print(" ");
    }
    Serial.print(" ");
    Serial.print(targetPos);
    Serial.print(" (");
    Serial.print(targetDist);
    Serial.print(") ");
    Serial.println();//*/
  
}

void drive(int spd, int dir){  
  int dirN = SERVO_MIDPOINT+dir;
  dirN = constrain(dirN, SERVO_MIN, SERVO_MAX);
  servo.write(dirN);

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
