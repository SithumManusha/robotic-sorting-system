// =========================================================================
// Automated Robotic Sorting System - Conveyor & Sorting Controller
// Target Board: Arduino Mega 2560
// University of Moratuwa - Level 1 Hardware Project
// =========================================================================

#include <SPI.h>

#include <MFRC522.h>

#include <Servo.h>

#include <LiquidCrystal_I2C.h>


//  LCD Display setup (I2C address 0x27, 16x2 display)
LiquidCrystal_I2C lcd(0x27, 16, 2);


//  RFID pins

#define SS_PIN 53

#define RST_PIN 6  
//  Changed from 5 to 6 to free pin 5 for servoC
MFRC522 rfid(SS_PIN, RST_PIN);


//  Servo setup
Servo servoA;
 
//  MG995 - Group A - pin 10
Servo servoB;
 
//  MG996R - Group B - pin 9
Servo servoC;
 
//  MG995 - New servo - pin 5


//  Conveyor belt pins

#define stepPin 2

#define dirPin 3

#define enablePin 4


//  Ultrasonic sensor pins

#define TRIG_PIN_1 27  
//  Shared trigger for sensors 1 and 3

#define ECHO_PIN_1 41  
//  Echo pin for sensor 1 (D5:CB:3C:02)

#define TRIG_PIN_2 45  
//  Trigger for sensor 2

#define ECHO_PIN_2 46  
//  Echo pin for sensor 2 (12:7C:3B:02)

#define ECHO_PIN_3 35  
//  Echo pin for sensor 3 (2E:7B:28:02)


//  LED pins for ultrasonic sensors

#define LED_SENSOR_1 31  
//  LED for ultrasonic sensor 1 (Group B)

#define LED_SENSOR_2 42  
//  LED for ultrasonic sensor 2 (Group A)

#define LED_SENSOR_3 32  
//  LED for ultrasonic sensor 3 (Group C)


//  Buzzer pin

#define BUZZER_PIN 24


//  Detection counters for each sensor
int sensor1Count = 0;
  
//  Group B counter
int sensor2Count = 0;
  
//  Group C counter  
int sensor3Count = 0;
  
//  Group A counter


//  Cooldown variables
unsigned long lastScanTime = 0;
const unsigned long cooldown = 3000;
 
//  3 seconds cooldown after any card


//  Communication and belt control variables
bool rfidScanEnabled = true;
  
//  Flag to control RFID scanning
bool objectDropped = false;
   
//  Flag to track if object was dropped
unsigned long dropTime = 0;
  
//  Time when object was dropped


//  Origin and open positions for servoA and servoB
const int ORIGIN_A = 0;
     
//  MG995 origin (start) position
const int OPEN_A = 120;
     
//  MG995 open position
const int ORIGIN_B = 160;
   
//  MG996R origin adjusted for more downward travel
const int OPEN_B = 0;
       
//  MG996R open position


//  New servoC positions and control
const int ORIGIN_C = 0;
     
//  MG995 (servoC) origin position
const int OPEN_C = 180;
     
//  MG995 (servoC) open position
int servoCPos = ORIGIN_C;
   
//  Current position of servoC
bool servoCMovingToOpen = true;
 
//  Direction: true = moving to 180, false = moving to 0
unsigned long lastServoCStep = 0;
 
//  Time of last servoC step
const unsigned long SERVO_C_STEP_DELAY = 8;
 
//  Delay per degree (ms), ~2.88s cycle


//  Detection threshold - adjust this based on your setup
const float DETECTION_THRESHOLD = 8.0;
 
//  Detection threshold in cm


//  Belt movement distance (approximately 5cm)
const int BELT_MOVE_STEPS = 625;
 
//  Adjust based on your belt setup for ~5cm movement

