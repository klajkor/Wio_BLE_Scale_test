#include <Arduino.h>
/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 */

#include "main_defs.h"
//#include <string.h>
#if defined(__SAMD51__)
// for Wio Terminal:
// Bluetooth driver:
#include "BLEDevice.h"
#include "seeed_rpcUnified.h"
#include <BLEAdvertisedDevice.h>
#include <BLEScan.h>
//#include "seeed_rpcUnified.h"
//#include "rtl_ble/ble_unified.h"
//#include "rtl_ble/ble_client.h"
#else
// for ESP32:
#include "BLEDevice.h"
#include <BLEAdvertisedDevice.h>
#include <BLEScan.h>
#endif

#include "counter.h"
#include "decent_scale.h"
#include "display.h"
#include "serial_print.h"
#include "weight_queue.h"

#if defined(__SAMD51__)
#include "LIS3DHTR.h"

LIS3DHTR<TwoWire> lis;
#define LIS_THRESHOLD 40
#endif

#define SCALE_MAIN_SERVICE_UUID "0000FFF0-0000-1000-8000-00805F9B34FB"
#define SCALE_READ_CHAR_UUID "0000FFF4-0000-1000-8000-00805F9B34FB"
#define SCALE_WRITE_CHAR_UUID "000036F5-0000-1000-8000-00805F9B34FB"

#define SCALE_PNP_SERVICE_UUID "0000180A-0000-1000-8000-00805F9B34FB"
#define SCALE_PNP_CHAR_UUID "00002A50-0000-1000-8000-00805F9B34FB"

// Scale main service UUID
static BLEUUID serviceUUID(SCALE_MAIN_SERVICE_UUID);
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID_RD(SCALE_READ_CHAR_UUID);
static BLEUUID charUUID_WR(SCALE_WRITE_CHAR_UUID);

static boolean                  doConnect = true;
static boolean                  connected = false;
static boolean                  doScan = false;
static boolean                  callBackRegistered = false;
static BLEScan *                pBLEScan;
static BLERemoteCharacteristic *pReadCharacteristic;
static BLERemoteCharacteristic *pWriteCharacteristic;
static BLEAdvertisedDevice *    myDevice;
#if defined(__SAMD51__)
// uint8_t scale_BT_MAC_addr[6] = {0x76, 0x01, 0x00, 0x10, 0xFF, 0xFF};
uint8_t scale_BT_MAC_addr[6] = {0x1C, 0x03, 0x00, 0x10, 0xFF, 0xFF};
#else
uint8_t scale_BT_MAC_addr[6] = {0xFF, 0xFF, 0x10, 0x00, 0x03, 0x1C};
#endif
BLEAddress        DecentScaleAddr(scale_BT_MAC_addr);
BLEClient *       pScaleClient;
BLERemoteService *pScaleRemoteService;

uint32_t call_back_counter;

char         accelero_str[32];
char         tap_cnt_str[32];
unsigned int tap_cnt = 0;

// Function defs for main.cpp only
static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length,
                           bool isNotify);
bool        connectToServer();
void        setup(void);
void        loop(void);

void tap_counter(void)
{
    tap_cnt++;
}

void wio_accelerometer_init(void)
{
#if defined(__SAMD51__)
    lis.begin(Wire1);
    lis.setOutputDataRate(LIS3DHTR_DATARATE_100HZ);
    lis.setFullScaleRange(LIS3DHTR_RANGE_2G);
    lis.setHighSolution(true);
    lis.click(1, LIS_THRESHOLD);
    attachInterrupt(digitalPinToInterrupt(GYROSCOPE_INT1), tap_counter, RISING);
    Serial.println("Accelerometer init completed.");
#endif
}

void wio_get_acceleroXYZ_str(char *pAcceleroStr_o)
{
#if defined(__SAMD51__)
    float x_values, y_values, z_values;
    lis.getAcceleration(&x_values, &y_values, &z_values);
    snprintf(pAcceleroStr_o, 30, "X:%6.3f Y:%6.3f Z:%6.3f", x_values, y_values, z_values);
#endif
}

