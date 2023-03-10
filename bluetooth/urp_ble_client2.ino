//#include <stdint.h>
//#include <stdlib.h>
//#include <string.h>
#include <BLEDevice.h>
//#include "BLEScan.h"


//static BLEUUID serviceUUID("0000ffe0-0000-1000-8000-00805f9b34fb");
//static BLEUUID charUUID("0000ffe1-0000-1000-8000-00805f9b34fb");
static BLEUUID serviceUUID("0000ff00-0000-1000-8000-00805f9b34fb");
static BLEUUID charUUID("0000ff01-0000-1000-8000-00805f9b34fb");
//static BLEUUID serviceUUID("FFE0");
//static BLEUUID charUUID("FFE1");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
int i=0;

uint8_t dest[] = {0x02, 0x11, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x03};
uint8_t dest1[] = {0x02, 0x11, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x03};
uint8_t dest2[] = {0x02, 0x11, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x03};
uint8_t dest3[] = {0x02, 0x11, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x03};
uint8_t dest4[] = {0x02, 0x11, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x03};


static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  
    // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

//    Serial.println("before");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
//    Serial.println(pRemoteService);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");

//    Serial.println("after");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}
//???????????? ?????? ?????? adverising device??? ????????? ??? ????????? ??????
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {

  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

   //?????? scanning?????? ?????? BLE device??? UUID??? ????????? ????????? ?????? ??? UUID?????????, scan??? ????????? BLEAdvertisedDevice??? ?????? device??? ??????
  //-> doConnect = true == connect ?????? ??????
  if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();//?????? ??????
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();//BLEScan* ?????? scan ?????? ??????
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  // scan interval??? window ????????? ??????????????? ?????? ???????????? ???. -> ??? ??????????????? ?????? ???????????? BLE ????????? ????????????
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);//Server setScanResponse(true/false)??? ??????
  pBLEScan->start(5, false);
} // End of setup.


// This is the Arduino main loop function.
void loop() {
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    
    
//    uint8_t dest[] = {02, 11, FF, 00, 00, 0x00, 0x00, 0x03};
//    String read_data = pRemoteCharacteristic->readValue().c_str();
    uint8_t *m_rawData = pRemoteCharacteristic->readRawData();
//    Serial.print("1: ");Serial.println(read_data);
    for(int i=0;i<8;i++){
      Serial.print("2: ");Serial.print(m_rawData[i]);Serial.print(", ");
    }
    Serial.println("");
    pRemoteCharacteristic->writeValue(dest, sizeof(dest));

  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
  }
  
  delay(1000); // Delay a second between loops.
} // End of loop
