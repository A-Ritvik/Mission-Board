//printing accel & gyro values to the display
#include <TFT_eSPI.h>
#include<Adafruit_MLX90393.h>
#include<SPI.h>
#include <Wire.h>
#include <L3G.h>
#include "SparkFun_LIS2DH12.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

L3G gyro;
SPARKFUN_LIS2DH12 accel;
Adafruit_MLX90393 mag = Adafruit_MLX90393();

const int I2C_SDA = 18;
const int I2C_SCL = 17;
const int CS_PIN = 7;
TFT_eSPI tft = TFT_eSPI();

bool connected = false;
bool moveReceived = false;
bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    tft.setCursor(10,100);
    tft.print("Phone Connected!!");
  }
  void onDisconnect (BLEServer* pServer) {
    deviceConnected = false;
    tft.setCursor(10,100);
    tft.println("Phone Disconnected :(");
    BLEDevice::startAdvertising();
  }
};

class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic -> getValue().c_str();
    if(rxValue.length()>0) {
      Serial.print("move command recieved");
    }
    if(rxValue.indexOf("Move") != -1) {
      moveReceived = true;
    }
  }
};
void setup() {
  // put your setup code here, to run once:
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("Initializing Gyroscope...");
  if(!gyro.init()){
    Serial.println("Failed to detect Gyroscope");
  }
  gyro.enableDefault();
  Serial.println("Gyroscope succesful");

  if(!mag.begin_I2C()){
    Serial.println("Magnetometer not found :(");
  }

  if(accel.begin() == false) {
    Serial.println("Failed to detected accelerometer :(");
  }

  BLEDevice::init("my mission board esp");

  BLEServer *pServer = BLEDevice::createServer();
  pServer ->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer-> createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService ->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );
  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising -> addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
  Serial.println("BLE is ACTIVE!!!!! :)");
  
  SPI.begin();
  pinMode(CS_PIN, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  gyro.read();
  tft.setCursor(10,10);
  tft.print("Gyro X: ");
  tft.print((int)gyro.g.x);
  tft.print(" | Y: ");
  tft.print((int)gyro.g.y);
  tft.print(" | Z: ");
  tft.println((int)gyro.g.z);

  float liveMagX = 0, liveMagY = 0, liveMagZ = 0;
  mag.readData(&liveMagX, &liveMagY, &liveMagZ);
  tft.setCursor(10,40);
  tft.print("Mag X: ");
  tft.print(liveMagX, 1);
  tft.print("uT ");
  tft.print("Mag Y: ");
  tft.print(liveMagY, 1);
  tft.print("uT ");

  float accelX = accel.getX();
  float accelY = accel.getY();
  float accelZ = accel.getZ();
  tft.setCursor(10, 70);
  tft.print("AccelX: ");
  tft.println(accelX, 1);
  tft.print("AccelY: ");
  tft.println(accelY, 1);
  tft.print("AccelZ: ");
  tft.println(accelZ, 1);
  delay(250);
  if(moveReceived) {
    Serial.println("esp --> RP2040 sending");
    sendCommand(0x01);
    moveReceived = false;
  }
  if(!moveReceived) {
    Serial.println("esp --> X RP2040 Braking");
    sendCommand(0x00);
  }
  delay(20);
}

void sendCommand(byte command) {
  digitalWrite(CS_PIN, LOW);
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  SPI.transfer(command);
  SPI.endTransaction();
  digitalWrite(CS_PIN, HIGH);
}
