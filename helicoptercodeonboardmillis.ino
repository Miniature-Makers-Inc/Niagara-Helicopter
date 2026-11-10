#include <Arduino.h>
#include <elapsedMillis.h>

#define beaconPin 5
#define strobeLeft 4
#define strobeRight 3
#define landingLight 2
#define navLight 1
#define statePin 0

elapsedMillis strobeTimer;
elapsedMillis beaconTimer;
elapsedMillis sequenceTimer;

int strobeDelay = 1000;
int strobeOnTime = 50;
int beaconDelay = 2000;
int beaconOff = 500;
int sequenceDelay = 3000;

bool beaconState_flag = false;
bool startUp_flag = true;

// Define the states
enum StartupState
{
  BEACON_STATE,
  STROBE_STATE,
  LANDING_STATE,
  NAV_STATE,
};

StartupState startupState = BEACON_STATE;
int stateVal;

// Function prototypes
void BEACON();
void STROBE();

void setup()
{
  pinMode(beaconPin, OUTPUT);
  pinMode(strobeLeft, OUTPUT);
  pinMode(strobeRight, OUTPUT);
  pinMode(landingLight, OUTPUT);
  pinMode(navLight, OUTPUT);
  pinMode(statePin, INPUT_PULLUP);

  // Everything off at startup
  digitalWrite(beaconPin, LOW);
  digitalWrite(strobeLeft, LOW);
  digitalWrite(strobeRight, LOW);
  digitalWrite(landingLight, LOW);
  digitalWrite(navLight, LOW);
}

void loop()
{
  stateVal = digitalRead(statePin);

  // --- STARTUP SEQUENCE ---
  if (stateVal == HIGH)
  {
    startUp_flag = false; // We are active/starting up
    
    if (sequenceTimer > sequenceDelay)
    {
      sequenceTimer = 0; 

      switch (startupState)
      {
        case BEACON_STATE:
          startupState = STROBE_STATE;
          break;
        case STROBE_STATE:
          startupState = LANDING_STATE;
          digitalWrite(landingLight, HIGH);
          break;
        case LANDING_STATE:
          startupState = NAV_STATE;
          digitalWrite(navLight, HIGH);
          break;
        case NAV_STATE:
          break;
      }
    }
  }
  // --- SHUTDOWN SEQUENCE ---
  // Changed condition to check if we are past BEACON_STATE
  else if (stateVal == LOW && startupState >= STROBE_STATE)
  {
    if (sequenceTimer > sequenceDelay)
    {
      sequenceTimer = 0;

      switch (startupState)
      {
        case NAV_STATE:
          digitalWrite(navLight, LOW);
          startupState = LANDING_STATE;
          break;
        case LANDING_STATE:
          digitalWrite(landingLight, LOW);
          startupState = STROBE_STATE;
          break;
        case STROBE_STATE:
          digitalWrite(strobeLeft, LOW);
          digitalWrite(strobeRight, LOW);
          startupState = BEACON_STATE;
          break;
        default:
          break;
      }
    }
  }
  // Fully shut down beacon when switch is LOW and we are back at BEACON_STATE
  else if (stateVal == LOW && startupState == BEACON_STATE)
  {
    digitalWrite(beaconPin, LOW);
    startUp_flag = true;
  }

  // --- RUN ACTIVE LIGHTS CONTINUOUSLY ---
  if (startupState >= BEACON_STATE && !startUp_flag) {
    BEACON();
  }
  
  if (startupState >= STROBE_STATE && !startUp_flag) {
    STROBE();
  }
}

// --- BEACON FUNCTION ---
void BEACON()
{
  if (!beaconState_flag && (beaconTimer > beaconDelay))
  {
    beaconTimer = 0;
    beaconState_flag = !beaconState_flag;
    digitalWrite(beaconPin, beaconState_flag ? HIGH : LOW);
  }
  else if (beaconState_flag && (beaconTimer > beaconOff))
  {
    beaconTimer = 0;
    beaconState_flag = !beaconState_flag;
    digitalWrite(beaconPin, beaconState_flag ? HIGH : LOW);
  }
}

// --- STROBE FUNCTION ---
void STROBE()
{
  static bool strobeState = false; 
  static bool strobeSide = false;  
  static bool isWaiting = false;   

  unsigned long currentLimit = isWaiting ? strobeOnTime : strobeDelay;

  if (strobeTimer >= currentLimit)
  {
    int activePin = strobeSide ? strobeRight : strobeLeft;
     digitalWrite(activePin, strobeState ? HIGH : LOW);
     strobeState = !strobeState;
     strobeTimer = 0;
    
    if (!strobeState)
    {
      if (!isWaiting)
      {
        isWaiting = true; 
      }
      else
      {
        digitalWrite(activePin, LOW);
        isWaiting = false; 
        strobeSide = !strobeSide;
        //digitalWrite(activePin, strobeState ? HIGH : LOW);
      }
    }
  }
}