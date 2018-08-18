#include <i2c_t3.h>
#include <VL53L0X.h>
#include <Bounce.h>
#include <SPI.h>
#include "RF24.h"

RF24 radio(9,10);
VL53L0X distSensor;

const uint64_t kDiscAddr = 0xF0F0F0F0E1LL;
const uint64_t kRemoteAddr = 0xF0F0F0F0D2LL;

#define MSG_HB       0x10
#define MSG_DISTANCE 0x20
#define MSG_BUTTON_1 0x30
#define MSG_BUTTON_2 0x31

#define MSG_BUTTON_DOWN 0x0
#define MSG_BUTTON_UP   0x1

#define BUTTON_PIN_1  23
#define BUTTON_PIN_2  22
#define LED_PIN       21

Bounce button1 = Bounce(BUTTON_PIN_1, 20);
Bounce button2 = Bounce(BUTTON_PIN_2, 20);

bool role = 0;
#define MESSAGE_LENGTH 3
byte sendMessage[MESSAGE_LENGTH] = { 0x0, 0x0, 0x0 };
byte receiveMessage[MESSAGE_LENGTH] = { 0x0, 0x0, 0x0 };
uint8_t receiveIdx = 0;

int rollingDist = 0;

unsigned long now;
unsigned long receiveTime = 0;
unsigned long blinkTime = 0;
unsigned long sendTime = 0;
unsigned long successTime = 0;
bool ledsHigh = false;

void setup() {
  // Teensy always operates at full speed
  Serial.begin(9600);

  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  Serial.println("Starting radio...");
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(kDiscAddr);
  radio.openReadingPipe(1,kRemoteAddr);
  radio.powerUp();
  radio.setAutoAck(true);
  radio.startListening();
  radio.printDetails();

  Serial.println("Starting I2C...");
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, I2C_RATE_400);
  
  Serial.println("Starting sensor init...");
  distSensor.init();
  distSensor.setTimeout(500);
  distSensor.setSignalRateLimit(0.1);
  distSensor.setMeasurementTimingBudget(20000);
  distSensor.startContinuous();

  Serial.println("Done init.");
}

void handleMessage() {
  if (receiveMessage[0] == MSG_HB) {
    receiveTime = now;
  }
}

bool txMessage(uint8_t v1, uint8_t v2) {
  radio.stopListening();
  sendMessage[0] = v1;
  sendMessage[1] = v2;
  bool success = radio.write(sendMessage, sizeof(byte) * MESSAGE_LENGTH);
  radio.startListening();
  return success;
}

void loop() {
  now = millis();
  
  if (now - blinkTime > 500) {
    blinkTime = now;
    ledsHigh = !ledsHigh;
    if (now - receiveTime < 500) {
      ledsHigh = true;
    }
    analogWrite(LED_PIN, ledsHigh ? 150 : 50);
  }

  int dist = distSensor.readRangeContinuousMillimeters();
  rollingDist += int(float(dist - rollingDist) * 0.3);
  float distVal = float(min(max(map(rollingDist, 50, 500, 0, 255), 0), 255)) / 255.0;

  if (button1.update()) {
    if (button1.fallingEdge()) {
      txMessage(MSG_BUTTON_1, MSG_BUTTON_DOWN);
    } else if (button1.risingEdge()) {
      txMessage(MSG_BUTTON_1, MSG_BUTTON_UP);
    }
  }
  if (button2.update()) {
    if (button2.fallingEdge()) {
      txMessage(MSG_BUTTON_2, MSG_BUTTON_DOWN);
    } else if (button2.risingEdge()) {
      txMessage(MSG_BUTTON_2, MSG_BUTTON_UP);
    }
  }

  if (now - sendTime > 50) {
    sendTime = now;
    bool success = txMessage(MSG_DISTANCE, distVal * 255);
    if (success) {
      successTime = now;
    }
  }

  if (radio.available()) {
    byte inChar;
    while (radio.available()) {
      radio.read(&inChar, sizeof(byte));
      receiveMessage[receiveIdx] = inChar;
      receiveIdx++;

      if (receiveIdx == MESSAGE_LENGTH) {
        handleMessage();
        receiveIdx = 0;
      }
    }
  }
}
