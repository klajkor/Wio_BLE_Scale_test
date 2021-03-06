#pragma once

#include <stdint.h>
#include <string.h>
#include <string>

#include "Arduino.h"
#include "BLEAddress.h"
#include "BLEAdvertisedDevice.h"

#include "app_messages.h"
#include "counter.h"
#include "display.h"
#include "main_defs.h"
#include "serial_print.h"
#include "weight_queue.h"

#define DECENT_SCALE_PACKET_LEN 7

bool    is_decent_scale_mac_address(bd_addr_t *device_mac_address);
int16_t get_weight_tenthgramm_from_packet(char *p_packet_i);
float   get_weight_gramm_from_packet(char *p_packet_i);
void    set_scale_cmd_string(uint8_t *pCmd, std::string *p_packet_o);
void    decent_write_init(BLERemoteCharacteristic *p_decent_ble_write_characteristic_i);
void    decent_read_init(BLERemoteCharacteristic *p_decent_ble_read_characteristic_i);
void    decent_deinit(void);
void    decent_cmd_led_off(void);
void    decent_cmd_led_on(void);
void    decent_cmd_timer_reset(void);
void    decent_cmd_timer_start(void);
void    decent_cmd_timer_stop(void);
void    ble_scale_weight_read(void);
void    decent_cmd_tare(void);