void setup() {

  Serial.begin(9600);
  Serial1.begin(9600);
 
//  Initialize Serial1 for communication with Uno
  while (!Serial);
 
//  Wait for Serial to be ready
  
  Serial.println("Starting setup...");
  
  SPI.begin();
  Serial.println("SPI started");
  
  rfid.PCD_Init();
  Serial.println("RFID reader initialized");
  
  
//  Test RFID reader
  byte v = rfid.PCD_ReadRegister(rfid.VersionReg);
  Serial.print("RFID Reader Version: 0x");
  Serial.println(v, HEX);
  
  if((v == 0x00) || (v == 0xFF)) {

    Serial.println("WARNING: RFID reader not detected!");
  }
  
  
//  Attach servos
  servoA.attach(10);
  servoB.attach(9);
  servoC.attach(5);
 
//  Attach new MG995 servo to pin 5
  Serial.println("Servos attached");

  pinMode(LED_BUILTIN, OUTPUT);
  
  
//  Setup ultrasonic sensor pins
  pinMode(TRIG_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);
  pinMode(TRIG_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);
  pinMode(ECHO_PIN_3, INPUT);

  
//  Setup LED pins
  pinMode(LED_SENSOR_1, OUTPUT);
  pinMode(LED_SENSOR_2, OUTPUT);
  pinMode(LED_SENSOR_3, OUTPUT);
  
  
//  Setup buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  
//  Turn off all LEDs initially
  digitalWrite(LED_SENSOR_1, LOW);
  digitalWrite(LED_SENSOR_2, LOW);
  digitalWrite(LED_SENSOR_3, LOW);

  
//  Initialize LCD display
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sorting System");
  lcd.setCursor(0, 1);
  lcd.print("Ready...");
  Serial.println("LCD Display initialized");

  
//  Initialize all servos
  Serial.println("Initializing servos...");
  servoA.write(OPEN_A);
  delay(1000);
  servoA.write(ORIGIN_A);
  delay(300);

  servoB.write(OPEN_B);
  delay(1000);
  servoB.write(ORIGIN_B);
  delay(300);

  servoC.write(ORIGIN_C);
 
//  Initialize servoC to 0 degrees
  delay(300);
  Serial.println("Initialization complete. Ready for communication...");
  Serial.println("Servo C starting rotation cycle...");

  
//  Conveyor belt setup
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);

  digitalWrite(enablePin, HIGH);
 
//  Disable driver initially (HIGH = disable for A4988/DRV8825)
  digitalWrite(dirPin, HIGH);
   
//  Rotate in one direction
  
  
//  Check initial ultrasonic readings
  Serial.println("Initial ultrasonic sensor readings:");
  Serial.print("Sensor 1 (pin 41): ");
  Serial.print(getDistance(TRIG_PIN_1, ECHO_PIN_1));
  Serial.println(" cm");
  Serial.print("Sensor 2 (pin 46): ");
  Serial.print(getDistance(TRIG_PIN_2, ECHO_PIN_2));
  Serial.println(" cm");
  Serial.print("Sensor 3 (pin 35): ");
  Serial.print(getDistance(TRIG_PIN_1, ECHO_PIN_3));
  Serial.println(" cm");
}

void loop() {

  
//  Handle servoC smooth rotation (0 to 180 degrees and back, ~3s cycle)
  if (millis() - lastServoCStep >= SERVO_C_STEP_DELAY) {

    if (servoCMovingToOpen) {

      if (servoCPos < OPEN_C) {

        servoCPos++;
        servoC.write(servoCPos);
        
//  Serial.print("Servo C: Moving to ");
        
//  Serial.print(servoCPos);
        
//  Serial.println(" degrees");
      }
 else {

        servoCMovingToOpen = false;
 
//  Switch direction
      }
    }
 else {

      if (servoCPos > ORIGIN_C) {

        servoCPos--;
        servoC.write(servoCPos);
        
//  Serial.print("Servo C: Moving to ");
        
//  Serial.print(servoCPos);
        
//  Serial.println(" degrees");
      }
 else {

        servoCMovingToOpen = true;
 
//  Switch direction
      }
    }
    lastServoCStep = millis();
 
//  Update last step time
  }

  
//  Check for communication from Arduino Uno
  checkCommunication();
  
  
//  Handle belt movement after object drop
  handleObjectDrop();
  
  
//  Only scan RFID if enabled
  if (rfidScanEnabled) {

    handleRFIDScanning();
  }
}

void checkCommunication() {

  if (Serial1.available()) {

    String message = Serial1.readStringUntil('\n');
    message.trim();
    
    if (message.equals("OBJECT_DROPPED")) {

      Serial.println("Received: Object dropped signal from Uno");
      objectDropped = true;
      dropTime = millis();
      rfidScanEnabled = false;
 
//  Disable RFID scanning temporarily
      
      
//  Start belt movement immediately
      Serial.println("Starting belt movement for 5cm...");
      digitalWrite(enablePin, LOW);
 
//  Enable belt
      delay(100);
    }
  }
}

