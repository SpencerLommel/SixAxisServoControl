#include <PWMServo.h>
#include <Arduino.h>
 
PWMServo myservo;  // create servo object to control a servo
 
int pos = 0;  // variable to store the servo position
 
void setup() {
  myservo.attach(
      SERVO_PIN_A);  // attaches the servo on pin 9 to the servo object
  // myservo.attach(SERVO_PIN_A, 1000, 2000); // some motors need min/max
  // setting
}
 
bool onState = false;
 
void loop() {
  if (Serial.available() > 0) {
    // Read and discard all incoming data to clear the buffer
    while (Serial.available() > 0) {
      Serial.read();
      //Serial.print(onState);
    }
 
    onState = !onState;
 
    if (onState) {
      for (pos = 0; pos < 180;
           pos += 1) {  // goes from 0 degrees to 180 degrees, 1 degree steps
        myservo.write(pos);  // tell servo to go to position in variable 'pos'
        delay(15);           // waits 15ms for the servo to reach the position
      }
      for (pos = 180; pos >= 1;
           pos -= 1) {       // goes from 180 degrees to 0 degrees
        myservo.write(pos);  // tell servo to go to position in variable 'pos'
        delay(15);           // waits 15ms for the servo to reach the position
      }
    }
  }
}