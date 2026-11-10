
#include <AccelStepper.h>


int rotorSpeed = 0;
int lights = 15;
int rotor = 18;


//adjust flight variables
int hovertime1 = 2000;  //Time hovering before guy runs out
int hovertime2 = 2000;  //Time hovering before landing
int clearance = 5000;   //Time to spool rotor and wait for clearance to take off
int startup = 10000;    //Time to start lights and spool motor
int shutDown = 15000;   //Time for light shutdown and rotor spooldown
int height = 12750;     //Hover height (for leadscrew on NEMA17)
int angle2;
int waittime2;

#define home_switch 8  // Pin 8 connected to Home Switch (IR slot sensor)
//#define home_second xx // Pin 7 connected to home switch for runForrest

AccelStepper altitude(1, 0, 1);  //Setup easydriver 1  9 step 8 dir

long initial_homing = -1;  // Used to Home Stepper at startup

void setup() {  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  pinMode(lights, OUTPUT);
  pinMode(rotor, OUTPUT);
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
  altitude.setMaxSpeed(150);      //Set homing speed for altitude
  altitude.setAcceleration(150);  //Set homing acceleration for altitude

  pinMode(home_switch, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println("homing");
  digitalWrite(11, HIGH);              // Enable altitude motor
  while (!digitalRead(home_switch)) {  // Make the Stepper move CCW until the switch is activated
    altitude.moveTo(initial_homing);   // Set the position to move to
    initial_homing--;                  // Decrease by 1 for next move if needed
    altitude.run();                    // Start moving the stepper
    delay(5);
  }
  Serial.println("back");
  initial_homing = 1;
  while (digitalRead(home_switch)) {  // Make the Stepper move back until the switch is released
    altitude.moveTo(initial_homing);  // steps
    initial_homing++;                 // Increase by 1 for next move if needed
    altitude.run();
    delay(5);
  }
  altitude.setCurrentPosition(0);
  Serial.println("homing complete");
  Serial.println("reset speed");
  altitude.setMaxSpeed(1800);
  altitude.setAcceleration(200);
  digitalWrite(6, HIGH);
}

void loop() {  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  Serial.println("lights on");
  digitalWrite(lights, HIGH);  //turn on light sequence (self contained)
  Serial.println("wait");
  delay(startup);  //delay for light sequence and motor spool audio (add audio trigger later)

  Serial.println("motor spool");
  for (int rotorSpeed = 7; rotorSpeed <= 20; rotorSpeed += 1) {  //motor spool
    analogWrite(rotor, rotorSpeed);
    delay(700);
    Serial.println("up");
  }
  for (int rotorSpeed1 = 20; rotorSpeed1 <= 200; rotorSpeed1 += 5) {  //motor spool
    analogWrite(rotor, rotorSpeed1);
    delay(80);
    Serial.println("up again");
  }
  Serial.println("clearance");
  delay(clearance);  //waiting for clearance

  digitalWrite(6, LOW);   //enable the stepper
  Serial.println("takeoff");
  altitude.move(height);  //takeoff
  while (altitude.distanceToGo() != 0) { altitude.run(); }
 // digitalWrite(6, HIGH);
  Serial.println("hover");
  for (int i = 0; i < 20; i++) {  // oscillate in place for a short while
    altitude.setMaxSpeed(300);    //set oscillatin speed
    altitude.setAcceleration(80);
    angle2 = random(height - 300, height + 300);  //oscillation variance
    altitude.moveTo(angle2);
    Serial.println(angle2);
    Serial.println("  new spot");
    while (altitude.distanceToGo() != 0) {
      altitude.run();
    }
    waittime2 = random(0, 2000);
    delay(waittime2);
  }

  //digitalWrite(6, LOW);
  Serial.println("descent");
  altitude.moveTo(1);  // descent
  while (altitude.distanceToGo() != 0) { altitude.run(); }
  delay(2000);
  digitalWrite(6, HIGH);

  Serial.println("power down");
  for (int rotorSpeed = 180; rotorSpeed >= 0; rotorSpeed -= 1) {  //motor spool down
    analogWrite(rotor, rotorSpeed);
    delay(30);
    Serial.println("down");
  }
  delay(shutDown);

  Serial.println("lights off");
  digitalWrite(lights, LOW);  //lights off sequence
  delay(5000);

  Serial.println("done");
  delay(60000);
}
