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

#include "app_messages.h"
#include "app_timer.h"
#include "decent_scale.h"
#include "display.h"
#include "main.h"
#include "serial_print.h"
#include "stopwatch.h"
#include "weight_queue.h"
#include "wio_battery.h"
#include "wio_gpio.h"

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
static BLEScan *                pBLEScan;
static BLERemoteCharacteristic *pReadCharacteristic;
static BLERemoteCharacteristic *pWriteCharacteristic;
static BLEAdvertisedDevice *    myDevice;

BLEClient *       pScaleClient;
BLERemoteService *pScaleRemoteService;

static uint32_t display_cnt = 0;

static TimerHandle_t scale_read_timer;
bool                 do_scale_read = false;

static TimerHandle_t battery_status_update_timer;
bool                 do_battery_status_update = false;

class MyClientCallback : public BLEClientCallbacks
{
    void onConnect(BLEClient *pCBclient)
    {
        connected = true;
        serial_println(MSG_ONCONNECT_METHOD);
    }

    void onDisconnect(BLEClient *pCBclient)
    {
        serial_println(MSG_ONDISCONNECT_METHOD);
        connected = false;
        doConnect = true;
        doScan = true;
        decent_deinit();
        pCBclient->disconnect();
        delay(100);
        wio_ble_status_update(MSG_DISCONNECTED);
        serial_println(MSG_DISCONNECTED);
    }
};

bool connectToServer()
{
    uint8_t     read_cycle;
    std::string read_value = "123456789012345678901";
    char        wio_status_msg[40];
    bool        doWriteCmd;
    float       scale_weight;

    wio_ble_status_update(MSG_CONNECTING_TO_SCALE);
    serial_println(MSG_CONNECTING_TO_SCALE);
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
        serial_println(MSG_FAILED_TO_CREATE_BLE_CLIENT);
        wio_ble_status_update(MSG_FAILED_TO_CREATE_BLE_CLIENT);
        return false;
    }
    snprintf(wio_status_msg, 39, "%s: %s", MSG_CONNECTING_TO, myDevice->getAddress().toString().c_str());
    wio_ble_status_update(wio_status_msg);
    serial_println(wio_status_msg);

    delay(100);
    // Connect to the remove BLE Server.
    pScaleClient->connect(myDevice); // if you pass BLEAdvertisedDevice instead of address, it will be recognized type
                                     // of peer device address (public or private)
    delay(100);
    if (pScaleClient->isConnected())
    {
        snprintf(wio_status_msg, 39, "Connected: %s", myDevice->getAddress().toString().c_str());
        wio_ble_status_update(wio_status_msg);
        serial_println(wio_status_msg);
    }
    else
    {
        serial_println(MSG_CONNECTION_FAILED);
        wio_ble_status_update(MSG_CONNECTION_FAILED);
        return false;
    }
    // Obtain a reference to the service we are after in the remote BLE server.
    delay(100);
    serial_print(MSG_SEARCHING_FOR);
    serial_print(MSG_SERVICE);
    serial_println(serviceUUID.toString().c_str());
    pScaleRemoteService = nullptr;
    pScaleRemoteService = pScaleClient->getService(serviceUUID);
    if (pScaleRemoteService == nullptr)
    {
        serial_print(MSG_FAILED_TO_FIND);
        serial_print(MSG_SERVICE);
        serial_println(MSG_DISCONNECTING);
        wio_ble_status_update(MSG_DISCONNECTING);
        pScaleClient->disconnect();
        return false;
    }
    serial_print(MSG_FOUND);
    serial_println(MSG_SERVICE);
    if (doWriteCmd)
    {
        serial_print(MSG_SEARCHING_FOR);
        serial_print(MSG_WRITE_CHARACTERISTIC);
        serial_println(charUUID_WR.toString().c_str());
        pWriteCharacteristic = pScaleRemoteService->getCharacteristic(charUUID_WR);
        if (pWriteCharacteristic == nullptr)
        {
            serial_print(MSG_FAILED_TO_FIND);
            serial_println(MSG_WRITE_CHARACTERISTIC);
        }
        else
        {
            serial_println(MSG_FOUND);
            serial_println(MSG_WRITE_CHARACTERISTIC);
            if (pWriteCharacteristic->canWrite())
            {
                serial_println(MSG_WRITEABLE);
                decent_write_init(pWriteCharacteristic);
                delay(50);
                decent_cmd_led_off();
                delay(50);
                decent_cmd_timer_stop();
                delay(50);
                decent_cmd_timer_stop();
                delay(50);
                decent_cmd_timer_reset();
                delay(50);
                decent_cmd_led_on();
            }
            else
            {
                serial_println(MSG_NOT_WRITEABLE);
            }
        }
    }

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    serial_print(MSG_SEARCHING_FOR);
    serial_print(MSG_READ_CHARACTERISTIC);
    serial_println(charUUID_RD.toString().c_str());
    pReadCharacteristic = nullptr;
    pReadCharacteristic = pScaleRemoteService->getCharacteristic(charUUID_RD);
    if (pReadCharacteristic == nullptr)
    {
        serial_print(MSG_FAILED_TO_FIND);
        serial_print(MSG_READ_CHARACTERISTIC);
        serial_println(MSG_DISCONNECTING);
        wio_ble_status_update(MSG_DISCONNECTING);
        pScaleClient->disconnect();
        return false;
    }
    serial_print(MSG_FOUND);
    serial_print(MSG_READ_CHARACTERISTIC);

    // Read the value of the characteristic.
    if (pReadCharacteristic->canRead())
    {
        serial_println(MSG_TEST_READING_STARTED);
        read_cycle = 0;
        while (read_cycle < 1)
        {
            read_value = pReadCharacteristic->readValue();
            if (read_value.at(0) == 3)
            {
                scale_weight = get_weight_gramm_from_packet((char *)read_value.c_str());
                serial_print("Value: ");
                serial_print_string_in_hex(&read_value, DECENT_SCALE_PACKET_LEN);
                serial_print(" => ");
                snprintf(wio_status_msg, 39, "%f gr", scale_weight);
                serial_println(wio_status_msg);
                wio_weight_display_update(scale_weight);
            }
            read_cycle++;
            delay(1000);
        }
        decent_read_init(pReadCharacteristic);
    }
    return true;
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
        char wio_status_msg[40];
        snprintf(wio_status_msg, 39, "%s: %s", MSG_SCANNING, advertisedDevice.getAddress().toString().c_str());
        wio_ble_status_update(wio_status_msg);
        serial_println(wio_status_msg);

        // We have found a device, let us now see if it contains the service we are looking for.
        if (is_decent_scale_mac_address(advertisedDevice.getAddress().getNative()))
        {
            snprintf(wio_status_msg, 39, "%s: %s", MSG_DECENT_SCALE_FOUND,
                     advertisedDevice.getAddress().toString().c_str());
            wio_ble_status_update(wio_status_msg);
            serial_print(wio_status_msg);
            BLEDevice::getScan()->stop();
            myDevice = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
            doScan = false;
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
    serial_println(MSG_SETUP_STARTED);
    weight_q_reset(&scale_weight_q);
    battery_init();
    wio_gpio_init();
    wio_display_init(display_rotation);
    wio_set_background();
    // StateMachine_counter1((float)(0.0), MIN_WEIGHT_INC);
    delay(2000);
    fsm_stopwatch(STOPWATCH_RESET);
    wio_battery_status_update();
    wio_weight_display_update(0.0);
    scale_read_timer =
        xTimerCreate("scale_read_timer", PERIOD_SCALE_READ_TASK, pdTRUE, (void *)0, scale_read_timer_callback);
    if (scale_read_timer == NULL)
    {
        serial_println(MSG_TIMER_CREATE_ERROR);
    }
    else
    {
        serial_println(MSG_TIMER_CREATED);
        xTimerStart(scale_read_timer, 0);
    }
    battery_status_update_timer = xTimerCreate("battery_status_update_timer", PERIOD_BATTERY_STATUS_UPDATE, pdTRUE,
                                               (void *)0, battery_status_update_callback);
    if (battery_status_update_timer == NULL)
    {
        serial_println(MSG_TIMER_CREATE_ERROR);
    }
    else
    {
        serial_println(MSG_TIMER_CREATED);
        xTimerStart(battery_status_update_timer, 0);
    }

    serial_println(MSG_START_BLE_APP);
    BLEDevice::init("Wio_Scale_Client");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    // pBLEScan->setInterval(1349);
    // pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(60);
    wio_ble_status_update(MSG_SCANNING);

} // End of setup.

