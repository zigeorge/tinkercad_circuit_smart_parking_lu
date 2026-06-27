#include <Keypad.h>
#include <Servo.h>

// ---------- Servos ----------
Servo entranceServo;
Servo exitServo;

const int entranceServoPin = 10;
const int exitServoPin = 11;

const int gateClosedAngle = 0;
const int gateOpenAngle = 90;

// Signal to Parking Monitoring Arduino
// Gate Arduino D12 -> Parking Arduino D11
const int gateSignalPin = 12;

// ---------- One PIR Sensor for Both Gates ----------
const int pirPin = A0;

// ---------- Timing ----------
const unsigned long unlockDuration = 60000;    // 1 minute unlock state
const unsigned long clearConfirmTime = 800;    // PIR must be LOW for 0.8 sec before closing

// ---------- Keypad ----------
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 9};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ---------- Fixed Password ----------
const String parkingPassword = "1234";

// ---------- Input ----------
String inputBuffer = "";

// ---------- Unlock/Gate State ----------
bool unlockActive = false;
char unlockedGateMode = 'N';   // A = entrance, B = exit, N = none
unsigned long unlockStartedAt = 0;

bool gateOpen = false;
char openedGateMode = 'N';
unsigned long clearStartedAt = 0;

void setup() {
  Serial.begin(9600);

  entranceServo.attach(entranceServoPin);
  exitServo.attach(exitServoPin);

  entranceServo.write(gateClosedAngle);
  exitServo.write(gateClosedAngle);

  pinMode(gateSignalPin, OUTPUT);
  digitalWrite(gateSignalPin, LOW);

  pinMode(pirPin, INPUT);

  Serial.println("================================");
  Serial.println("Gate Access Node Started");
  Serial.println("================================");
  Serial.println("Fixed password: 1234");
  Serial.println();
  Serial.println("A1234# = Unlock entrance gate for 1 minute");
  Serial.println("B1234# = Unlock exit gate for 1 minute");
  Serial.println("PIR motion during unlock state opens selected gate");
  Serial.println("* = Clear input");
  Serial.println("--------------------------------");
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    handleKey(key);
  }

  updateUnlockState();
  updateGateByPir();
}

// ---------- Key Handling ----------
void handleKey(char key) {
  Serial.print("Key pressed: ");
  Serial.println(key);

  if (key == '*') {
    inputBuffer = "";
    Serial.println("Input cleared");
    return;
  }

  if (key == '#') {
    processInput(inputBuffer);
    inputBuffer = "";
    return;
  }

  if (inputBuffer.length() < 5) {
    inputBuffer += key;
    Serial.print("Current input: ");
    Serial.println(inputBuffer);
  } else {
    Serial.println("Input too long. Press * to clear.");
  }
}

// ---------- Input Processing ----------
// Expected format: A1234 or B1234
void processInput(String command) {
  Serial.print("Submitted command: ");
  Serial.println(command);

  if (command.length() != 5) {
    denyAccess("Invalid input. Use A1234# or B1234#");
    return;
  }

  char mode = command.charAt(0);
  String enteredPassword = command.substring(1, 5);

  if (mode != 'A' && mode != 'B') {
    denyAccess("First key must be A for entry or B for exit");
    return;
  }

  if (enteredPassword != parkingPassword) {
    denyAccess("Wrong password");
    printPayload(mode, "DENIED", "WRONG_PASSWORD");
    return;
  }

  activateUnlock(mode);
}

// ---------- Unlock State ----------
void activateUnlock(char mode) {
  unlockActive = true;
  unlockedGateMode = mode;
  unlockStartedAt = millis();

  if (mode == 'A') {
    Serial.println("Correct password. ENTRANCE gate unlocked for 1 minute.");
    printPayload(mode, "GRANTED", "ENTRANCE_UNLOCKED");
  } else {
    Serial.println("Correct password. EXIT gate unlocked for 1 minute.");
    printPayload(mode, "GRANTED", "EXIT_UNLOCKED");
  }

  Serial.println("Waiting for PIR motion near gate...");
}

void updateUnlockState() {
  if (!unlockActive) {
    return;
  }

  if (!gateOpen && millis() - unlockStartedAt >= unlockDuration) {
    unlockActive = false;
    unlockedGateMode = 'N';

    Serial.println("Unlock time expired. Gate locked again.");
  }
}

// ---------- PIR Gate Logic ----------
void updateGateByPir() {
  int pirState = digitalRead(pirPin);

  // If unlocked and gate is closed, PIR motion opens the selected gate
  if (unlockActive && !gateOpen && pirState == HIGH) {
    if (unlockedGateMode == 'A') {
      openEntranceGate();
    } else if (unlockedGateMode == 'B') {
      openExitGate();
    }
  }

  // If gate is open, close only after PIR becomes LOW
  if (gateOpen) {
    if (pirState == HIGH) {
      clearStartedAt = 0;
    } else {
      if (clearStartedAt == 0) {
        clearStartedAt = millis();
      }

      if (millis() - clearStartedAt >= clearConfirmTime) {
        closeOpenedGate();
      }
    }
  }
}

// ---------- Gate Open ----------
void openEntranceGate() {
  Serial.println("PIR detected motion. Opening ENTRANCE gate...");

  entranceServo.write(gateOpenAngle);
  digitalWrite(gateSignalPin, HIGH);

  gateOpen = true;
  openedGateMode = 'A';
  clearStartedAt = 0;

  printPayload('A', "GRANTED", "ENTRANCE_GATE_OPENED");
}

void openExitGate() {
  Serial.println("PIR detected motion. Opening EXIT gate...");

  exitServo.write(gateOpenAngle);
  digitalWrite(gateSignalPin, HIGH);

  gateOpen = true;
  openedGateMode = 'B';
  clearStartedAt = 0;

  printPayload('B', "GRANTED", "EXIT_GATE_OPENED");
}

// ---------- Gate Close ----------
void closeOpenedGate() {
  if (openedGateMode == 'A') {
    entranceServo.write(gateClosedAngle);
    Serial.println("PIR clear. ENTRANCE gate closed.");
    printPayload('A', "CLOSED", "ENTRANCE_GATE_CLOSED");
  } else if (openedGateMode == 'B') {
    exitServo.write(gateClosedAngle);
    Serial.println("PIR clear. EXIT gate closed.");
    printPayload('B', "CLOSED", "EXIT_GATE_CLOSED");
  }

  digitalWrite(gateSignalPin, LOW);

  gateOpen = false;
  openedGateMode = 'N';
  clearStartedAt = 0;
}

// ---------- Access Denied ----------
void denyAccess(String reason) {
  Serial.print("Access denied: ");
  Serial.println(reason);
}

// ---------- Simulated Core System Payload ----------
void printPayload(char mode, String accessResult, String eventType) {
  Serial.println("SIMULATED GATE CORE SYSTEM DATA");

  Serial.print("{\"nodeId\":\"LU-GATE-NODE-01\",");
  Serial.print("\"eventType\":\"");
  Serial.print(eventType);
  Serial.print("\",");

  Serial.print("\"mode\":\"");
  if (mode == 'A') {
    Serial.print("ENTRY");
  } else {
    Serial.print("EXIT");
  }
  Serial.print("\",");

  Serial.print("\"accessResult\":\"");
  Serial.print(accessResult);
  Serial.println("\"}");

  Serial.println();
}