void handleObjectDrop() {

  if (objectDropped) {

    
//  Run belt for specified number of steps (approximately 5cm)
    Serial.println("Moving belt for 5cm...");
    
    
//  Enable belt
    digitalWrite(enablePin, LOW);
    delay(100);
    
    
//  Move belt for the specified distance
    for (int i = 0;
 i < BELT_MOVE_STEPS;
 i++) {

      runConveyor();
    }
    
    
//  Stop belt
    digitalWrite(enablePin, HIGH);
    Serial.println("Belt movement complete - RFID scanning enabled");
    
    
//  Reset RFID reader to ensure it's ready
    rfid.PCD_Init();
    delay(100);
    
    
//  Reset flags and re-enable RFID scanning
    objectDropped = false;
    rfidScanEnabled = true;
 
//  Re-enable RFID scanning
    lastScanTime = 0;
 
//  Reset scan cooldown immediately to allow immediate scanning
    
    Serial.println("System ready - You can now scan RFID cards");
  }
}

void handleRFIDScanning() {

  
//  RFID operation with cooldown
  if (millis() - lastScanTime < cooldown) {

    return;
 
//  Skip RFID scanning during cooldown
  }

  
//  Check for new card
  if (!rfid.PICC_IsNewCardPresent()) {

    return;
  }
  
  Serial.println("Card detected!");
  
  
//  Try to read the card
  if (!rfid.PICC_ReadCardSerial()) {

    Serial.println("Failed to read card");
    return;
  }
  
  Serial.println("Card read successfully!");

  String uidStr = getUIDString();
  Serial.print("Scanned UID: ");
  Serial.println(uidStr);

  
//  Handle different cards based on requirements
  if (uidStr.equals("D5:CB:3C:02") || uidStr.equals("73:68:3D:02")) {

    Serial.print("MATCH: ");
    Serial.print(uidStr);
    Serial.println(" → Starting belt and servo B, monitoring sensor 1 (pin 41)");
    
    
//  Display group on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Active: Group B");
    lcd.setCursor(0, 1);
    lcd.print("Sorting...");
    
    handleCardTypeB(TRIG_PIN_1, ECHO_PIN_1);
  }
  else if (uidStr.equals("12:7C:3B:02") || uidStr.equals("E6:5D:3D:02")) {

    Serial.print("MATCH: ");
    Serial.print(uidStr);
    Serial.println(" → Starting belt only, monitoring sensor 2 (pin 46)");
    
    
//  Display group on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Active: Group C");
    lcd.setCursor(0, 1);
    lcd.print("Belt Only...");
    
    handleCardBeltOnly(TRIG_PIN_2, ECHO_PIN_2);
  }
  else if (uidStr.equals("2E:7B:28:02") || uidStr.equals("4C:8B:27:02")) {

    Serial.print("MATCH: ");
    Serial.print(uidStr);
    Serial.println(" → Starting belt and servo A, monitoring sensor 3 (pin 35)");
    
    
//  Display group on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Active: Group A");
    lcd.setCursor(0, 1);
    lcd.print("Sorting...");
    
    handleCardTypeA(TRIG_PIN_1, ECHO_PIN_3);
  }
  else {

    Serial.println("Unknown card. No action.");
    
    
//  Display unknown card
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Unknown Card");
    lcd.setCursor(0, 1);
    lcd.print("Try Again...");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sorting System");
    lcd.setCursor(0, 1);
    lcd.print("Ready...");
  }

  rfid.PICC_HaltA();
  lastScanTime = millis();
 
//  Apply cooldown
}

