// DeepSync Sleep Apnoea Monitor
// LED steady = normal
// LED blinking = no breathing for 10s
// LED fast blinking = no breathing + SpO2 below 95%
// LED off = no stable pulse / finger not detected

#include <Wire.h>
#include <SoftwareSerial.h>
#include "DFRobot_BloodOxygen_S.h"

// -------------------- PINS --------------------
const int stretchSensor = A0;
const int ledPin = 7;

SoftwareSerial BT(3, 6); // RX, TX

#define I2C_ADDRESS 0x57
DFRobot_BloodOxygen_S_I2C MAX30102(&Wire, I2C_ADDRESS);

// -------------------- BREATHING VARIABLES --------------------
float threshold = 512;
float smoothValue = 512;
const int breathHysteresis = 15;

unsigned long lastBreathTime = 0;
unsigned long lastMovementTime = 0;
unsigned long lastBluetoothSend = 0;

bool inhaling = false;
int lastRawValue = 512;

// -------------------- OXIMETER FILTERING --------------------
const int FILTER_SIZE = 5;

float hrHistory[FILTER_SIZE] = {72, 72, 72, 72, 72};
float spo2History[FILTER_SIZE] = {98, 98, 98, 98, 98};

int filterIndex = 0;

float spo2 = 98;
float heartRate = 72;

bool fingerPresent = false;
int noFingerCounter = 0;
const int noFingerLimit = 25;

// -------------------- SETUP --------------------
void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.begin(115200);
  BT.begin(9600);

  Wire.begin();
  Wire.setClock(400000);

  Serial.println("Initializing MAX30102...");
  BT.println("Initializing MAX30102...");

  while (!MAX30102.begin()) {
    Serial.println("MAX30102 missing - check wiring");
    BT.println("MAX30102 missing - check wiring");
    delay(2000);
  }

  MAX30102.sensorStartCollect();

  Serial.println("MAX30102 ready");
  BT.println("MAX30102 ready");

  // -------------------- BREATH SENSOR CALIBRATION --------------------
  Serial.println("Breathe normally for 5 seconds...");
  BT.println("Breathe normally for 5 seconds...");

  int lowest = 1023;
  int highest = 0;
  unsigned long startTime = millis();

  while (millis() - startTime < 5000) {
    int value = analogRead(stretchSensor);

    if (value < lowest) lowest = value;
    if (value > highest) highest = value;

    delay(10);
  }

  if (highest - lowest > 10) {
    threshold = (lowest + highest) / 2.0;
  } else {
    threshold = analogRead(stretchSensor);
  }

  smoothValue = analogRead(stretchSensor);
  lastRawValue = smoothValue;
  lastBreathTime = millis();
  lastMovementTime = millis();

  Serial.print("Calibration complete. Threshold: ");
  Serial.println(threshold);

  BT.print("Calibration complete. Threshold: ");
  BT.println(threshold);
}

// -------------------- MAIN LOOP --------------------
void loop() {

  // -------------------- READ BREATHING SENSOR --------------------
  int rawValue = analogRead(stretchSensor);
  smoothValue = (0.15 * rawValue) + (0.85 * smoothValue);

  if (abs(rawValue - lastRawValue) >= 10) {
    lastMovementTime = millis();
    lastRawValue = rawValue;
  }

  if (!inhaling && smoothValue > threshold + breathHysteresis) {
    inhaling = true;
    lastBreathTime = millis();
  } 
  else if (inhaling && smoothValue < threshold - breathHysteresis) {
    inhaling = false;
    lastBreathTime = millis();
  }

  bool noBreathing = (millis() - lastBreathTime > 10000) ||
                     (millis() - lastMovementTime > 10000);

  // -------------------- READ OXIMETER --------------------
  MAX30102.getHeartbeatSPO2();

  float rawSpO2 = MAX30102._sHeartbeatSPO2.SPO2;
  float rawHR = MAX30102._sHeartbeatSPO2.Heartbeat;

  bool usableReading = false;

  if (rawSpO2 >= 65 && rawSpO2 <= 100 && rawHR >= 35 && rawHR <= 200) {
    usableReading = true;
  }

  if (usableReading) {
    noFingerCounter = 0;
    fingerPresent = true;

    hrHistory[filterIndex] = rawHR;
    spo2History[filterIndex] = rawSpO2;

    filterIndex = (filterIndex + 1) % FILTER_SIZE;

    float hrSum = 0;
    float spo2Sum = 0;

    for (int i = 0; i < FILTER_SIZE; i++) {
      hrSum += hrHistory[i];
      spo2Sum += spo2History[i];
    }

    heartRate = hrSum / FILTER_SIZE;
    spo2 = spo2Sum / FILTER_SIZE;
  } 
  else {
    noFingerCounter++;

    if (noFingerCounter >= noFingerLimit) {
      fingerPresent = false;
    }
  }

  bool lowOxygen = fingerPresent && (spo2 < 95);

  // -------------------- LED CONTROL --------------------
  if (!fingerPresent) {
    digitalWrite(ledPin, LOW);
  } 
  else if (noBreathing && lowOxygen) {
    int fastBlink = (millis() / 100) % 2;
    digitalWrite(ledPin, fastBlink ? HIGH : LOW);
  }
  else if (noBreathing) {
    int slowBlink = (millis() / 300) % 2;
    digitalWrite(ledPin, slowBlink ? HIGH : LOW);
  }
  else {
    digitalWrite(ledPin, HIGH);
  }

  // -------------------- SERIAL + BLUETOOTH OUTPUT --------------------
  if (millis() - lastBluetoothSend > 2000) {
    lastBluetoothSend = millis();

    if (!fingerPresent) {
      Serial.println("No stable pulse yet - keep finger still on sensor");
      BT.println("No stable pulse yet - keep finger still on sensor");
    } 
    else {
      Serial.print("Breath: ");
      Serial.print(rawValue);

      Serial.print(" | SpO2: ");
      Serial.print((int)spo2);
      Serial.print("%");

      Serial.print(" | HR: ");
      Serial.print((int)heartRate);
      Serial.print(" BPM");

      if (noBreathing && lowOxygen) {
        Serial.println(" | No breathing + low oxygen");
      }
      else if (noBreathing) {
        Serial.println(" | No breathing detected");
      }
      else if (lowOxygen) {
        Serial.println(" | Low oxygen");
      }
      else {
        Serial.println(" | Normal");
      }

      BT.print("Breath: ");
      BT.print(rawValue);
      BT.print(" | SpO2: ");
      BT.print((int)spo2);
      BT.print("%");

      BT.print(" | HR: ");
      BT.print((int)heartRate);
      BT.print(" BPM");

      if (noBreathing && lowOxygen) {
        BT.println(" | No breathing + low oxygen");
      }
      else if (noBreathing) {
        BT.println(" | No breathing detected");
      }
      else if (lowOxygen) {
        BT.println(" | Low oxygen");
      }
      else {
        BT.println(" | Normal");
      }
    }
  }

  delay(20);
}
