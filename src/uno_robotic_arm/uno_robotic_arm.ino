// =========================================================================
// Automated Robotic Arm & Gripper Kinematics Controller
// Target Board: Arduino Uno
// University of Moratuwa - Level 1 Hardware Project
// =========================================================================

// Pin definitions
#define IN1 A3

#define IN2 A0

#define IN3 A1

#define IN4 A2

#define Switch A4

const int stepPinSholder = 2;
const int dirPinSholder = 5;
const int stepPinElbow = 3;
const int dirPinElbow  = 6;
const int stepPinBase = 4;
const int dirPinBase = 7;
const int enablePin = 8;
const int limitSholder = 10;
const int limitBase = 11;
const int limitElbow = 9;
const int Gripper_Limit = 12;

const int stepsPer45Deg = 1500;
const int stepSequence[8][4] = {

  {
1, 0, 0, 0}
,
  {
1, 1, 0, 0}
,
  {
0, 1, 0, 0}
,
  {
0, 1, 1, 0}
,
  {
0, 0, 1, 0}
,
  {
0, 0, 1, 1}
,
  {
0, 0, 0, 1}
,
  {
1, 0, 0, 1}
}
;

void setup() {

  
//  Stepper motor pins
  pinMode(stepPinSholder, OUTPUT);
  pinMode(dirPinSholder, OUTPUT);
  pinMode(stepPinElbow, OUTPUT);
  pinMode(stepPinBase, OUTPUT);
  pinMode(dirPinBase, OUTPUT);
  pinMode(dirPinElbow, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(enablePin, OUTPUT);

  
//  Limit switch pin
  pinMode(limitSholder, INPUT_PULLUP);
 
//  Assuming switch is active LOW
  pinMode(limitElbow, INPUT_PULLUP);
 
//  Assuming switch is active LOW
  pinMode(limitBase, INPUT_PULLUP);
 
//  Assuming switch is active LOW
  pinMode(Gripper_Limit, INPUT_PULLUP);
  pinMode(Switch, INPUT_PULLUP);

  
//  Enable stepper driver (LOW = enabled)
  digitalWrite(enablePin, LOW);

  Serial.begin(9600);
  Serial.println("Homing to origin...");

  
// -----------------------GO to Limit -----------------------------------------------------

  while ((digitalRead(limitSholder) == HIGH) && (digitalRead(limitElbow) == HIGH)) {
  
//  Move Sholder & Elbow stepper until limit switch is triggered
    digitalWrite(dirPinSholder, HIGH);
    digitalWrite(stepPinSholder, HIGH);
    delayMicroseconds(400);
  
//  Adjust speed
    digitalWrite(stepPinSholder, LOW);
    delayMicroseconds(400);

    digitalWrite(dirPinElbow, LOW);
    digitalWrite(stepPinElbow, HIGH);
    delayMicroseconds(400);
  
//  Adjust speed
    digitalWrite(stepPinElbow, LOW);
    delayMicroseconds(400);
  }

  while ((digitalRead(limitSholder) == HIGH)) {
  
//  Move Sholder stepper until limit switch is triggered
    digitalWrite(dirPinSholder, HIGH);
    digitalWrite(stepPinSholder, HIGH);
    delayMicroseconds(800);
  
//  Adjust speed
    digitalWrite(stepPinSholder, LOW);
    delayMicroseconds(800);
  }

  while ((digitalRead(limitElbow) == HIGH)) {
 
//  Move Elbow stepper until limit switch is triggered
    digitalWrite(dirPinElbow, LOW);
    digitalWrite(stepPinElbow, HIGH);
    delayMicroseconds(800);
  
//  Adjust speed
    digitalWrite(stepPinElbow, LOW);
    delayMicroseconds(800);
  }

  
// --------------------------------------------------------------------------------------

  
// -----------------------GO to GO to Orgin -----------------------------------------------------
  for (int i = 0;
 i < 2000;
 i++) {

    digitalWrite(dirPinSholder, LOW);
    digitalWrite(stepPinSholder, HIGH);
    delayMicroseconds(200);
  
//  Adjust speed
    digitalWrite(stepPinSholder, LOW);
    delayMicroseconds(200);

    digitalWrite(dirPinElbow, HIGH);
    digitalWrite(stepPinElbow, HIGH);
    delayMicroseconds(200);
  
//  Adjust speed
    digitalWrite(stepPinElbow, LOW);
    delayMicroseconds(200);
  }

  for (int i = 0;
 i < 1000;
 i++) {

    digitalWrite(dirPinElbow, HIGH);
    digitalWrite(stepPinElbow, HIGH);
    delayMicroseconds(400);
  
//  Adjust speed
    digitalWrite(stepPinElbow, LOW);
    delayMicroseconds(400);
  }

  while (digitalRead(limitBase) == HIGH) {
    
//  Move base stepper until limit switch is triggered
    digitalWrite(dirPinBase, LOW);
    digitalWrite(stepPinBase, HIGH);
    delayMicroseconds(800);
  
//  Adjust speed
    digitalWrite(stepPinBase, LOW);
    delayMicroseconds(800);
  }
  for (int base = 0;
 base < 5000;
 base++) {
  
//  Move base stepper until Mideel point 
// 3900 tbbe
    digitalWrite(dirPinBase, HIGH);
    digitalWrite(stepPinBase, HIGH);
    delayMicroseconds(400);
  
//  Adjust speed
    digitalWrite(stepPinBase, LOW);
    delayMicroseconds(400);
  }

  while (digitalRead(Gripper_Limit)) {
    
//  Rotate 45 degrees clockwise
    stepMotor(stepsPer45Deg, true);
  }
  delay(3000);

  
// --------------------------------------------------------------------------------------------
  
// delay(2000);
  Serial.println("Origin reached. Motor stopped.");
  digitalWrite(enablePin, HIGH);
 
//  Disable motor after homing
}

void loop() {

  if (digitalRead(Switch) == LOW) {

    One_circul();
  }
}

void One_circul() {

  stepMotorCCW(stepsPer45Deg, false);
      
//  Rotate 45 degrees counter-clockwise (open gripper)
  delay(1000);
  digitalWrite(enablePin, LOW);
 
//  Enable motor after homing

  for (int i = 0;
 i < 1500;
 i++) {

    digitalWrite(dirPinSholder, LOW);
    digitalWrite(stepPinSholder, HIGH);
    delayMicroseconds(400);
  
//  Adjust speed
    digitalWrite(stepPinSholder, LOW);
    delayMicroseconds(400);
  }
  
  for (int i = 0;
 i < 1000;
 i++) {

    digitalWrite(dirPinElbow, HIGH);
    digitalWrite(stepPinElbow, HIGH);
    delayMicroseconds(400);
  
//  Adjust speed
    digitalWrite(stepPinElbow, LOW);
    delayMicroseconds(400);

    digitalWrite(dirPinSholder, LOW);
    digitalWrite(stepPinSholder, HIGH);
    delayMicroseconds(400);
  
//  Adjust speed
    digitalWrite(stepPinSholder, LOW);
    delayMicroseconds(400);
  }

  while (digitalRead(Gripper_Limit)) {
    
//  Close gripper (grab object)
    stepMotor(stepsPer45Deg, true);
  }
  delay(2000);

  for (int i = 0;
 i < 500;
 i++) {

    digitalWrite(dirPinElbow, LOW);
    digitalWrite(stepPinElbow, HIGH);
    delayMicroseconds(400);
  
//  Adjust speed
    digitalWrite(stepPinElbow, LOW);
    delayMicroseconds(400);
    digitalWrite(dirPinSholder, HIGH);
    digitalWrite(stepPinSholder, HIGH);
    delayMicroseconds(400);
  
//  Adjust speed
    digitalWrite(stepPinSholder, LOW);
    delayMicroseconds(400);
  }

  for (int i = 0;
 i < 1500;
 i++) {

    digitalWrite(dirPinSholder, HIGH);
    digitalWrite(stepPinSholder, HIGH);
    delayMicroseconds(400);
  
//  Adjust speed
    digitalWrite(stepPinSholder, LOW);
    delayMicroseconds(400);
  }

  for (int base = 0;
 base < 3500;
 base++) {
  
//  Move base stepper until drop point
    digitalWrite(dirPinBase, LOW);
    digitalWrite(stepPinBase, HIGH);
    delayMicroseconds(800);
  
//  Adjust speed
    digitalWrite(stepPinBase, LOW);
    delayMicroseconds(800);
  }

  for (int i = 0;
 i < 500;
 i++) {

    digitalWrite(dirPinElbow, HIGH);
    digitalWrite(stepPinElbow, HIGH);
    delayMicroseconds(400);
  
//  Adjust speed
    digitalWrite(stepPinElbow, LOW);
    delayMicroseconds(400);
    digitalWrite(dirPinSholder, LOW);
    digitalWrite(stepPinSholder, HIGH);
    delayMicroseconds(400);
  
//  Adjust speed
    digitalWrite(stepPinSholder, LOW);
    delayMicroseconds(400);
  }

  for (int i = 0;
 i < 1500;
 i++) {

    digitalWrite(dirPinSholder, LOW);
    digitalWrite(stepPinSholder, HIGH);
    delayMicroseconds(400);
  
//  Adjust speed
    digitalWrite(stepPinSholder, LOW);
    delayMicroseconds(400);
  }

  
//  **MODIFIED: Open gripper to drop object and send signal to Mega**
  stepMotorCCW(stepsPer45Deg, false);
      
//  Open gripper (drop object)
  delay(1000);
  
  
//  Send signal to Arduino Mega that object has been dropped
  Serial.println("OBJECT_DROPPED");
  Serial.flush();
 
//  Make sure the message is sent immediately
  
  Serial.println("Object dropped - Signal sent to Mega");

  
// ------------------------------GO to Origin-----------------------------------------------

  for (int i = 0;
 i < 1000;
 i++) {

    digitalWrite(dirPinElbow, LOW);
    digitalWrite(stepPinElbow, HIGH);
    delayMicroseconds(400);
  
//  Adjust speed
    digitalWrite(stepPinElbow, LOW);
    delayMicroseconds(400);
    digitalWrite(dirPinSholder, HIGH);
    digitalWrite(stepPinSholder, HIGH);
    delayMicroseconds(400);
  
//  Adjust speed
    digitalWrite(stepPinSholder, LOW);
    delayMicroseconds(400);
  }

  for (int i = 0;
 i < 1500;
 i++) {

    digitalWrite(dirPinSholder, HIGH);
    digitalWrite(stepPinSholder, HIGH);
    delayMicroseconds(400);
  
//  Adjust speed
    digitalWrite(stepPinSholder, LOW);
    delayMicroseconds(400);
  }

  for (int base = 0;
 base < 3500;
 base++) {
  
//  Return to initial picking position
    digitalWrite(dirPinBase, HIGH);
    digitalWrite(stepPinBase, HIGH);
    delayMicroseconds(400);
  
//  Adjust speed
    digitalWrite(stepPinBase, LOW);
    delayMicroseconds(400);
  }

  while (digitalRead(Gripper_Limit)) {
    
//  Close gripper
    stepMotor(stepsPer45Deg, true);
  }
}

void stepMotor(int steps, bool clockwise) {

  for (int i = 0;
 i < steps;
 i++) {

    if (digitalRead(Gripper_Limit) == LOW) {

      stopMotor();
      return;
    }
    int stepIndex = clockwise ? i % 8 : (7 - i % 8);
    digitalWrite(IN1, stepSequence[stepIndex][0]);
    digitalWrite(IN2, stepSequence[stepIndex][1]);
    digitalWrite(IN3, stepSequence[stepIndex][2]);
    digitalWrite(IN4, stepSequence[stepIndex][3]);
    delay(2);
  
//  Adjust speed if needed
  }
}

void stepMotorCCW(int steps, bool clockwise) {

  for (int i = 0;
 i < steps;
 i++) {

    int stepIndex = clockwise ? i % 8 : (7 - i % 8);
    digitalWrite(IN1, stepSequence[stepIndex][0]);
    digitalWrite(IN2, stepSequence[stepIndex][1]);
    digitalWrite(IN3, stepSequence[stepIndex][2]);
    digitalWrite(IN4, stepSequence[stepIndex][3]);
    delay(2);
  
//  Adjust speed if needed
  }
}

void stopMotor() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}