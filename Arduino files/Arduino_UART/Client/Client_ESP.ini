#include <Arduino.h>
#include "BLEDevice.h"

// UUIDs of the remote service and its characteristics
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID servoUUID ("ab9b739b-aa65-4bc9-b18f-c7813805e78a"); // read/write
static BLEUUID accelUUID ("af62835c-2061-4920-9920-2028d50b94c3"); // notify
static BLEUUID gyroUUID ("9ea241b0-7f32-4433-ab54-70682ab76806"); // notify
static BLEUUID magnetUUID ("739281b2-6335-4253-ae75-834f4a45223f"); // notify

// Connection states
static bool doConnect = false; // set by the scan callback when our server is found
static bool connected = false; // true while a connection is up
static bool doScan = false; // allow loop() to rescan after a disconnect

// Handles to the remote characteristics, filled in by connectToServer()
static BLERemoteCharacteristic *pServoChar = nullptr;
static BLERemoteCharacteristic *pAccelChar = nullptr;
static BLERemoteCharacteristic *pGyroChar = nullptr;
static BLERemoteCharacteristic *pMagnetChar = nullptr;

// The server found during scanning
static BLEAdvertisedDevice *myDevice = nullptr;


// Latest sensor values, written by MyClientCallback::notifyCallback().
// REVIEW(9): written from the BLE task, read from loop(); no protection.

float accX, accY, accZ, std_accX, std_accY, std_accZ;
float gyroX, gyroY, gyroZ, std_gyroX, std_gyroY, std_gyroZ;
float magX, magY, magZ, std_magX, std_magY, std_magZ;
int check = 0 ;

// Client callbacks
class MyClientCallback : public BLEClientCallbacks {
 void onConnect(BLEClient *pclient) {}

 void onDisconnect(BLEClient *pclient) {
 connected = false;
 Serial.println("onDisconnect");
 }

public:
 // Callback function to handle notifications
 static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
 Serial.print("Client: Notify callback for characteristic ");
 Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
 Serial.print(" of data length ");
 Serial.println(length);
 Serial.print("Client: Data: ");
 Serial.write(pData, length);
 Serial.println();

 std::string charUUID = pBLERemoteCharacteristic->getUUID().toString().c_str();

 // Copy the payload into a local buffer so strtok() can split it.
 char buffer[64];
 size_t n = min(length, sizeof(buffer) - 1);
 memcpy(buffer, pData, n);
 buffer[n] = '\0';

 // Payload format: "x,y,z"
 // REVIEW(4): strtok() returns NULL if a field is missing; atof(NULL) crashes.
 char *token = strtok(buffer, ",");
 float number1 = atof(token);

 token = strtok(NULL, ",");
 float number2 = atof(token);

 token = strtok(NULL, ",");
 float number3 = atof(token);

 token = strtok(NULL, ",");
 float number4 = atof(token);

 token = strtok(NULL, ",");
 float number5 = atof(token);

 token = strtok(NULL, ",");
 float number6 = atof(token);

 // Route the three values to the globals of the right sensor.
 if (charUUID == accelUUID.toString().c_str()) {
  accX = number1;
  accY = number2;
  accZ = number3;
  std_accX = number4;
  std_accY = number5;
  std_accZ = number6;
 }
 if (charUUID == gyroUUID.toString().c_str()) {
  gyroX = number1;
  gyroY = number2;
  gyroZ = number3;
  std_gyroX = number4;
  std_gyroY = number5;
  std_gyroZ = number6;
 }
 if (charUUID == magnetUUID.toString().c_str()) {
  magX = number1;
  magY = number2;
  magZ = number3;
  std_magX = number4;
  std_magY = number5;
  std_magZ = number6;
 }
 }
};


