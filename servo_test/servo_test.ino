
  #define SERVO_MIN 0// to be determined expermentaly !!!
  #define SERVO_MAX 180
  #define SERVO_MIDPOINT 180
  #define SERVO_PIN 6

  #include <Servo.h>

  Servo myservo;

  int pos = 0;
void setup() {
  myservo.attach(SERVO_PIN);

}

void loop() {
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15 ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15 ms for the servo to reach the position
  }
}
