/*
 * sensors.cpp - Doc cam bien DHT22 (1 day), BH1750 (I2C), do am dat (ADC)
 */
#include <string.h>
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "i2cdev.h"
#include "bh1750.h"
#include "dht.h"
#include "config.h"
#include "sensors.h"

static const char *TAG = "SENSORS";

static adc_oneshot_unit_handle_t s_adc;
static bool s_adc_ok = false;

static i2c_dev_t s_bh;
static bool s_bh_desc_ok = false;   // da tao mo ta thiet bi I2C
static bool s_bh_ready = false;     // da cau hinh che do do (tu cau hinh lai neu mat ket noi)

esp_err_t sensors_init(void)
{
    // ---- BH1750 ----
    memset(&s_bh, 0, sizeof(s_bh));
    esp_err_t err = bh1750_init_desc(&s_bh, BH1750_ADDR_LO, I2C_NUM_0, PIN_I2C_SDA, PIN_I2C_SCL);
    if (err == ESP_OK) {
        s_bh_desc_ok = true;
        s_bh_ready = (bh1750_setup(&s_bh, BH1750_MODE_CONTINUOUS, BH1750_RES_HIGH) == ESP_OK);
        if (!s_bh_ready) {
            ESP_LOGW(TAG, "BH1750 chua phan hoi, kiem tra day SDA/SCL/ADDR (se thu lai)");
        }
    } else {
        ESP_LOGE(TAG, "Khong tao duoc mo ta BH1750");
    }

    // ---- ADC cho cam bien do am dat ----
    adc_oneshot_unit_init_cfg_t unit_cfg = {};
    unit_cfg.unit_id = ADC_UNIT_1;
    err = adc_oneshot_new_unit(&unit_cfg, &s_adc);
    if (err == ESP_OK) {
        adc_oneshot_chan_cfg_t chan_cfg = {};
        chan_cfg.atten = ADC_ATTEN_DB_12;           // do duoc toi khoang 3.1V
        chan_cfg.bitwidth = ADC_BITWIDTH_DEFAULT;   // 12 bit (0..4095)
        err = adc_oneshot_config_channel(s_adc, SOIL_ADC_CHANNEL, &chan_cfg);
    }
    s_adc_ok = (err == ESP_OK);
    if (!s_adc_ok) {
        ESP_LOGE(TAG, "Khong khoi tao duoc ADC cho cam bien do am dat");
    }

    return ESP_OK;
}

// Doc trung binh SOIL_SAMPLES mau de giam nhieu. Tra ve true neu gia tri hop le.
static bool soil_read(int *raw, float *pct)
{
    if (!s_adc_ok) {
        return false;
    }

    long sum = 0;
    for (int i = 0; i < SOIL_SAMPLES; i++) {
        int v = 0;
        if (adc_oneshot_read(s_adc, SOIL_ADC_CHANNEL, &v) != ESP_OK) {
            return false;
        }
        sum += v;
    }
    int avg = (int)(sum / SOIL_SAMPLES);
    *raw = avg;

    // Ngoai khoang hop le -> nhieu kha nang cam bien roi day hoac chap nguon
    if (avg < SOIL_RAW_MIN_VALID || avg > SOIL_RAW_MAX_VALID) {
        return false;
    }

    float p = (float)(SOIL_RAW_DRY - avg) * 100.0f / (float)(SOIL_RAW_DRY - SOIL_RAW_WET);
    if (p < 0.0f) {
        p = 0.0f;
    }
    if (p > 100.0f) {
        p = 100.0f;
    }
    *pct = p;
    return true;
}

void sensors_read(sensor_data_t *d)
{
    // ---- 1. Anh sang (BH1750) ----
    d->lux_ok = false;
    if (s_bh_desc_ok) {
        if (!s_bh_ready) {
            s_bh_ready = (bh1750_setup(&s_bh, BH1750_MODE_CONTINUOUS, BH1750_RES_HIGH) == ESP_OK);
        }
        if (s_bh_ready) {
            uint16_t lux = 0;
            if (bh1750_read(&s_bh, &lux) == ESP_OK) {
                d->lux = lux;
                d->lux_ok = true;
            } else {
                s_bh_ready = false;   // mat ket noi: lan sau cau hinh lai
            }
        }
    }

    // ---- 2. Nhiet do va do am khong khi (DHT22 co kieu AM2301 trong thu vien) ----
    d->dht_ok = false;
    float t = 0, h = 0;
    if (dht_read_float_data(DHT_TYPE_AM2301, PIN_DHT22, &h, &t) == ESP_OK &&
        t >= -40.0f && t <= 80.0f && h >= 0.0f && h <= 100.0f) {
        d->temp = t;
        d->hum = h;
        d->dht_ok = true;
    }

    // ---- 3. Do am dat ----
    d->soil_ok = false;
    int raw = 0;
    float pct = 0;
    bool ok = soil_read(&raw, &pct);
    d->soil_raw = raw;
    if (ok) {
        d->soil_pct = pct;
        d->soil_ok = true;
    }
}
