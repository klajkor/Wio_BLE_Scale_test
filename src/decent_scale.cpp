#include "decent_scale.h"

#define ENABLE_DOUBLE_COMMAND_SENDING (1)

#define STANDARD_TASK_STACK_SIZE ((uint32_t)(2 * 1024)) // 2k
#define PRIO_SCALE_READ_TASK ((UBaseType_t)7)

static const uint32_t DELAY_SCALE_READ_TASK = ((TickType_t)((125) / portTICK_PERIOD_MS));

typedef struct
{
    uint8_t pos;
    uint8_t value;
} mac_addr_check_st;

#if defined(__SAMD51__)
// NOTE! Reverse byte order on SAMD51!
static const mac_addr_check_st DECENT_MAC_ADDR[] = {
    {.pos = 3, .value = 0x10}, {.pos = 4, .value = 0xFF}, {.pos = 5, .value = 0xFF}};
#else
static const mac_addr_check_st DECENT_MAC_ADDR[] = {
    {.pos = 2, .value = 0x10}, {.pos = 1, .value = 0xFF}, {.pos = 0, .value = 0xFF}};
#endif

static uint8_t CMD_LED_ON[DECENT_SCALE_PACKET_LEN] = {0x03, 0x0A, 0x01, 0x01, 0x00, 0x00, 0x09};
static uint8_t CMD_LED_OFF[DECENT_SCALE_PACKET_LEN] = {0x03, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x09};
static uint8_t CMD_TIMER_STOP[DECENT_SCALE_PACKET_LEN] = {0x03, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x08};
static uint8_t CMD_TIMER_START[DECENT_SCALE_PACKET_LEN] = {0x03, 0x0B, 0x03, 0x00, 0x00, 0x00, 0x0B};
static uint8_t CMD_TIMER_RESET[DECENT_SCALE_PACKET_LEN] = {0x03, 0x0B, 0x02, 0x00, 0x00, 0x00, 0x0A};

static BLERemoteCharacteristic *p_decent_ble_write_characteristic = nullptr;
static BLERemoteCharacteristic *p_decent_ble_read_characteristic = nullptr;

TaskHandle_t xHandle_ble_scale_weight_read = NULL;

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

void decent_write_init(BLERemoteCharacteristic *p_decent_ble_write_characteristic_i)
{
    p_decent_ble_write_characteristic = p_decent_ble_write_characteristic_i;
}

void decent_read_init(BLERemoteCharacteristic *p_decent_ble_read_characteristic_i)
{
    p_decent_ble_read_characteristic = p_decent_ble_read_characteristic_i;
}

void decent_deinit(void)
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

void create_scale_read_task(void)
{
    BaseType_t xReturned;

    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(xTask_ble_scale_weight_read,     /* Function that implements the task. */
                            "ble_scale_weight_read",         /* Text name for the task. */
                            STANDARD_TASK_STACK_SIZE,        /* Stack size in words, not bytes. */
                            (void *)1,                       /* Parameter passed into the task. */
                            PRIO_SCALE_READ_TASK,            /* Priority at which the task is created. */
                            &xHandle_ble_scale_weight_read); /* Used to pass out the created task's handle. */
    if (xReturned != pdPASS)
    {
        Serial.println(F("FAILED to create scale weight read task"));
    }
}

void xTask_ble_scale_weight_read(void *pvParameters)
{
    std::string read_value = "123456789012345678901";
    float       scale_weight;

    while (1)
    {
        if (p_decent_ble_read_characteristic != nullptr)
        {
            read_value = p_decent_ble_read_characteristic->readValue();
            if (read_value.at(0) == 3)
            {
                scale_weight = get_weight_gramm_from_packet((char *)read_value.c_str());
                weight_q_push(&scale_weight_q, scale_weight);
                // StateMachine_counter1(scale_weight_q.total_diff, MIN_WEIGHT_INC);
                wio_weight_display_update(scale_weight);
            }
        }
        vTaskDelay(DELAY_SCALE_READ_TASK);
    }
    vTaskDelete(xHandle_ble_scale_weight_read);
}