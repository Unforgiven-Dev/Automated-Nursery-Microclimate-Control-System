/*
 * HE THONG KIEM SOAT MOI TRUONG VUON UOM
 * Board: ESP32-S3 N16R8 | ESP-IDF v5.4
 *
 * Cam bien : DHT22 (nhiet do, do am KK), BH1750 (anh sang), cam bien do am dat (ADC)
 * Chap hanh: Bom 12V (relay kenh 1), Quat 5V (relay kenh 2)
 * Hien thi : OLED 0.96 inch SSD1306 (I2C) va log Serial
 *
 * Nguyen ly:
 *   - Quat  : bat khi nhiet do > TEMP_FAN_ON, tat khi < TEMP_FAN_OFF (co do tre)
 *   - Bom   : tuoi theo xung (bom 5 giay, nghi 30 giay) khi do am dat < SOIL_PUMP_ON,
 *             ngung khi do am dat >= SOIL_PUMP_OFF
 *   - Bao ve: gioi han so xung, khoa bom khi qua tai, tat bom khi cam bien dat loi
 *
 * Moi nguong va chan ket noi nam trong config.h
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "i2cdev.h"
#include "config.h"
#include "sensors.h"
#include "actuators.h"
#include "control.h"
#include "oled_ssd1306.h"

static const char *TAG = "VUON_UOM";

static oled_t oled;

static inline uint32_t millis(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

// Tao dong chu mo ta trang thai bom
static void pump_text(const pump_ctrl_t *p, bool soil_fault, char *out, size_t n)
{
    if (soil_fault) {
        snprintf(out, n, "Bom: TAT (loi CB)");
        return;
    }
    switch (p->state) {
    case PUMP_ON:
        snprintf(out, n, "Bom: BAT (xung %d/%d)", p->pulses, PUMP_MAX_PULSES);
        break;
    case PUMP_REST:
        snprintf(out, n, "Bom: NGHI (xung %d/%d)", p->pulses, PUMP_MAX_PULSES);
        break;
    case PUMP_LOCKOUT:
        snprintf(out, n, "Bom: KHOA (qua tai)");
        break;
    default:
        snprintf(out, n, "Bom: TAT");
        break;
    }
}

// Ve man hinh OLED (8 dong x 21 ky tu, chu khong dau)
static void show_display(const sensor_data_t *d, const pump_ctrl_t *pump, bool fan_on,
                         bool dht_fault, bool lux_fault, bool soil_fault)
{
    oled_clear(&oled);
    oled_printf(&oled, 0, "VUON UOM THONG MINH");

    // Dong 1: danh sach cam bien dang loi
    if (dht_fault || lux_fault || soil_fault) {
        char f[24] = "";
        if (dht_fault)  strncat(f, " DHT", sizeof(f) - strlen(f) - 1);
        if (lux_fault)  strncat(f, " BH1750", sizeof(f) - strlen(f) - 1);
        if (soil_fault) strncat(f, " DAT", sizeof(f) - strlen(f) - 1);
        oled_printf(&oled, 1, "LOI:%s", f);
    }

    if (d->dht_ok) {
        oled_printf(&oled, 2, "Nhiet do: %.1f C", d->temp);
        oled_printf(&oled, 3, "Do am KK: %.1f %%", d->hum);
    } else {
        oled_printf(&oled, 2, "Nhiet do: --");
        oled_printf(&oled, 3, "Do am KK: --");
    }

    if (d->lux_ok) {
        oled_printf(&oled, 4, "Anh sang: %u lx", (unsigned)d->lux);
    } else {
        oled_printf(&oled, 4, "Anh sang: --");
    }

    if (d->soil_ok) {
        oled_printf(&oled, 5, "Do am dat: %.1f %%", d->soil_pct);
    } else {
        oled_printf(&oled, 5, "Do am dat: --");
    }

    char pt[24];
    pump_text(pump, soil_fault, pt, sizeof(pt));
    oled_printf(&oled, 6, "%s", pt);
    oled_printf(&oled, 7, "Quat: %s", fan_on ? "BAT" : "TAT");

    if (oled_flush(&oled) != ESP_OK) {
        ESP_LOGE(TAG, "Loi ghi ra OLED");
    }
}

static void log_sensors(const sensor_data_t *d)
{
    if (d->dht_ok) {
        ESP_LOGI(TAG, "Nhiet do: %.1f C | Do am KK: %.1f %%", d->temp, d->hum);
    } else {
        ESP_LOGW(TAG, "DHT22: loi doc");
    }
    if (d->lux_ok) {
        ESP_LOGI(TAG, "Anh sang: %u lx", (unsigned)d->lux);
    } else {
        ESP_LOGW(TAG, "BH1750: loi doc");
    }
    if (d->soil_ok) {
        ESP_LOGI(TAG, "Do am dat: %.1f %% (ADC raw = %d)", d->soil_pct, d->soil_raw);
    } else {
        ESP_LOGW(TAG, "Do am dat: loi doc (ADC raw = %d)", d->soil_raw);
    }
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Khoi dong he thong kiem soat moi truong vuon uom");

    // 1. Relay: dat TAT truoc tien de bom/quat khong chay ngoai y muon luc khoi dong
    if (actuators_init() != ESP_OK) {
        ESP_LOGE(TAG, "Loi khoi tao relay");
    }

    // 2. Bus I2C dung chung cho BH1750 va OLED
    ESP_ERROR_CHECK(i2cdev_init());

    // 3. OLED (loi thi chi mat hien thi, he thong van chay)
    bool oled_ok = (oled_init(&oled, OLED_I2C_ADDR, I2C_NUM_0, PIN_I2C_SDA, PIN_I2C_SCL) == ESP_OK);
    if (oled_ok) {
        oled_clear(&oled);
        oled_printf(&oled, 3, "   KHOI DONG...");
        oled_flush(&oled);
    } else {
        ESP_LOGE(TAG, "Khong khoi tao duoc OLED, kiem tra day va dia chi 0x%02X (thu 0x3D)", OLED_I2C_ADDR);
    }

    // 4. Cam bien
    sensors_init();

    // 5. Bo dieu khien
    pump_ctrl_t pump;
    pump_ctrl_init(&pump);
    bool fan_on = false;

    sensor_data_t data;
    memset(&data, 0, sizeof(data));
    int dht_fail = 0, lux_fail = 0, soil_fail = 0;
    uint32_t last_read = 0;
    bool first = true;

    pump_state_t last_pump_state = PUMP_IDLE;
    int last_pulses = 0;

    while (1) {
        uint32_t now = millis();
        bool refresh = false;

        // ---- Doc cam bien theo chu ky ----
        if (first || (now - last_read) >= SENSOR_PERIOD_MS) {
            first = false;
            last_read = now;
            sensors_read(&data);

            dht_fail  = data.dht_ok  ? 0 : (dht_fail  < 1000 ? dht_fail  + 1 : dht_fail);
            lux_fail  = data.lux_ok  ? 0 : (lux_fail  < 1000 ? lux_fail  + 1 : lux_fail);
            soil_fail = data.soil_ok ? 0 : (soil_fail < 1000 ? soil_fail + 1 : soil_fail);

            log_sensors(&data);
            refresh = true;
        }

        bool dht_fault  = (dht_fail  >= FAULT_FAIL_COUNT);
        bool lux_fault  = (lux_fail  >= FAULT_FAIL_COUNT);
        bool soil_fault = (soil_fail >= FAULT_FAIL_COUNT);

        // ---- Dieu khien ----
        bool pump_cmd = pump_ctrl_update(&pump, data.soil_ok, soil_fault, data.soil_pct, now);
        bool fan_cmd  = fan_ctrl_update(fan_on, data.dht_ok, data.temp, dht_fault);

        if (pump_cmd != actuators_pump_is_on()) {
            actuators_set_pump(pump_cmd);
            ESP_LOGI(TAG, ">>> BOM %s", pump_cmd ? "BAT" : "TAT");
            refresh = true;
        }
        if (fan_cmd != fan_on) {
            fan_on = fan_cmd;
            actuators_set_fan(fan_on);
            ESP_LOGI(TAG, ">>> QUAT %s", fan_on ? "BAT" : "TAT");
            refresh = true;
        }
        if (pump.state != last_pump_state || pump.pulses != last_pulses) {
            if (pump.state == PUMP_LOCKOUT && last_pump_state != PUMP_LOCKOUT) {
                ESP_LOGW(TAG, "Bom bi KHOA %u giay: da bom %d xung ma dat van kho (het nuoc? cam bien hong?)",
                         (unsigned)(PUMP_LOCKOUT_MS / 1000), PUMP_MAX_PULSES);
            }
            last_pump_state = pump.state;
            last_pulses = pump.pulses;
            refresh = true;
        }

        // ---- Cap nhat man hinh khi co thay doi ----
        if (refresh && oled_ok) {
            show_display(&data, &pump, fan_on, dht_fault, lux_fault, soil_fault);
        }

        vTaskDelay(pdMS_TO_TICKS(LOOP_PERIOD_MS));
    }
}
