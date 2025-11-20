#include <Stepper.h>

const int stepsPerRevolution = 200;

// Motor 1 en pines STEP=3, DIR=2
Stepper stepper1(stepsPerRevolution, 3, 2);

// Motor 2 en pines STEP=5, DIR=4
Stepper stepper2(stepsPerRevolution, 5, 4);

void setup() {
  stepper1.setSpeed(60); // en RPM
  stepper2.setSpeed(60);
}

void loop() {
  stepper1.step(200); // 1 vuelta
  stepper2.step(-200);
  delay(1000);

  stepper1.step(-200);
  stepper2.step(200);
  delay(1000);
}