// Connect to the server
bool connectToServer() {
 Serial.print("Forming a connection to ");
 Serial.println(myDevice->getAddress().toString().c_str());

 BLEClient *pClient = BLEDevice::createClient(); // REVIEW(10): new client per attempt
 Serial.println(" - Created client");

 pClient->setClientCallbacks(new MyClientCallback());

 // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
 if (!pClient->connect(myDevice)) {
   Serial.println(" - Connection failed");
   return false;
 }
 Serial.println(" - Connected to server");

 // Request the largest MTU
 pClient->setMTU(517);

 // Obtain a reference to the service we are after in the remote BLE server.
 BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
 if (pRemoteService == nullptr) {
 Serial.print("Failed to find our service UUID: ");
 Serial.println(serviceUUID.toString().c_str());
 pClient->disconnect();
 return false;
 }
 Serial.println(" - Found our service");

 // Servo characteristic
 pServoChar = pRemoteService->getCharacteristic(servoUUID);
 if (pServoChar == nullptr) {
 Serial.print("Failed to find servo characteristic UUID: ");
 Serial.println(servoUUID.toString().c_str());
 pClient->disconnect();
 return false;
 }
 Serial.println(" - Found servo characteristic");

 // Accelerometer characteristic
 pAccelChar = pRemoteService->getCharacteristic(accelUUID);
 if (pAccelChar == nullptr) {
 Serial.print("Failed to find accelerometer characteristic UUID: ");
 Serial.println(accelUUID.toString().c_str());
 pClient->disconnect();
 return false;
 }
 Serial.println(" - Found accelerometer characteristic");

 // Gyroscope characteristic
 pGyroChar = pRemoteService->getCharacteristic(gyroUUID);
 if (pGyroChar == nullptr) {
 Serial.print("Failed to find gyroscope characteristic UUID: ");
 Serial.println(gyroUUID.toString().c_str());
 pClient->disconnect();
 return false;
 }
 Serial.println(" - Found gyroscope characteristic");

 // Magnetometer characteristic
 pMagnetChar = pRemoteService->getCharacteristic(magnetUUID);
 if (pMagnetChar == nullptr) {
 Serial.print("Failed to find magnetometer characteristic UUID: ");
 Serial.println(magnetUUID.toString().c_str());
 pClient->disconnect();
 return false;
 }
 Serial.println(" - Found magnetometer characteristic");

 // Initial read of the servo characteristic
 if (pServoChar->canRead()) {
 String value = pServoChar->readValue();
 Serial.print("The characteristic value was: ");
 Serial.println(value.c_str());
 }

 // Subscribe to sensor notifications
 if (pAccelChar->canNotify()) {
 pAccelChar->registerForNotify(MyClientCallback::notifyCallback);
 }
 if (pGyroChar->canNotify()) {
 pGyroChar->registerForNotify(MyClientCallback::notifyCallback);
 }
 if (pMagnetChar->canNotify()) {
 pMagnetChar->registerForNotify(MyClientCallback::notifyCallback);
 }

 connected = true;
 return true;
}


// Scan for BLE servers and find the first one that advertises the service we are looking for.
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
 void onResult(BLEAdvertisedDevice advertisedDevice) {
 Serial.print("BLE Advertised Device found: ");
 Serial.println(advertisedDevice.toString().c_str());

 if (advertisedDevice.haveServiceUUID() &&
  advertisedDevice.isAdvertisingService(serviceUUID)) {
  BLEDevice::getScan()->stop();
  myDevice = new BLEAdvertisedDevice(advertisedDevice); // REVIEW(10): never freed
  doConnect = true;
  doScan = true;
 }
 }
};


// setup: start BLE and run an active scan for 5 s

void setup() {
 Serial.begin(115200);
 check = millis();
 Serial.println("Starting Arduino BLE Client application...");
 BLEDevice::init("");

 BLEScan *pBLEScan = BLEDevice::getScan();
 pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
 pBLEScan->setInterval(1349); // scan interval
 pBLEScan->setWindow(449);// scan window (must be <= interval)
 pBLEScan->setActiveScan(true);
 pBLEScan->start(5, false); // 5 s, do not keep earlier results
 
}


// main loop
void loop() {
 if (doConnect) {
 if (connectToServer()) {
  Serial.println("We are now connected to the BLE Server.");
 } else {
  Serial.println("We have failed to connect to the server; rescanning.");
 }
 doConnect = false;
 }

 if (connected) {
  if (millis() > (check + 3000)){
   // Print the latest sensor values
   Serial.printf("acc: %.2f, %.2f, %.2f  | std: %.2f, %.2f, %.2f \n", accX, accY, accZ, std_accX, std_accY, std_accZ);
   Serial.printf("gyro: %.2f, %.2f, %.2f | std: %.2f, %.2f, %.2f \n", gyroX, gyroY, gyroZ, std_gyroX, std_gyroY, std_gyroZ);
   Serial.printf("mag: %.2f, %.2f, %.2f  | std: %.2f, %.2f, %.2f \n", magX, magY, magZ, std_magX, std_magY, std_magZ);
   check = millis();
  }
   // Read a servo angle typed in the terminal and send it to the server
   if (Serial.available()) {
     String input = Serial.readStringUntil('\n');
     input.trim();
     int angle = input.toInt();

     if (input.length() > 0 && angle >= 0 && angle <= 90) {
       String value = String(angle);
       pServoChar->writeValue(value, true);
       Serial.println("Sent servo angle: " + value);
     } else {
       Serial.println("Invalid angle, use 0-90");
     }
   }
 } else if (doScan) {
 BLEDevice::getScan()->start(0);
 }

 delay(1000);
}