#include <Arduino.h>
/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 */

#include <string.h>
#if defined(__SAMD51__)
// for Wio Terminal:
// Bluetooth driver:
#include "BLEDevice.h"
#include "Seeed_rpcUnified.h"
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
//Display driver:
#include "TFT_eSPI.h" //TFT LCD library
#include <SPI.h>
#include "Free_Fonts.h"
//#include "seeed_rpcUnified.h"
//#include "rtl_ble/ble_unified.h"
//#include "rtl_ble/ble_client.h"
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

#define MAX_NOTIFY_READ_CYCLE 50000U
#define DECENT_SCALE_PACKET_LEN 7

// Scale main service UUID
static BLEUUID serviceUUID(SCALE_MAIN_SERVICE_UUID);
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID_RD(SCALE_READ_CHAR_UUID);
static BLEUUID charUUID_WR(SCALE_WRITE_CHAR_UUID);

static boolean doConnect = true;
static boolean connected = false;
static boolean doScan = false;
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

uint8_t cmd_LedOn[DECENT_SCALE_PACKET_LEN] = {0x03, 0x0A, 0x01, 0x01, 0x00, 0x00, 0x09};
uint8_t cmd_LedOff[DECENT_SCALE_PACKET_LEN] = {0x03, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x09};
uint8_t cmd_TimerStop[DECENT_SCALE_PACKET_LEN] = {0x03, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x08};
uint8_t cmd_TimerReset[DECENT_SCALE_PACKET_LEN] = {0x03, 0x0B, 0x02, 0x00, 0x00, 0x00, 0x0A};

uint32_t call_back_counter;

#if defined(__SAMD51__)
TFT_eSPI tft; //initialize TFT LCD
TFT_eSprite status_display(&tft);
TFT_eSprite scale_display(&tft);
#endif

void wio_gpio_init(void)
{
#if defined(__SAMD51__)
  pinMode(WIO_KEY_A, INPUT);
  pinMode(WIO_KEY_B, INPUT);
  pinMode(WIO_KEY_C, INPUT);
  Serial.println("GPIO init completed.");
#endif
}

void wio_display_init(void)
{
#if defined(__SAMD51__)
  tft.begin();
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  Serial.println("Display init completed.");
#endif
}

void wio_set_background(void)
{
#if defined(__SAMD51__)
  //background.createSprite(320, 240);
  //background.fillSprite(TFT_LIGHTGREY);
  tft.fillScreen(TFT_WHITE);
  tft.fillRect(0, 0, 320, 60, TFT_DARKGREEN);
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(FSSB18);
  tft.drawFastHLine(0, 200, 320, TFT_BLUE);
  tft.setTextSize(1);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Decent Scale", 159, 15);
  //tft.pushSprite(0, 0);
  Serial.println("Display set background completed.");
#endif
}

void wio_status_update(char *pStatusMessage)
{
#if defined(__SAMD51__)
  status_display.createSprite(320, 38);
  status_display.fillSprite(TFT_LIGHTGREY);
  status_display.setFreeFont(FSS9);
  status_display.setTextColor(TFT_BLACK);
  status_display.drawString((const char *)pStatusMessage, 5, 10);
  status_display.pushSprite(0, 201);
#endif
}

void wio_weigth_display_update(float weight_i)
{
#if defined(__SAMD51__)
  char weight_str[8];
  scale_display.createSprite(320, 138);
  scale_display.fillSprite(TFT_WHITE);
  scale_display.setFreeFont(FSSB24);
  scale_display.setTextColor(TFT_BLACK);
  scale_display.setTextSize(2);
  snprintf(weight_str, sizeof(weight_str), "%5.1f", weight_i);
  scale_display.setTextDatum(TR_DATUM);
  scale_display.drawString((const char *)weight_str, 310, 30);
  scale_display.pushSprite(0, 61);
#endif
}

int16_t get_weight_tenthgramm_from_packet(char *pString_i)
{
  int8_t highByte;
  int8_t lowByte;
  int16_t ret_weight;
  highByte = (int8_t)pString_i[2];
  lowByte = (int8_t)pString_i[3];
  ret_weight = (int16_t)(((highByte & 0xFF) << 8) | (lowByte & 0xFF));
  return ret_weight;
}

float get_weight_gramm_from_packet(char *pString_i)
{
  int16_t tenthgramm;
  tenthgramm = get_weight_tenthgramm_from_packet(pString_i);
  return (float)(tenthgramm / 10.0);
}

void set_ScaleCmd(uint8_t *pCmd, std::string *pString_o)
{
  uint8_t i;
  for (i = 0; i < DECENT_SCALE_PACKET_LEN; i++)
  {
    pString_o->at(i) = pCmd[i];
  }
}

