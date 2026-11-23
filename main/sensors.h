/*
 * sensors.h - Doc 3 cam bien: DHT22, BH1750, do am dat
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

typedef struct {
    bool     dht_ok;      // true neu lan doc DHT22 gan nhat hop le
    float    temp;        // nhiet do (do C)
    float    hum;         // do am khong khi (%)

    bool     lux_ok;      // true neu lan doc BH1750 gan nhat hop le
    uint16_t lux;         // cuong do anh sang (lux)

    bool     soil_ok;     // true neu lan doc do am dat gan nhat hop le
    int      soil_raw;    // gia tri ADC tho (dung de hieu chinh)
    float    soil_pct;    // do am dat (%)
} sensor_data_t;

// Khoi tao BH1750 va ADC. Phai goi i2cdev_init() truoc.
// Loi cua tung cam bien chi ghi log, ham van tra ve ESP_OK de he thong tiep tuc chay.
esp_err_t sensors_init(void);

// Doc tat ca cam bien. Gia tri chi duoc cap nhat khi lan doc hop le.
void sensors_read(sensor_data_t *d);