// This is the Arduino main loop function.
void loop(void)
{
    stopwatch_states_t prev_stopwatch_state;
    // If the flag "doConnect" is true then we have scanned for and found the desired
    // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
    // connected we set the connected flag to be true.
    if (doConnect == true)
    {
        connected = connectToServer();
        if (connected)
        {
            serial_println(MSG_CONNECTED_TO_BLE_SERVER);
            doConnect = false;
            doScan = false;
        }
        else
        {
            serial_println(MSG_NOT_CONNECTED_TO_BLE_SERVER);
            delay(1000);
        }
    }
    if (connected == false)
    {
        if (display_cnt % 50 == 0)
        {
            serial_print(".");
        }
    }
    if (connected == false && doScan == true)
    {
        serial_println(MSG_BLE_SCAN_RESTART);
        // BLEDevice::getScan()->start(0);
        pBLEScan->setActiveScan(true);
        pBLEScan->start(30);
        /*
          serial_println("BLE Scan stop");
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
    display_cnt++;
    prev_stopwatch_state = fsm_stopwatch(STOPWATCH_NO_CHANGE);
    if (digitalRead(WIO_KEY_A) == LOW)
    {
        fsm_stopwatch(STOPWATCH_STARTING);
    }
    else if (digitalRead(WIO_KEY_B) == LOW)
    {
        fsm_stopwatch(STOPWATCH_STOPPING);
    }
    else if (digitalRead(WIO_KEY_C) == LOW)
    {
        fsm_stopwatch(STOPWATCH_RESET);
    }
    else
    {
        fsm_stopwatch(prev_stopwatch_state);
    }
    if (do_scale_read)
    {
        do_scale_read = false;
        ble_scale_weight_read();
    }
    if (do_battery_status_update)
    {
        do_battery_status_update = false;
        wio_battery_status_update();
    }
    delay(DELAY_MAIN_LOOP);
}

void scale_read_timer_callback(TimerHandle_t xTimer)
{
    do_scale_read = true;
}

void battery_status_update_callback(TimerHandle_t xTimer)
{
    do_battery_status_update = true;
}