void handleCardTypeB(int trigPin, int echoPin) {

  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Starting servo B and conveyor belt...");
  
  
//  Start servo B
  servoB.write(OPEN_B);
  delay(500);
  
  
//  Start conveyor belt
  digitalWrite(enablePin, LOW);
  delay(100);
  
  Serial.println("Running until sensor 1 detects object...");
  unsigned long startTime = millis();
  unsigned long lastDistanceTime = millis();
  
  
//  Run until ultrasonic sensor detects object
  while (!checkUltrasonicSensor(trigPin, echoPin)) {

    
//  Run conveyor
    for(int i = 0;
 i < 50;
 i++) {

      runConveyor();
    }
    
    
//  Print distance every 2 seconds
    if (millis() - lastDistanceTime >= 2000) {

      float distance = getDistance(trigPin, echoPin);
      Serial.print("Sensor 1 distance: ");
      Serial.print(distance);
      Serial.println(" cm");
      lastDistanceTime = millis();
    }
    
    
//  Safety timeout after 30 seconds
    if (millis() - startTime > 30000) {

      Serial.println("Safety timeout reached - stopping");
      break;
    }
  }
  
  Serial.println("Object detected or timeout - stopping belt and returning servo B");
  digitalWrite(enablePin, HIGH);
  
//  Stop belt
  delay(500);
  servoB.write(ORIGIN_B);
  
//  Return servo B to start position
  digitalWrite(LED_BUILTIN, LOW);
  
  
//  Always turn off LED after operation complete
  digitalWrite(LED_SENSOR_1, LOW);
  Serial.println("LED Sensor 1 OFF");
  
  
//  Reset counter after 2 detections
  if (sensor1Count >= 2) {

    sensor1Count = 0;
    Serial.println("Group B counter reset to 0");
  }
  
  
//  Display completion message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Group B Complete");
  lcd.setCursor(0, 1);
  lcd.print("Ready...");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sorting System");
  lcd.setCursor(0, 1);
  lcd.print("Ready...");
}

void handleCardTypeA(int trigPin, int echoPin) {

  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Starting servo A and conveyor belt...");
  
  
//  Start servo A
  servoA.write(OPEN_A);
  delay(500);
  
  
//  Start conveyor belt
  digitalWrite(enablePin, LOW);
  delay(100);
  
  Serial.println("Running until sensor 3 detects object...");
  unsigned long startTime = millis();
  unsigned long lastDistanceTime = millis();
  
  
//  Run until ultrasonic sensor detects object
  while (!checkUltrasonicSensor(trigPin, echoPin)) {

    
//  Run conveyor
    for(int i = 0;
 i < 50;
 i++) {

      runConveyor();
    }
    
    
//  Print distance every 2 seconds
    if (millis() - lastDistanceTime >= 2000) {

      float distance = getDistance(trigPin, echoPin);
      Serial.print("Sensor 3 distance: ");
      Serial.print(distance);
      Serial.println(" cm");
      lastDistanceTime = millis();
    }
    
    
//  Safety timeout after 30 seconds
    if (millis() - startTime > 30000) {

      Serial.println("Safety timeout reached - stopping");
      break;
    }
  }
  
  Serial.println("Object detected or timeout - stopping belt and returning servo A");
  digitalWrite(enablePin, HIGH);
  
//  Stop belt
  delay(500);
  servoA.write(ORIGIN_A);
  
//  Return servo A to start position
  digitalWrite(LED_BUILTIN, LOW);
  
  
//  Always turn off LED after operation complete
  digitalWrite(LED_SENSOR_3, LOW);
  Serial.println("LED Sensor 3 OFF");
  
  
//  Reset counter after 2 detections
  if (sensor3Count >= 2) {

    sensor3Count = 0;
    Serial.println("Group A counter reset to 0");
  }
  
  
//  Display completion message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Group A Complete");
  lcd.setCursor(0, 1);
  lcd.print("Ready...");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sorting System");
  lcd.setCursor(0, 1);
  lcd.print("Ready...");
}

