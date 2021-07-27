#include <Arduino.h>
/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 */

#include "rpcBLEDevice.h"
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
//#include "seeed_rpcUnified.h"
//#include "rtl_ble/ble_unified.h"

#define SCALE_SERVICE_UUID "0000FFF0-0000-1000-8000-00805F9B34FB"
// The remote service we wish to connect to.
//static BLEUUID serviceUUID(0xFEE0);
static BLEUUID serviceUUID(SCALE_SERVICE_UUID);
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID_RD("0000FFF4-0000-1000-8000-00805F9B34FB");
static BLEUUID charUUID_WR("000036F5-0000-1000-8000-00805F9B34FB");

static boolean doConnect = true;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic *pRemoteCharacteristic;
static BLEAdvertisedDevice *myDevice;
//uint8_t bd_addr[6] = {0xFF, 0xFF, 0x10, 0x00, 0x03, 0x1C};
uint8_t bd_addr[6] = {0x1C, 0x03, 0x00, 0x10, 0xFF, 0xFF};
BLEAddress BattServer(bd_addr);

uint8_t cmd_LedOn[7] = {0x03, 0x0A, 0x01, 0x01, 0x00, 0x00, 0x09};
uint8_t cmd_LedOff[7] = {0x03, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x09};
std::string strScaleCmd = "1234567";

void set_ScaleCmd(uint8_t *pCmd)
{
  uint8_t i;
  for (i = 0; i < 7; i++)
  {
    strScaleCmd[i] = pCmd[i];
  }
}

static void notifyCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  Serial.print("data: ");
  Serial.print(*(uint8_t *)pData);
}

class MyClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient *pclient)
  {
    connected = true;
    Serial.println("onConnect method");
  }

  void onDisconnect(BLEClient *pclient)
  {
    connected = false;
    Serial.println("onDisconnect method");
  }
};

bool connectToServer()
{
  uint8_t i;
  uint8_t read_cycle;
  std::string read_value = "1234567";
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());
  //Serial.println(BattServer.toString().c_str());

  BLEClient *pClient = BLEDevice::createClient();
  if (pClient == nullptr)
  {
    Serial.println(" - Failed to create client");
    return false;
  }
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(myDevice); // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  /*
  if (pClient->connect(BattServer, GAP_REMOTE_ADDR_LE_PUBLIC) == false)
  {
    Serial.println("Connection failed!");
    return false;
  }
  */
  delay(100);
  if (pClient->isConnected())
  {
    Serial.println(" - Connected to server");
  }
  else
  {
    Serial.println("Connection failed!");
    return false;
  }
  read_value = pClient->getValue(serviceUUID, charUUID_RD);
  Serial.print("The characteristic value was: ");
  for (i = 0; i < 8; i++)
  {
    Serial.print("0x");
    Serial.print(read_value[i], HEX);
    Serial.print(",");
  }
  Serial.println("");
  // Obtain a reference to the service we are after in the remote BLE server.
  Serial.println(serviceUUID.toString().c_str());
  //BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
  BLERemoteService *pRemoteService = pClient->getService(SCALE_SERVICE_UUID);
  if (pRemoteService == nullptr)
  {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID_WR);
  if (pRemoteCharacteristic == nullptr)
  {
    Serial.print("Failed to find our Write characteristic UUID: ");
    Serial.println(charUUID_WR.toString().c_str());
    pClient->disconnect();
    return false;
  }
  else
  {
    Serial.println(" - Found our Write characteristic");
    if (pRemoteCharacteristic->canWrite())
    {
      Serial.println(" -  and it is writeable");
      set_ScaleCmd(cmd_LedOn);
      Serial.print("Sending Led ON command to scale ");
      for (i = 0; i < 7; i++)
      {
        Serial.print("0x");
        Serial.print(strScaleCmd[i], HEX);
        Serial.print(",");
      }
      Serial.println("");
      pRemoteCharacteristic->writeValue(strScaleCmd, true);
      delay(250);
      set_ScaleCmd(cmd_LedOff);
      Serial.print("Sending Led OFF command to scale ");
      for (i = 0; i < 7; i++)
      {
        Serial.print("0x");
        Serial.print(strScaleCmd[i], HEX);
        Serial.print(",");
      }
      Serial.println("");
      pRemoteCharacteristic->writeValue(strScaleCmd, true);
      delay(250);
    }
  }

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID_RD);
  if (pRemoteCharacteristic == nullptr)
  {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID_RD.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

  // Read the value of the characteristic.
  std::string value = "1234567";
  if (pRemoteCharacteristic->canRead())
  {
    Serial.println(" -  can  read  start");
    read_cycle = 0;
    while (read_cycle < 10)
    {
      value.clear();
      value = pRemoteCharacteristic->readValue();
      if (value[0] != 3)
      {
        read_cycle = 10;
      }
      Serial.print("The characteristic value was: ");
      for (i = 0; i < 7; i++)
      {
        Serial.print("0x");
        Serial.print(value[i], HEX);
        Serial.print(",");
        value[i] = 0x31;
      }
      Serial.println("");
      read_cycle++;
      delay(2000);
    }
  }

  if (pRemoteCharacteristic->canNotify())
  {
    Serial.println(" -  can  notify");
    //pRemoteCharacteristic->registerForNotify(notifyCallback);
  }
  Serial.println("delay(10000)");
  delay(10000);
  Serial.println("disconnect");
  pClient->disconnect();
  connected = false;
  return false;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());
    //Serial.println(BattServer.toString());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (memcmp(advertisedDevice.getAddress().getNative(), BattServer.getNative(), 6) == 0)
    {
      Serial.print("BATT Client Device found: ");
      Serial.println(advertisedDevice.toString().c_str());
      Serial.print("Address Type: ");
      Serial.println(advertisedDevice.getAddressType());
      BLEDevice::getScan()->stop();
      Serial.println("new BLEAdvertisedDevice");
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      Serial.println("new BLEAdvertisedDevice done");
      doConnect = true;
      doScan = true;
    } // onResult
  }
}; // MyAdvertisedDeviceCallbacks

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
  };
  delay(2000);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("Scale_Client");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.

  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(15, false);

  doConnect = true;
} // End of setup.

// This is the Arduino main loop function.
void loop()
{

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (doConnect == true)
  {
    if (connectToServer())
    {
      Serial.println("We are now connected to the BLE Server.");
    }
    else
    {
      Serial.println("We are now disconnected from the BLE Server.");
    }
    //doConnect = false;
  }
  Serial.printf(".");
  delay(1000);
} // End of loop