#include <Arduino.h>
/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 */

#if defined(__SAMD51__)
// for Wio Terminal:
#include "rpcBLEDevice.h"
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "seeed_rpcUnified.h"
#include "rtl_ble/ble_unified.h"
#include "rtl_ble/ble_client.h"
#else
// for ESP32:
#include "BLEDevice.h"
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#endif

#define SCALE_MAIN_SERVICE_UUID "0000FFF0-0000-1000-8000-00805F9B34FB"
#define SCALE_READ_CHAR_UUID "0000FFF4-0000-1000-8000-00805F9B34FB"
#define SCALE_WRITE_CHAR_UUID "000036F5-0000-1000-8000-00805F9B34FB"

#define SCALE_PNP_SERVICE_UUID "0000180A-0000-1000-8000-00805F9B34FB"
#define SCALE_PNP_CHAR_UUID "00002A50-0000-1000-8000-00805F9B34FB"

#define MAX_NOTIFY_READ_CYCLE 100U

// Scale main service UUID
static BLEUUID serviceUUID(SCALE_MAIN_SERVICE_UUID);
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID_RD(SCALE_READ_CHAR_UUID);
static BLEUUID charUUID_WR(SCALE_WRITE_CHAR_UUID);

static boolean doConnect = true;
static boolean connected = false;
static boolean doScanRestart = false;
static boolean callBackRegistered = false;
static BLEScan *pBLEScan;
static BLERemoteCharacteristic *pReadCharacteristic;
static BLERemoteCharacteristic *pWriteCharacteristic;
static BLEAdvertisedDevice *myDevice;
#if defined(__SAMD51__)
uint8_t scale_BT_MAC_addr[6] = {0x1C, 0x03, 0x00, 0x10, 0xFF, 0xFF};
#else
uint8_t scale_BT_MAC_addr[6] = {0xFF, 0xFF, 0x10, 0x00, 0x03, 0x1C};
#endif
BLEAddress DecentScaleAddr(scale_BT_MAC_addr);
BLEClient *pScaleClient;
BLERemoteService *pScaleRemoteService;

uint8_t cmd_LedOn[7] = {0x03, 0x0A, 0x01, 0x01, 0x00, 0x00, 0x09};
uint8_t cmd_LedOff[7] = {0x03, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x09};
uint8_t cmd_TimerStop[7] = {0x03, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x08};
uint8_t cmd_TimerReset[7] = {0x03, 0x0B, 0x02, 0x00, 0x00, 0x00, 0x0A};

uint32_t call_back_counter;

void set_ScaleCmd(uint8_t *pCmd, std::string *pString_o)
{
  uint8_t i;
  for (i = 0; i < 7; i++)
  {
    pString_o->at(i) = pCmd[i];
  }
}

void Serial_println_string_in_hex(std::string *pString_i)
{
  uint32_t i;
  for (i = 0; i < pString_i->length(); i++)
  {
    if (pString_i->at(i) < 16)
    {
      Serial.print("0x0");
    }
    else
    {
      Serial.print("0x");
    }
    Serial.print(pString_i->at(i), HEX);
    Serial.print(",");
  }
  Serial.println("");
}

void Serial_println_chars_in_hex(uint8_t *pString_i, uint32_t len_i)
{
  uint32_t i;
  for (i = 0; i < len_i; i++)
  {
    if (pString_i[i] < 16)
    {
      Serial.print("0x0");
    }
    else
    {
      Serial.print("0x");
    }
    Serial.print(pString_i[i], HEX);
    Serial.print(",");
  }
  Serial.println("");
}

static void notifyCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
  /*
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  */
  if (call_back_counter < MAX_NOTIFY_READ_CYCLE)
  {
    call_back_counter++;
    if (call_back_counter % 10 == 0)
    {
      Serial.print("Scale data #");
      Serial.print(call_back_counter);
      Serial.print(": ");
      Serial_println_chars_in_hex(pData, length);
    }
  }
  else
  {
    pBLERemoteCharacteristic->registerForNotify(nullptr);
    delay(100);
    pBLERemoteCharacteristic->registerForNotify(nullptr);
    callBackRegistered = false;
    Serial.print("Call back unregistered.");
  }
}

class MyClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient *pCBclient)
  {
    connected = true;
    Serial.println("onConnect method");
  }

  void onDisconnect(BLEClient *pCBclient)
  {
    connected = false;
    Serial.println("onDisconnect method");
    doConnect = true;
  }
};

