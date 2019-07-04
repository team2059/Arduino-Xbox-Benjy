//Joseph Schroedl FRC Team 2059

//A delay of 1000 Microseconds is Full Reverse
//A delay of 1000 to 1460 Microseconds is Proportional Reverse
//A delay of 1460 to 1540 Microseconds is neutral
//A delay of 1540 to 2000 Microseconds is Proportional Forward
//A delay of 2000 Microseconds is Full Forward

//For Xbox controller with USB shield
#include <Servo.h>
#include <XBOXRECV.h>

//Other Includes for USB shield
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

/*****User Configurable Variables*****/

//Motor Controller Deadzones. This zone keeps the motor controllers from fluctuation from off to +-1% speed.
const int lowNeutral = 1460;
const int highNeutral = 1540;

const int motorNeutral = 1500;

//This is deadzone threshold on the joystick because the resting position of the joystick varries. Making this value bigger will reqire the user to move the joystick further before the code starts using the joystick values
const int joystickDeadzone = 7500;

const int backLeftPin = 36;  //Back Left Motor pin
int backLeftSpeed = 1500; //Back Left Motor starting speed
Servo backLeftMotor;  //Create the backLeftMotor object for the servo library

const int frontLeftPin = 34;  //Front Left Motor pin
int frontLeftSpeed = 1500; //Front Left Motor starting speed
Servo frontLeftMotor; //Create the frontLeftMotor object for the servo library

const int backRightPin = 22;  //Back Right Motor pin
int backRightSpeed = 1500; //Back Right Motor starting speed
Servo backRightMotor; //Create the backRightMotor object for the servo library

const int frontRightPin = 24; //Front Right Motor pin
int frontRightSpeed = 1500; //Front Right Motor starting speed
Servo frontRightMotor; //Create the frontRightMotor object for the servo library

const int topRollerPin = 26;  //Top Roller Motor pin
//1460 for no running motors, no buttons held.
//1600 for low power, "X" button held.
//1800 for medium power, "A" button held.
//2000 for high power, "B" button held.
int topRollerSpeed = 1500;  //Top Roller Motor starting speed
Servo topRollerMotor; //Create the topRollerMotor object for the servo library

const int bottomRollerPin = 30; //Bottom Roller Motor pin
//1460 for no running motors, no buttons held.
//1600 for low power, "X" button held.
//1800 for medium power, "A" button held.
//2000 for high power, "B" button held.
int bottomRollerSpeed = 1500; //Top Roller Motor starting speed
Servo bottomRollerMotor; //Create the bottomRollerMotor object for the servo library

//Motor definition for the ball elevator/collector
const int collectorPin = 32;  //Collector pin
int collectorSpeed = 1500;  //Motor starting speed
Servo collectorMotor; //Create the collectorMotor object for the servo library

//Motor definitions for the turret
const int turretPin = 28; //Turret pin
int turretSpeed = motorNeutral; //Motor starting speed
Servo turretMotor;  //Create the turretMotor object for the servo library

const int demoToggleSwitch = 42;

//Use the Xbox controller number 0, I am only using one controller
const int controlNum = 0;

/*****Non-Configurable Variables*****/

short joyX = 0;             //joyX < 0 = Left, joyX > 0 = Right
short joyY = 0;             //joyY > 0 = Forward, joyY < 0 = Reverse

//Joystick turret value
short turretJoyX = 0;

//Variable to keep track of whether we are driving in any direction
bool driving = false;

//Variable to keep track of whether we are shooting
bool shooting = false;

//Variable to keep track of wether we are collecting
bool collecting = false;

//Variable to keep track of wether the turret is moving
bool turret = false;

bool demo = false;

//demoShooterLoop mode where the robot just shoots and collects.
bool demoShooterLoop = false;

//Initialization for USB shield
USB Usb;
XBOXRECV Xbox(&Usb);

void setup()
{
  //Initialize USB shield
  Usb.Init();
  //Wait for initialization before continueing
  delay(1000);

  //Setup for the motor controller outputs pins
  //Drive train
  backLeftMotor.attach(backLeftPin);
  frontLeftMotor.attach(frontLeftPin);
  backRightMotor.attach(backRightPin);
  frontRightMotor.attach(frontRightPin);
  //Other motors
  topRollerMotor.attach(topRollerPin);
  bottomRollerMotor.attach(bottomRollerPin);
  collectorMotor.attach(collectorPin);
  turretMotor.attach(turretPin);

  pinMode(demoToggleSwitch, INPUT_PULLUP);

  Serial.begin(2000000);

  //Wait for USB and other setup to finish. This delay is probably not necessary.
  delay(1000);
}