void handleCardBeltOnly(int trigPin, int echoPin) {

  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Starting conveyor belt only...");
  
  
//  Start conveyor belt
  digitalWrite(enablePin, LOW);
  delay(100);
  
  Serial.println("Running until sensor 2 detects object...");
  unsigned long startTime = millis();
  unsigned long lastDistanceTime = millis();
  
  
//  Run until ultrasonic sensor detects object
  while (!checkUltrasonicSensor(trigPin, echoPin)) {

    
//  Run conveyor
    for(int i = 0;
 i < 50;
 i++) {

      runConveyor();
    }
    
    
//  Print distance every 2 seconds
    if (millis() - lastDistanceTime >= 2000) {

      float distance = getDistance(trigPin, echoPin);
      Serial.print("Sensor 2 distance: ");
      Serial.print(distance);
      Serial.println(" cm");
      lastDistanceTime = millis();
    }
    
    
//  Safety timeout after 30 seconds
    if (millis() - startTime > 30000) {

      Serial.println("Safety timeout reached - stopping");
      break;
    }
  }
  
  Serial.println("Object detected or timeout - stopping belt");
  digitalWrite(enablePin, HIGH);
  
//  Stop belt
  digitalWrite(LED_BUILTIN, LOW);
  
  
//  Always turn off LED after operation complete
  digitalWrite(LED_SENSOR_2, LOW);
  Serial.println("LED Sensor 2 OFF");
  
  
//  Reset counter after 2 detections
  if (sensor2Count >= 2) {

    sensor2Count = 0;
    Serial.println("Group C counter reset to 0");
  }
  
  
//  Display completion message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Group C Complete");
  lcd.setCursor(0, 1);
  lcd.print("Ready...");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sorting System");
  lcd.setCursor(0, 1);
  lcd.print("Ready...");
}

String getUIDString() {

  String uidStr = "";
  for (byte i = 0;
 i < rfid.uid.size;
 i++) {

    if (i > 0) uidStr += ":";
    if (rfid.uid.uidByte[i] < 0x10) uidStr += "0";
    uidStr += String(rfid.uid.uidByte[i], HEX);
  }
  uidStr.toUpperCase();
  return uidStr;
}

void runConveyor() {

  digitalWrite(stepPin, HIGH);
  delayMicroseconds(1000);
 
//  Increased delay to slow down the belt
  digitalWrite(stepPin, LOW);
  delayMicroseconds(1000);
 
//  Increased delay to slow down the belt
}

float getDistance(int trigPin, int echoPin) {

  
//  Clear the trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  
//  Send trigger pulse
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  
//  Read the echo pin with timeout
  long duration = pulseIn(echoPin, HIGH, 30000);
  
  
//  If timeout occurred
  if (duration == 0) {

    Serial.println("Warning: Ultrasonic sensor timeout - check connections");
    return 400;
 
//  Return large distance
  }
  
  
//  Calculate distance in cm
  float distance = duration * 0.034 / 2;
  
  
//  Filter out very small readings that might be noise
  if (distance < 2.0) {

    return 400;
 
//  Treat very small readings as no object
  }
  
  return distance;
}

bool checkUltrasonicSensor(int trigPin, int echoPin) {

  float distance = getDistance(trigPin, echoPin);
  
  if (distance < DETECTION_THRESHOLD) {

    Serial.print("Object detected at distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    
    
//  Count detections and handle LED/Buzzer for each sensor
    if (echoPin == ECHO_PIN_1) {

      sensor1Count++;
      Serial.print("Sensor 1 detection count: ");
      Serial.println(sensor1Count);
      
      if (sensor1Count >= 2) {

        digitalWrite(LED_SENSOR_1, HIGH);
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Group B Bin");
        lcd.setCursor(0, 1);
        lcd.print("is Full!");
        
        Serial.println("LED Sensor 1 ON - Group B bin is full!");
        delay(2000);
      }
    } 
    else if (echoPin == ECHO_PIN_2) {

      sensor2Count++;
      Serial.print("Sensor 2 detection count: ");
      Serial.println(sensor2Count);
      
      if (sensor2Count >= 2) {

        digitalWrite(LED_SENSOR_2, HIGH);
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Group C Bin");
        lcd.setCursor(0, 1);
        lcd.print("is Full!");
        
        Serial.println("LED Sensor 2 ON - Group C bin is full!");
        delay(2000);
      }
    } 
    else if (echoPin == ECHO_PIN_3) {

      sensor3Count++;
      Serial.print("Sensor 3 detection count: ");
      Serial.println(sensor3Count);
      
      if (sensor3Count >= 2) {

        digitalWrite(LED_SENSOR_3, HIGH);
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Group A Bin");
        lcd.setCursor(0, 1);
        lcd.print("is Full!");
        
        Serial.println("LED Sensor 3 ON - Group A bin is full!");
        delay(2000);
      }
    }
    
    return true;
  }
  return false;
}