bool connectToServer()
{
  uint8_t read_cycle;
  std::string read_value = "123456789012345678901";
  bool doWriteCmd;

  doWriteCmd = false;
  pScaleClient = BLEDevice::getClient();
  if (pScaleClient == nullptr)
  {
    pScaleClient = BLEDevice::createClient();
    pScaleClient->setClientCallbacks(new MyClientCallback());
    doWriteCmd = true;
  }
  if (pScaleClient == nullptr)
  {
    Serial.println("Failed to create a new BLE client");
    return false;
  }
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());
  //Serial.println(DecentScaleAddr.toString().c_str());

  delay(100);
  // Connect to the remove BLE Server.
  pScaleClient->connect(myDevice); // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  //pScaleClient->connect(DecentScaleAddr);
  delay(100);
  if (pScaleClient->isConnected())
  {
    Serial.println(" - Connected to server");
  }
  else
  {
    Serial.println("Connection failed!");
    return false;
  }
  // Obtain a reference to the service we are after in the remote BLE server.
  delay(200);
  Serial.print(F("Searching for service: "));
  Serial.println(serviceUUID.toString().c_str());
  pScaleRemoteService = nullptr;
  pScaleRemoteService = pScaleClient->getService(serviceUUID);
  if (pScaleRemoteService == nullptr)
  {
    Serial.print(F(" - Failed to find our service, disconnecting."));
    pScaleClient->disconnect();
    return false;
  }
  Serial.println(F(" - Found our service"));
  if (doWriteCmd)
  {
    Serial.print(F("Searching for Write characteristic: "));
    Serial.println(charUUID_WR.toString().c_str());
    pWriteCharacteristic = pScaleRemoteService->getCharacteristic(charUUID_WR);
    if (pWriteCharacteristic == nullptr)
    {
      Serial.print(F("Failed to find our Write characteristic"));
    }
    else
    {
      Serial.println(F(" - Found our Write characteristic"));
      if (pWriteCharacteristic->canWrite())
      {
        Serial.println(F(" -  and it is writeable"));
        Serial.println(F("Sending double Led OFF command to scale "));
        pWriteCharacteristic->writeValue(cmd_LedOff, 7, false);
        delay(300);
        pWriteCharacteristic->writeValue(cmd_LedOff, 7, false);
        delay(300);
        Serial.println(F("Sending double Led ON command to scale "));
        pWriteCharacteristic->writeValue(cmd_LedOn, 7, false);
        delay(300);
        pWriteCharacteristic->writeValue(cmd_LedOn, 7, false);
        delay(300);
        Serial.println(F("Sending Timer Stop & Reset command to scale "));
        pWriteCharacteristic->writeValue(cmd_TimerStop, 7, false);
        delay(300);
        pWriteCharacteristic->writeValue(cmd_TimerReset, 7, false);
        delay(300);
      }
    }
  }

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  Serial.print(F("Searching for Read characteristic: "));
  Serial.println(charUUID_RD.toString().c_str());
  pReadCharacteristic = pScaleRemoteService->getCharacteristic(charUUID_RD);
  if (pReadCharacteristic == nullptr)
  {
    Serial.print("Failed to find our Read characteristic, disconnecting");
    pScaleClient->disconnect();
    return false;
  }
  Serial.println(" - Found our Read characteristic");

  // Read the value of the characteristic.
  if (pReadCharacteristic->canRead())
  {
    Serial.println(" - test reading started");
    read_cycle = 0;
    while (read_cycle < 5)
    {
      read_value = pReadCharacteristic->readValue();
      if (read_value.at(0) != 3)
      {
        read_cycle = 10;
      }
      Serial.print("Value: ");
      Serial_println_string_in_hex(&read_value);
      read_cycle++;
      delay(2000);
    }
  }

  if (pReadCharacteristic->canNotify())
  {
    Serial.println(" - Characteristic can notify");
    call_back_counter = 0;
    callBackRegistered = true;
    pReadCharacteristic->registerForNotify(notifyCallback);
    delay(200);
  }
  /*
  Serial.println("Disconnecting");
  pScaleClient->disconnect();
  connected = false;
  read_cycle = 0;
  */
  while (callBackRegistered && call_back_counter < MAX_NOTIFY_READ_CYCLE)
  {
    delay(1);
  }
  pWriteCharacteristic->~BLERemoteCharacteristic();
  pReadCharacteristic->~BLERemoteCharacteristic();
  pScaleClient->disconnect();
  delay(300);
  pScaleClient->disconnect();
  delay(300);
  Serial.println("Disconnected");
  doScanRestart = true;
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

    // We have found a device, let us now see if it contains the service we are looking for.
    if (memcmp(advertisedDevice.getAddress().getNative(), DecentScaleAddr.getNative(), 6) == 0)
    {
      Serial.print("Decent Scale found: ");
      Serial.println(advertisedDevice.toString().c_str());
      Serial.print("Address Type: ");
      Serial.println(advertisedDevice.getAddressType());
      BLEDevice::getScan()->stop();
      /*
      */
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScanRestart = true;
    } // onResult
    else
    {
      doConnect = false;
    }
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
  BLEDevice::init("Wio_Scale_Client");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
  /*
  */

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
      delay(1000);
      doConnect = true;
    }
    if (doScanRestart == true)
    {
      doScanRestart = false;
      /*
      Serial.println("BLE Scan stop");
      pBLEScan = BLEDevice::getScan();
      pBLEScan->stop();
      delay(2000);
      delay(2000);
      Serial.println("BLE Scan start");
      pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
      pBLEScan->setInterval(1349);
      pBLEScan->setWindow(449);
      pBLEScan->setActiveScan(true);
      pBLEScan->start(25, true);
      */
    }
  }
  else
  {
    doScanRestart = false;
  }
  Serial.print(".");
  delay(1000);
} // End of loop