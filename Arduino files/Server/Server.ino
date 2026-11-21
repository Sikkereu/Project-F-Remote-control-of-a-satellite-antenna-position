#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define SERVO_UUID "ab9b739b-aa65-4bc9-b18f-c7813805e78a"
#define ACCEL_UUID "af62835c-2061-4920-9920-2028d50b94c3"
#define GYRO_UUID "9ea241b0-7f32-4433-ab54-70682ab76806"
#define MAGNETOMETER_UUID "739281b2-6335-4253-ae75-834f4a45223f"
#define RXD2 6
#define TXD2 7
#define baudrate 115200

BLECharacteristic *servoCharacteristic = nullptr;
BLECharacteristic *accelCharacteristic = nullptr;
BLECharacteristic *gyroCharacteristic = nullptr;
BLECharacteristic *magnetometerCharacteristic = nullptr;

String servoData;
bool newData = 0;

//Callback that occurs when the server is written to
class CharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    servoData = pCharacteristic->getValue();
    newData = 1;
  }
};
/*

REMOVE DEBUG STAMENTS BEFORE HANDIN


*/
String readData(){
  String received;
  if (Serial1.available() > 0){
    Serial.println("entered function");
    received = Serial1.readStringUntil('\n');
    Serial.println(received);
  }
  return received;
}

void getIDPayload(String &ID, char *payload, int payloadSize){
  String data = readData();
  int seperator = data.indexOf(',');
  if(seperator != -1){
    ID = data.substring(0,seperator);
    data = data.substring(seperator + 1);
    data.toCharArray(payload,payloadSize);
    Serial.println(ID);
    Serial.println(payload);
    data = "";
    seperator = -1;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  if (!BLEDevice::init("BLE Server Example")) {
    Serial.println("BLE initialization failed!");
    return;
  }

  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pServer->advertiseOnDisconnect(true);
  servoCharacteristic =
    pService->createCharacteristic(SERVO_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  accelCharacteristic =
    pService->createCharacteristic(ACCEL_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  gyroCharacteristic =
    pService->createCharacteristic(GYRO_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  magnetometerCharacteristic =
    pService->createCharacteristic(MAGNETOMETER_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  servoCharacteristic->setValue("");
  accelCharacteristic->setValue("");
  gyroCharacteristic->setValue("");
  magnetometerCharacteristic->setValue("");
  servoCharacteristic->setCallbacks(new CharacteristicCallbacks());
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMaxPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}
char payload[64];
String ID;
void loop() {
  //Check for new data from other bluetooth device
  if(newData == 1){
    Serial1.println(servoData);
    newData == 0;
  }

  //Check for UART data and if new data is available read the ID and create payload
  getIDPayload(ID, payload, sizeof(payload));  

  //Send data based on ID
  if(ID == "accel"){
    accelCharacteristic->setValue(payload);
    accelCharacteristic->notify();
    ID = "";
  }
  else if(ID == "gyro"){
    gyroCharacteristic->setValue(payload);
    gyroCharacteristic->notify();
    ID = "";
  }
  else if(ID == "magnetometer"){
    magnetometerCharacteristic->setValue(payload);
    magnetometerCharacteristic->notify();
    ID = "";
  }
}
