#include <i2c_t3.h>
#include <XBee.h>
#include <VL53L0X.h>
#include <Bounce.h>

#define LIGHT_GREEN_PIN     5
#define LIGHT_BLUE_PIN      6
#define MOMENTARY_WHITE_PIN 12
#define MOMENTARY_BLACK_PIN 11

XBee xbee = XBee();

Bounce whiteButton = Bounce(MOMENTARY_WHITE_PIN, 10);
Bounce blackButton = Bounce(MOMENTARY_BLACK_PIN, 10);
bool whiteButtonPressed = false;
bool blackButtonPressed = false;
unsigned long whiteButtonChangedTime = 0;
unsigned long blackButtonChangedTime = 0;

#define MSG_DIST  10
#define MSG_WHITE 20
#define MSG_BLACK 21
#define MSG_BOTH  22
#define MSG_HB    99

#define BRIGHTNESS_CONNECTED    127
#define BRIGHTNESS_DISCONNECTED 50

uint8_t payload[] = { 0, 0 };
int rollingDist = 0;

VL53L0X sensor;
int serverAddr = 0x1000;
Tx16Request tx = Tx16Request(serverAddr, payload, sizeof(payload));
TxStatusResponse txStatus = TxStatusResponse();
Rx16Response rx16 = Rx16Response();

void setup() {
  Serial.begin(57600);
  Serial.println("Starting...");
  delay(100);

  Serial1.begin(57600);
  xbee.setSerial(Serial1);

  pinMode(LIGHT_GREEN_PIN, OUTPUT);
  pinMode(LIGHT_BLUE_PIN, OUTPUT);
  pinMode(MOMENTARY_WHITE_PIN, INPUT_PULLUP);
  pinMode(MOMENTARY_BLACK_PIN, INPUT_PULLUP);

  analogWrite(LIGHT_GREEN_PIN, BRIGHTNESS_DISCONNECTED);
  analogWrite(LIGHT_BLUE_PIN, 0);

  Serial.println("Starting I2C...");
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, I2C_RATE_400);
  
  Serial.println("Starting sensor init...");
  bool finished = sensor.init();
  sensor.setTimeout(500);
  sensor.setSignalRateLimit(0.1);
  sensor.setMeasurementTimingBudget(20000);
  sensor.startContinuous();
  
  Serial.println("Done init!");
}

unsigned long now = 0;
unsigned long lastMsgReceived = 0;
unsigned long lastNoConnBlink = 0;
unsigned long lastSendDistance = 0;
unsigned long lastButtonPressSent = 0;
byte greenBrightness = BRIGHTNESS_DISCONNECTED;

void loop() {
  now = millis();
  
  int dist = sensor.readRangeContinuousMillimeters();
  rollingDist += int(float(dist - rollingDist) * 0.3);
  float distVal = float(min(max(map(rollingDist, 50, 500, 0, 255), 0), 255)) / 255.0;

  if (whiteButton.update()) {
    whiteButtonChangedTime = now;
    if (whiteButton.fallingEdge()) {
      whiteButtonPressed = true;
    } else {
      whiteButtonPressed = false;
    }
  }
  if (blackButton.update()) {
    blackButtonChangedTime = now;
    if (blackButton.fallingEdge()) {
      blackButtonPressed = true;
    } else {
      blackButtonPressed = false;
    }
  }

  if ((whiteButtonPressed || blackButtonPressed) && now - lastButtonPressSent > 100) {
    if (whiteButtonPressed && blackButtonPressed) {
      Serial.println("Sending both.");
      payload[0] = MSG_BOTH;
      xbee.send(tx);

      whiteButtonPressed = false;
      blackButtonPressed = false;
    } else if (whiteButtonPressed) {
      Serial.println("Sending white.");
      payload[0] = MSG_WHITE;
      xbee.send(tx);

      whiteButtonPressed = false;
    } else if (blackButtonPressed) {
      Serial.println("Sending black.");
      payload[0] = MSG_BLACK;
      xbee.send(tx);

      blackButtonPressed = false;
    }

    lastButtonPressSent = now;
  }

  if (now - lastSendDistance > 50) {
    Serial.print("Sending distance: ");
    Serial.println(int(distVal * 255));
  
    payload[0] = MSG_DIST;
    payload[1] = distVal * 255;
    xbee.send(tx);
    lastSendDistance = now;
  }

  if (now - lastMsgReceived > 1000) {
    if (now - lastNoConnBlink > 300) {
      greenBrightness = greenBrightness == BRIGHTNESS_DISCONNECTED ? BRIGHTNESS_CONNECTED : BRIGHTNESS_DISCONNECTED;
      lastNoConnBlink = now;
    }
  }

  // Write status LEDs
  analogWrite(LIGHT_GREEN_PIN, greenBrightness);
  analogWrite(LIGHT_BLUE_PIN, (1.0 - pow(distVal, 0.1)) * 255);

  // Xbee response
  xbee.readPacket();
  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
      xbee.getResponse().getTxStatusResponse(txStatus);
      if (txStatus.getStatus() == SUCCESS) {
        Serial.println("Successful send!");
      } else {
        Serial.println("Error in sending.");
      }
    } else if (xbee.getResponse().getApiId() == RX_16_RESPONSE) {
      lastMsgReceived = now;
      
      xbee.getResponse().getRx16Response(rx16);
      Serial.print("XBee received: ");
      for (int i = 0; i < rx16.getDataLength(); i++) { 
        Serial.write(rx16.getData(i)); 
      }
      Serial.println();
    } else {
      Serial.print("Got unrecognized packet: ");
      Serial.println(xbee.getResponse().getApiId());
    }
  } else if (xbee.getResponse().isError()) {
    // get the error code
    int errorCode = xbee.getResponse().getErrorCode();
    Serial.print("XBee error: ");
    Serial.println(errorCode);
  }
}