void Serial_print_string_in_hex(std::string *pString_i, uint32_t len_i)
{
  uint32_t i;
  uint32_t max_len;
  max_len = pString_i->length();
  if (len_i < max_len)
  {
    max_len = len_i;
  }
  for (i = 0; i < max_len; i++)
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
}

void Serial_println_string_in_hex(std::string *pString_i, uint32_t len_i)
{
  Serial_print_string_in_hex(pString_i, len_i);
  Serial.println("");
}

void Serial_print_chars_in_hex(uint8_t *pString_i, uint32_t len_i)
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
}

void Serial_println_chars_in_hex(uint8_t *pString_i, uint32_t len_i)
{
  Serial_print_chars_in_hex(pString_i, len_i);
  Serial.println("");
}

static void notifyCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
  float scale_weight;
  if (call_back_counter < MAX_NOTIFY_READ_CYCLE)
  {
    call_back_counter++;
    scale_weight = get_weight_gramm_from_packet((char *)pData);
    wio_weigth_display_update(scale_weight);
    if (call_back_counter % 10 == 0)
    {
      Serial.print("Scale data #");
      Serial.print(call_back_counter);
      Serial.print(": ");
      Serial_print_chars_in_hex(pData, length);
      Serial.print(" => ");
      Serial.print(scale_weight);
      Serial.println(" gr");
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
  char wio_status_msg[40];
  bool doWriteCmd;
  float scale_weight;

  wio_status_update((char *)"Connecting to scale...");
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
    Serial.println(" - Connected to scale");
    sprintf(wio_status_msg, "Connected to: %s", myDevice->getAddress().toString().c_str());
    wio_status_update(wio_status_msg);
  }
  else
  {
    Serial.println("Connection failed!");
    wio_status_update((char *)"Connection failed!");
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
        pWriteCharacteristic->writeValue(cmd_LedOff, DECENT_SCALE_PACKET_LEN, false);
        delay(250);
        pWriteCharacteristic->writeValue(cmd_LedOff, DECENT_SCALE_PACKET_LEN, false);
        delay(250);
        Serial.println(F("Sending Timer Reset command to scale "));
        pWriteCharacteristic->writeValue(cmd_TimerReset, DECENT_SCALE_PACKET_LEN, false);
        delay(250);
        Serial.println(F("Sending double Led ON command to scale "));
        pWriteCharacteristic->writeValue(cmd_LedOn, DECENT_SCALE_PACKET_LEN, false);
        delay(250);
        pWriteCharacteristic->writeValue(cmd_LedOn, DECENT_SCALE_PACKET_LEN, false);
        delay(250);
      }
    }
  }

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  Serial.print(F("Searching for Read characteristic: "));
  Serial.println(charUUID_RD.toString().c_str());
  pReadCharacteristic = nullptr;
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
    while (read_cycle < 1)
    {
      read_value = pReadCharacteristic->readValue();
      if (read_value.at(0) != 3)
      {
        read_cycle = 10;
      }
      scale_weight = get_weight_gramm_from_packet((char *)read_value.c_str());
      Serial.print("Value: ");
      Serial_print_string_in_hex(&read_value, DECENT_SCALE_PACKET_LEN);
      Serial.print(" => ");
      Serial.print(scale_weight);
      Serial.println(" gr");
      wio_weigth_display_update(scale_weight);
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
  while (callBackRegistered && call_back_counter < MAX_NOTIFY_READ_CYCLE)
  {
    delay(1);
  }
  pScaleClient->disconnect();
  delay(300);
  pScaleClient->disconnect();
  delay(300);
  Serial.println("Disconnected");
  connected = false;
  doScan = true;
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
      wio_status_update((char *)"Decent Scale found");
      Serial.print("Decent Scale found: ");
      Serial.println(advertisedDevice.toString().c_str());
      Serial.print("Address Type: ");
      Serial.println(advertisedDevice.getAddressType());
      BLEDevice::getScan()->stop();
      /*
      */
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
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
  wio_gpio_init();
  wio_display_init();
  wio_set_background();
  delay(1000);
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
  wio_status_update((char *)"Scanning...");
  /*
  */

  //doConnect = true;
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
    }
    doConnect = true;
  }
  if (connected == false && doScan == true)
  {
    Serial.println("BLE Scan re-start");
    BLEDevice::getScan()->start(0);
    /*
      Serial.println("BLE Scan stop");
      pBLEScan->stop();
      delay(2000);
      delay(2000);
      pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
      pBLEScan->setInterval(1349);
      pBLEScan->setWindow(449);
      pBLEScan->setActiveScan(true);
      pBLEScan->start(25, true);
      */
  }
  Serial.print(".");
  delay(1000);
} // End of loop