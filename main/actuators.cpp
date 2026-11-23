/*
 * actuators.cpp - Dieu khien relay bom va quat
 */
#include "driver/gpio.h"
#include "esp_log.h"
#include "config.h"
#include "actuators.h"

static const char *TAG = "ACTUATORS";

#if RELAY_ACTIVE_LOW
#define RELAY_LEVEL_ON   0
#define RELAY_LEVEL_OFF  1
#else
#define RELAY_LEVEL_ON   1
#define RELAY_LEVEL_OFF  0
#endif

static bool s_pump_on = false;
static bool s_fan_on = false;

static esp_err_t relay_pin_init(gpio_num_t pin)
{
    esp_err_t err = gpio_reset_pin(pin);
    if (err != ESP_OK) {
        return err;
    }
    // Dat muc TAT truoc khi chuyen thanh ngo ra de relay khong "tach" luc khoi dong
    gpio_set_level(pin, RELAY_LEVEL_OFF);
    return gpio_set_direction(pin, GPIO_MODE_OUTPUT);
}

esp_err_t actuators_init(void)
{
    esp_err_t e1 = relay_pin_init(PIN_RELAY_PUMP);
    esp_err_t e2 = relay_pin_init(PIN_RELAY_FAN);
    s_pump_on = false;
    s_fan_on = false;
    if (e1 != ESP_OK || e2 != ESP_OK) {
        ESP_LOGE(TAG, "Loi cau hinh chan relay");
        return ESP_FAIL;
    }
    return ESP_OK;
}

void actuators_set_pump(bool on)
{
    s_pump_on = on;
    gpio_set_level(PIN_RELAY_PUMP, on ? RELAY_LEVEL_ON : RELAY_LEVEL_OFF);
}

void actuators_set_fan(bool on)
{
    s_fan_on = on;
    gpio_set_level(PIN_RELAY_FAN, on ? RELAY_LEVEL_ON : RELAY_LEVEL_OFF);
}

bool actuators_pump_is_on(void) { return s_pump_on; }
bool actuators_fan_is_on(void)  { return s_fan_on; }
