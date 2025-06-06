#include <PWMServo.h>
#include <Arduino.h>

#define SERVO_PIN_A 9
#define SERVO_PIN_B 10

PWMServo servoA;
PWMServo servoB;

void setup() {
  Serial.begin(9600);
  servoA.attach(SERVO_PIN_A);
  servoB.attach(SERVO_PIN_B);
}

void processCommand(const String& cmd) {
  // Expecting format: S1P70 or S2P180
  if (cmd.length() < 4) return;

  int servoNum = cmd.substring(1, 2).toInt();
  int pIndex = cmd.indexOf('P');
  if (pIndex == -1) return;
  int pos = cmd.substring(pIndex + 1).toInt();
  if (pos < 0 || pos > 180) return;

  if (servoNum == 1) {
    servoA.write(pos);
  } else if (servoNum == 2) {
    servoB.write(pos);
  }
}

void loop() {
  static String input = "";
  while (Serial.available() > 6) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      input = ""; // Reset on newline, ignore incomplete commands
    } else {
      input += c;
      if (input.length() == 6) { // Only process when exactly 6 chars received
        processCommand(input);
        input = "";
      } else if (input.length() > 6) {
        input = ""; // Reset if input is too long (invalid command)
      }
    }
  }
}


