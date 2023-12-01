/*   
 *   Basic example code for controlling a stepper with the AccelStepper library
 *      
 *   by Dejan, https://howtomechatronics.com
 */

#include <AccelStepper.h>

const int DIR = 12;
const int STEP = 14;

#define motorInterfaceType 1
// Define the stepper motor and the pins that is connected to0
AccelStepper motor(motorInterfaceType, STEP, DIR); // (Type of driver: with 2 pins, STEP, DIR)

void setup() {
  // Set maximum speed value for the stepper
  motor.setMaxSpeed(1000); // Set maximum speed value for the stepper
  motor.setAcceleration(500); // Set acceleration value for the stepper
  motor.setCurrentPosition(0); // Set the current position to 0 steps
  Serial.begin(115200);
  //pinMode(32,OUTPUT);
}

void loop() {
  Serial.println("Moving to 800");
  motor.moveTo(800); // Set desired move: 800 steps (in quater-step resolution that's one rotation)
  motor.runToPosition(); // Moves the motor to target position w/ acceleration/ deceleration and it blocks until is in position
     // Move back to position 0, using run() which is non-blocking - both motors will move at the same time
  motor.moveTo(0);
  Serial.println("Moving to 0");
  while (motor.currentPosition() != 0) {
    motor.run();  // Move or step the motor implementing accelerations and decelerations to achieve the target position. Non-blocking function
    //
    //
  }
}