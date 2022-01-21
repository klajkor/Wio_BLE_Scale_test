#include "decent_scale.h"

#define ENABLE_DOUBLE_COMMAND_SENDING (1)

typedef struct
{
    uint8_t pos;
    uint8_t value;
} mac_addr_check_st;

// NOTE! Reverse byte order on SAMD51!
static const mac_addr_check_st DECENT_MAC_ADDR[] = {
    {.pos = 3, .value = 0x10}, {.pos = 4, .value = 0xFF}, {.pos = 5, .value = 0xFF}};

uint8_t CMD_LED_ON[DECENT_SCALE_PACKET_LEN] = {0x03, 0x0A, 0x01, 0x01, 0x00, 0x00, 0x09};
uint8_t CMD_LED_OFF[DECENT_SCALE_PACKET_LEN] = {0x03, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x09};
uint8_t CMD_TIMER_STOP[DECENT_SCALE_PACKET_LEN] = {0x03, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x08};
uint8_t CMD_TIMER_START[DECENT_SCALE_PACKET_LEN] = {0x03, 0x0B, 0x03, 0x00, 0x00, 0x00, 0x0B};
uint8_t CMD_TIMER_RESET[DECENT_SCALE_PACKET_LEN] = {0x03, 0x0B, 0x02, 0x00, 0x00, 0x00, 0x0A};

static BLERemoteCharacteristic *p_decent_ble_write_characteristic = nullptr;

bool is_decent_scale_mac_address(bd_addr_t *device_mac_address)
{
    uint32_t  ix;
    uint32_t  pos;
    uint32_t  match = 0;
    uint32_t  length;
    bd_addr_t priv_mac;
    length = sizeof(DECENT_MAC_ADDR) / sizeof(mac_addr_check_st);
    memcpy(priv_mac, device_mac_address, 6);
    bool ret_val = false;
    for (ix = 0; ix < length; ix++)
    {
        pos = DECENT_MAC_ADDR[ix].pos;
        if (priv_mac[pos] == DECENT_MAC_ADDR[ix].value)
        {
            match++;
        }
    }
    if (match == length)
    {
        ret_val = true;
    }
    return ret_val;
}

int16_t get_weight_tenthgramm_from_packet(char *p_packet_i)
{
    int8_t  highByte;
    int8_t  lowByte;
    int16_t ret_weight;
    highByte = (int8_t)p_packet_i[2];
    lowByte = (int8_t)p_packet_i[3];
    ret_weight = (int16_t)(((highByte & 0xFF) << 8) | (lowByte & 0xFF));
    return ret_weight;
}

float get_weight_gramm_from_packet(char *p_packet_i)
{
    int16_t tenthgramm;
    tenthgramm = get_weight_tenthgramm_from_packet(p_packet_i);
    return (float)(tenthgramm / 10.0);
}

void set_scale_cmd_string(uint8_t *pCmd, std::string *p_packet_o)
{
    uint8_t i;
    for (i = 0; i < DECENT_SCALE_PACKET_LEN; i++)
    {
        p_packet_o->at(i) = pCmd[i];
    }
}

void decent_cmd_init(BLERemoteCharacteristic *p_decent_ble_write_characteristic_i)
{
    p_decent_ble_write_characteristic = p_decent_ble_write_characteristic_i;
}

void decent_cmd_deinit(void)
{
    p_decent_ble_write_characteristic = nullptr;
}

void decent_cmd_led_off(void)
{
    uint32_t ix;
    uint32_t cycles = (1 << (ENABLE_DOUBLE_COMMAND_SENDING & 0x01));
    if (p_decent_ble_write_characteristic == nullptr)
    {
        return;
    }
    Serial.println(F("Sending Led OFF command to scale "));
    delay(50);
    for (ix = 0; ix < cycles; ix++)
    {
        p_decent_ble_write_characteristic->writeValue(CMD_LED_OFF, DECENT_SCALE_PACKET_LEN, false);
        delay(60);
    }
}

void decent_cmd_led_on(void)
{
    uint32_t ix;
    uint32_t cycles = (1 << (ENABLE_DOUBLE_COMMAND_SENDING & 0x01));
    if (p_decent_ble_write_characteristic == nullptr)
    {
        return;
    }
    Serial.println(F("Sending Led ON command to scale "));
    delay(50);
    for (ix = 0; ix < cycles; ix++)
    {
        p_decent_ble_write_characteristic->writeValue(CMD_LED_ON, DECENT_SCALE_PACKET_LEN, false);
        delay(60);
    }
}

void decent_cmd_timer_reset(void)
{
    uint32_t ix;
    uint32_t cycles = (1 << (ENABLE_DOUBLE_COMMAND_SENDING & 0x01));
    if (p_decent_ble_write_characteristic == nullptr)
    {
        return;
    }
    Serial.println(F("Sending Timer Reset command to scale "));
    delay(50);
    for (ix = 0; ix < cycles; ix++)
    {
        p_decent_ble_write_characteristic->writeValue(CMD_TIMER_RESET, DECENT_SCALE_PACKET_LEN, false);
        delay(60);
    }
}

void decent_cmd_timer_start(void)
{
    uint32_t ix;
    uint32_t cycles = (1 << (ENABLE_DOUBLE_COMMAND_SENDING & 0x01));
    if (p_decent_ble_write_characteristic == nullptr)
    {
        return;
    }
    Serial.println(F("Sending Timer Start command to scale "));
    delay(50);
    for (ix = 0; ix < cycles; ix++)
    {
        p_decent_ble_write_characteristic->writeValue(CMD_TIMER_START, DECENT_SCALE_PACKET_LEN, false);
        delay(60);
    }
}

void decent_cmd_timer_stop(void)
{
    uint32_t ix;
    uint32_t cycles = (1 << (ENABLE_DOUBLE_COMMAND_SENDING & 0x01));
    if (p_decent_ble_write_characteristic == nullptr)
    {
        return;
    }
    Serial.println(F("Sending Timer Stop command to scale "));
    delay(50);
    for (ix = 0; ix < cycles; ix++)
    {
        p_decent_ble_write_characteristic->writeValue(CMD_TIMER_STOP, DECENT_SCALE_PACKET_LEN, false);
        delay(60);
    }
}