void wio_gpio_init(void)
{
#if defined(__SAMD51__)
    pinMode(WIO_KEY_A, INPUT_PULLUP);
    pinMode(WIO_KEY_B, INPUT_PULLUP);
    pinMode(WIO_KEY_C, INPUT_PULLUP);
    pinMode(WIO_5S_PRESS, INPUT_PULLUP);
    pinMode(WIO_MIC, INPUT);
    Serial.println("GPIO init completed.");
#endif
}

static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length,
                           bool isNotify)
{
    float scale_weight;
    if (call_back_counter < MAX_NOTIFY_READ_CYCLE)
    {
        call_back_counter++;
        scale_weight = get_weight_gramm_from_packet((char *)pData);
        wio_weight_display_update(scale_weight);
        weight_q_push(&scale_weight_q, scale_weight);
        if (call_back_counter % 5 == 0)
        {
            StateMachine_counter1(scale_weight_q.total_diff, MIN_WEIGHT_INC);
        }
        if (call_back_counter % 10 == 0)
        {
            Serial.print("Scale data #");
            Serial.print(call_back_counter);
            Serial.print(": ");
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
        doScan = true;
    }
};

bool connectToServer()
{
    uint8_t     read_cycle;
    std::string read_value = "123456789012345678901";
    char        wio_status_msg[40];
    bool        doWriteCmd;
    float       scale_weight;

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
    // Serial.println(DecentScaleAddr.toString().c_str());

    delay(100);
    // Connect to the remove BLE Server.
    pScaleClient->connect(myDevice); // if you pass BLEAdvertisedDevice instead of address, it will be recognized type
                                     // of peer device address (public or private)
    delay(100);
    if (pScaleClient->isConnected())
    {
        Serial.println(" - Connected to scale");
        snprintf(wio_status_msg, 39, "Connected: %s", myDevice->getAddress().toString().c_str());
        wio_status_update(wio_status_msg);
    }
    else
    {
        Serial.println("Connection failed!");
        wio_status_update((char *)"Connection failed!");
        return false;
    }
    // Obtain a reference to the service we are after in the remote BLE server.
    delay(100);
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
                decent_cmd_init(pWriteCharacteristic);
                decent_cmd_led_off();
                decent_cmd_timer_reset();
                decent_cmd_timer_stop();
                decent_cmd_led_on();
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
            if (read_value.at(0) == 3)
            {
                scale_weight = get_weight_gramm_from_packet((char *)read_value.c_str());
                Serial.print("Value: ");
                serial_print_string_in_hex(&read_value, DECENT_SCALE_PACKET_LEN);
                Serial.print(" => ");
                Serial.print(scale_weight);
                Serial.println(" gr");
                wio_weight_display_update(scale_weight);
            }
            read_cycle++;
            delay(1000);
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
    decent_cmd_deinit();
    delay(100);
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
        // if (memcmp(advertisedDevice.getAddress().getNative(), DecentScaleAddr.getNative(), 6) == 0)
        if (is_decent_scale_mac_address(advertisedDevice.getAddress().getNative()))
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

void setup(void)
{
    Serial.begin(115200);
    delay(200);
    Serial.println("Setup started.");
    weight_q_reset(&scale_weight_q);
    wio_gpio_init();
    wio_display_init(display_rotation);
    wio_set_background();
    wio_accelerometer_init();
    StateMachine_counter1((float)(0.0), MIN_WEIGHT_INC);
    delay(3000);
    wio_weight_display_update(0.0);
    wio_brew_timer_update(0);
    Serial.println("Starting Arduino BLE Client application...");
    BLEDevice::init("Wio_Scale_Client");

    // Retrieve a Scanner and set the callback we want to use to be informed when we
    // have detected a new device.  Specify that we want active scanning and start the
    // scan to run for 5 seconds.
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    // pBLEScan->setInterval(1349);
    // pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(30);
    wio_status_update((char *)"Scanning...");
    /*
     */

    // doConnect = true;
} // End of setup.

// This is the Arduino main loop function.
void loop(void)
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
        // BLEDevice::getScan()->start(0);
        pBLEScan->setActiveScan(true);
        pBLEScan->start(30);
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