void stopMotors()
{
  backLeftMotor.writeMicroseconds(motorNeutral);
  frontLeftMotor.writeMicroseconds(motorNeutral);
  backRightMotor.writeMicroseconds(motorNeutral);
  frontRightMotor.writeMicroseconds(motorNeutral);

  topRollerMotor.writeMicroseconds(motorNeutral);
  bottomRollerMotor.writeMicroseconds(motorNeutral);

  collectorMotor.writeMicroseconds(motorNeutral);

  turretMotor.writeMicroseconds(motorNeutral);
}

void loop()
{
  demo = digitalRead(demoToggleSwitch);
  //Serial.println(demo);
    Usb.Task();
    if (Xbox.XboxReceiverConnected) {
      //This if statement makes sure that the motors will run only when the controller is connected. The motors will stop running when the controller is disconnected. This is the only way to disable the system other than cutting power.
      if (Xbox.Xbox360Connected[controlNum]) {

        //We start by saying we are not shooting, driving, or collecting
        driving = false;
        shooting = false;
        collecting = false;
        turret = false;

        //We have to use the "Val" to seperate it from LeftHatX which is a different varible in the library
        //The "LeftHat" is the left joystick
        //"Pre" = Joystick value before maping the values
        int LeftHatXValPre = 0;
        int LeftHatYValPre = 0;

        int rightHatXValPre = 0;

        //Grabs the xbox controller left analog stick data once, done here once to save cpu cycles
        int xboxLeftHatXData = Xbox.getAnalogHat(LeftHatX, controlNum);
        int xboxLeftHatYData = Xbox.getAnalogHat(LeftHatY, controlNum);

        //Grabs the xbox controller right analog stick data once, done here once to save cpu cycles
        int xboxRightHatXData = Xbox.getAnalogHat(RightHatX, controlNum);

        //This is deadzone detection on the joystick because the resting position of the joystick varries
        if (xboxLeftHatXData > joystickDeadzone || xboxLeftHatXData < -joystickDeadzone) {
          LeftHatXValPre = xboxLeftHatXData;
          driving = true;
          //Serial.println(LeftHatXValPre);
        }
        if (xboxLeftHatYData > joystickDeadzone || xboxLeftHatYData < -joystickDeadzone) {
          LeftHatYValPre = xboxLeftHatYData;
          driving = true;
          //Serial.println(LeftHatYValPre);
        }

        //This is deadzone detection on the joystick because the resting position of the joystick varries
        if (xboxRightHatXData > joystickDeadzone || xboxRightHatXData < -joystickDeadzone) {
          rightHatXValPre = xboxRightHatXData;
          turret = true;
          //Serial.println(rightHatXValPre);
        }

        //This section detects which buttons on the Xbox controller are being held and sets the shooter speed (shooterSpeed) based on which button pressed.
        //1460 for no running motors, no buttons held.
        //1600 for low power, "X" button held.
        //1800 for medium power, "A" button held.
        //2000 for high power, "B" button held.
        //topRollerSpeed and bottomRollerSpeed are set opposite because the shooter needs to spin rollers the opposite direction to pull the ball out
        if ((Xbox.getButtonPress(X, controlNum))) {
          if (demo) {
          topRollerSpeed = 1400;
          bottomRollerSpeed = 1600;
          }
          else {
          topRollerSpeed = 1400;
          bottomRollerSpeed = 1600;
          }
          shooting = true;
          //Serial.println("X button is being held");
        }
        else if ((Xbox.getButtonPress(A, controlNum))) {
          if (demo) {
          topRollerSpeed = 1325;
          bottomRollerSpeed = 1625;
          }
          else {
          topRollerSpeed = 1200;
          bottomRollerSpeed = 1800; 
          }
          shooting = true;
          //Serial.println("A button is being held");
        }
        else if ((Xbox.getButtonPress(B, controlNum))) {
          if (demo) {
          topRollerSpeed = 1325;
          bottomRollerSpeed = 1615;
          }
          else {
          topRollerSpeed = 1000;
          bottomRollerSpeed = 2000;
          }
          shooting = true;
          //Serial.println("B button is being held");
        }
        else {
          topRollerSpeed = 1500;
          bottomRollerSpeed = 1500;
          shooting = false;
          //Serial.println("No Shooter buttons are being held");
        }

        if (Xbox.getButtonPress(R1, controlNum)) {
          if (demo) {
          collectorSpeed = 1775;
          }
          else {
          collectorSpeed = 2000;
          }
          collecting = true;
        }
        else if (Xbox.getButtonPress(L1, controlNum)) {
          if (demo) {
          collectorSpeed = 1225;
          }
          else {
          collectorSpeed = 1000;
          }
          collecting = true;
        }
        else {
          collectorSpeed = 1500;
          collecting = false;
        }

        if (Xbox.getButtonPress(L2, controlNum)) {
          bottomRollerSpeed -= Xbox.getButtonPress(L2, controlNum);
        }

        if (Xbox.getButtonPress(R2, controlNum)) {
          topRollerSpeed += Xbox.getButtonPress(R2, controlNum);
        }

        if (Xbox.getButtonPress(START, controlNum)) {
          if (demoShooterLoop == false) {
            demoShooterLoop = true;
          }
          else if (demoShooterLoop == true) {
            demoShooterLoop = false;
          }
        }

        //Convert the joystick values
        if (demo) {
        joyY = map(LeftHatYValPre, -32768, 32768, 1400, 1650);
        joyX = map(LeftHatXValPre, -32768, 32768, -150, 150);
        }
        else {
        joyY = map(LeftHatYValPre, -32768, 32768, 1000, 2000);
        joyX = map(LeftHatXValPre, -32768, 32768, -150, 150); 
        }

        //Convert the joystick values for the turret. We only use the X axis of the joystick because the turret only turns left and right
        turretJoyX = map(rightHatXValPre, -32768, 32768, 1000, 2000);

        if (Xbox.getButtonPress(BACK, controlNum)) {
          demoShooterLoop = false;
        }
        
        if (demoShooterLoop) {
          //Shooter
          topRollerSpeed = 1325;
          bottomRollerSpeed = 1625;
          shooting = true;

          //Collecting
          collectorSpeed = 1775;
          collecting = true;
        }

        if (driving) {
          bool drivingForward = joyY > highNeutral;  //Are we driving?
          bool drivingReverse = joyY < lowNeutral;

          backLeftSpeed = frontLeftSpeed = backRightSpeed = frontRightSpeed = joyY;   //Sets the speed for all motors based on the Forward/Reverse of the joystick

          int absJoyX = abs(joyX);
          if (absJoyX > 7) {    //Am I moving the joystick left or right?
            if (joyX < 0 && (!drivingForward && !drivingReverse)) {     //Zero point turn Left
              backRightSpeed = highNeutral + absJoyX;   //highNeutral for forwards movement
              frontRightSpeed = highNeutral + absJoyX;  //lowNeutral for backwords movement
              backLeftSpeed = lowNeutral + joyX;
              frontLeftSpeed = lowNeutral + joyX;
            }
            else if (joyX > 0 && (!drivingForward && !drivingReverse)) {      //Zero point turn Right
              backRightSpeed = lowNeutral - joyX;
              frontRightSpeed = lowNeutral - joyX;
              backLeftSpeed = highNeutral + joyX;
              frontLeftSpeed = highNeutral + joyX;
            }
          }
        }
        if (turret) {
          turretSpeed = turretJoyX;
        }

        if (driving) {
          //If we moved the joystick to drive then set the motors to the speed determined by the joystick
          backLeftMotor.writeMicroseconds(backLeftSpeed);
          frontLeftMotor.writeMicroseconds(frontLeftSpeed);
          backRightMotor.writeMicroseconds(backRightSpeed);
          frontRightMotor.writeMicroseconds(frontRightSpeed);
        }
        else {
          //Else set the motors to neutral speed so they don't run
          backLeftMotor.writeMicroseconds(motorNeutral);
          frontLeftMotor.writeMicroseconds(motorNeutral);
          backRightMotor.writeMicroseconds(motorNeutral);
          frontRightMotor.writeMicroseconds(motorNeutral);
        }

        if (shooting) {
          //If we pressed "X", "A", or, "B" to run the shooter then set the motors to the speed determined by the button pressed
          topRollerMotor.writeMicroseconds(topRollerSpeed);
          bottomRollerMotor.writeMicroseconds(bottomRollerSpeed);
        }
        else {
          //Else set the motors to neutral speed so they don't run
          topRollerMotor.writeMicroseconds(motorNeutral);
          bottomRollerMotor.writeMicroseconds(motorNeutral);
        }

        if (collecting) {
          //If we pressed the bumper buttons to run the collector then set the motor to the preset speed
          collectorMotor.writeMicroseconds(collectorSpeed);
        }
        else {
          //Else set the motors to neutral speed so they don't run
          collectorMotor.writeMicroseconds(motorNeutral);
        }

        if (turret) {
          turretMotor.writeMicroseconds(turretSpeed);
        }
        else {
          turretMotor.writeMicroseconds(motorNeutral);
        }
      }
      else {
        //******************The code after this is for safety.******************
        //If the Xbox controller is not connected set the motors to neutral. The motors will not spin without the Xbox controller connected.
        stopMotors();
      }
    }
    else {
      //If the Xbox controller is not connected set the motors to neutral. The motors will not spin without the Xbox controller connected.
      stopMotors();
    }
}