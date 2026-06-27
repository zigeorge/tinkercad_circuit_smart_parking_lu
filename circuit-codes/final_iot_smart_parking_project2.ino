#include <Adafruit_LiquidCrystal.h>
#include <Adafruit_NeoPixel.h>

Adafruit_LiquidCrystal lcd(0);

#define NEOPIXEL_PIN 10
#define NUM_PIXELS 4

Adafruit_NeoPixel strip(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Parking sensors
const int totalSlots = 4;
int slotPins[4] = {2, 3, 4, 5};

// Obstacle sensors
const int totalObstacleSensors = 3;
int obstaclePins[3] = {6, 7, 8};

const int buzzerPin = 9;
const int gateSignalPin = 11;

// Thresholds
const int parkingThreshold = 15;   // cm
const int obstacleThreshold = 12;  // cm

// Data
long slotDistance[4];
long obstacleDistance[3];
bool slotOccupied[4];

unsigned long lastSerialUpdate = 0;
int lastAvailableCount = -1;

void setup() {
  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.setBacklight(1);

  strip.begin();
  strip.setBrightness(80);
  strip.clear();
  strip.show();

  pinMode(buzzerPin, OUTPUT);
  noTone(buzzerPin);

  pinMode(gateSignalPin, INPUT);

  setAllPixels(0, 255, 0);
  strip.show();

  showWelcomeMessage();

  lastAvailableCount = -1;
  updateLCD(totalSlots);

  Serial.println("Parking Monitoring Node Started");

  delay(1000);
}

void loop() {
  int occupiedCount = readParkingSlots();
  int availableCount = totalSlots - occupiedCount;

  int obstacleAlert = readObstacleSensors();

  if (digitalRead(gateSignalPin) == HIGH) {
    gateOpenAnimation();
  } else {
    updateNeoPixel(availableCount);
  }

  updateLCD(availableCount);
  updateBuzzer(obstacleAlert);

  if (millis() - lastSerialUpdate >= 2000) {
    printSerialStatus(occupiedCount, availableCount, obstacleAlert);
    lastSerialUpdate = millis();
  }

  delay(200);
}

// Show welcome message
void showWelcomeMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome to LU");

  lcd.setCursor(0, 1);
  lcd.print("Smart Parking");

  delay(2500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Starting");

  lcd.setCursor(0, 1);
  lcd.print("Please Wait...");

  delay(1500);
}

// ---------- Read Parking Sensors ----------
int readParkingSlots() {
  int occupied = 0;

  for (int i = 0; i < totalSlots; i++) {
    slotDistance[i] = readUltrasonic3Pin(slotPins[i]);

    if (slotDistance[i] > 0 && slotDistance[i] <= parkingThreshold) {
      slotOccupied[i] = true;
      occupied++;
    } else {
      slotOccupied[i] = false;
    }

    delay(30);
  }

  return occupied;
}

// ---------- Read Obstacle Sensors ----------
int readObstacleSensors() {
  int alertIndex = -1;
  long closestDistance = 9999;

  for (int i = 0; i < totalObstacleSensors; i++) {
    obstacleDistance[i] = readUltrasonic3Pin(obstaclePins[i]);

    if (obstacleDistance[i] > 0 && obstacleDistance[i] <= obstacleThreshold) {
      if (obstacleDistance[i] < closestDistance) {
        closestDistance = obstacleDistance[i];
        alertIndex = i;
      }
    }

    delay(30);
  }

  return alertIndex;
}

// ---------- 3-pin Ultrasonic Sensor ----------
long readUltrasonic3Pin(int sigPin) {
  pinMode(sigPin, OUTPUT);

  digitalWrite(sigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(sigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(sigPin, LOW);

  pinMode(sigPin, INPUT);

  long duration = pulseIn(sigPin, HIGH, 30000);

  if (duration == 0) {
    return -1;
  }

  long distance = duration * 0.034 / 2;
  return distance;
}

// ---------- LCD: Only available parking count ----------
void updateLCD(int availableCount) {
  if (availableCount == lastAvailableCount) {
    return;
  }

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Slots Available:");

  lcd.setCursor(0, 1);
  lcd.print(availableCount);

  lastAvailableCount = availableCount;
}

// ---------- NeoPixel Normal Parking Status ----------
void updateNeoPixel(int availableCount) {
  if (availableCount > 0) {
    setAllPixels(0, 255, 0);      // Green = parking available
  } else {
    setAllPixels(255, 0, 0);      // Red = no parking available
  }

  strip.show();
}

// ---------- NeoPixel Gate Animation ----------
void gateOpenAnimation() {
  setAllPixels(255, 0, 0);      // Red
  strip.show();
  delay(250);

  setAllPixels(0, 255, 0);      // Green
  strip.show();
  delay(250);

  setAllPixels(0, 0, 255);      // Blue
  strip.show();
  delay(250);

  setAllPixels(255, 180, 0);    // Yellow
  strip.show();
  delay(250);
}

void setAllPixels(int red, int green, int blue) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, strip.Color(red, green, blue));
  }
}

// ---------- Buzzer ----------
void updateBuzzer(int obstacleAlert) {
  if (obstacleAlert >= 0) {
    tone(buzzerPin, 1000);
    delay(100);
    noTone(buzzerPin);
  } else {
    noTone(buzzerPin);
  }
}

// ---------- Serial Monitor ----------
void printSerialStatus(int occupiedCount, int availableCount, int obstacleAlert) {
  Serial.println("--------------------------------");
  Serial.println("LU Smart Parking Status");

  for (int i = 0; i < totalSlots; i++) {
    Serial.print("Slot ");
    Serial.print(i + 1);

    if (i == 3) {
      Serial.print(" Disabled");
    } else {
      Serial.print(" General");
    }

    Serial.print(" | Distance: ");
    Serial.print(slotDistance[i]);
    Serial.print(" cm | ");

    if (slotOccupied[i]) {
      Serial.println("Occupied");
    } else {
      Serial.println("Available");
    }
  }

  Serial.print("Occupied Slots: ");
  Serial.println(occupiedCount);

  Serial.print("Available Slots: ");
  Serial.println(availableCount);

  Serial.print("Parking Status: ");
  if (availableCount > 0) {
    Serial.println("AVAILABLE");
  } else {
    Serial.println("FULL");
  }

  Serial.print("LEFT Obstacle: ");
  Serial.print(obstacleDistance[0]);
  Serial.println(" cm");

  Serial.print("FRONT Obstacle: ");
  Serial.print(obstacleDistance[1]);
  Serial.println(" cm");

  Serial.print("RIGHT Obstacle: ");
  Serial.print(obstacleDistance[2]);
  Serial.println(" cm");

  if (obstacleAlert == 0) {
    Serial.println("Obstacle Alert: LEFT");
  } else if (obstacleAlert == 1) {
    Serial.println("Obstacle Alert: FRONT");
  } else if (obstacleAlert == 2) {
    Serial.println("Obstacle Alert: RIGHT");
  } else {
    Serial.println("Obstacle Alert: NONE");
  }

  Serial.println("--------------------------------");
}