/*
 * config.h - Cau hinh chan ket noi, nguong dieu khien va thong so he thong
 * Moi gia tri can chinh deu nam o file nay.
 */
#pragma once

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

/* ====================== CHAN KET NOI ====================== */
#define PIN_I2C_SDA       GPIO_NUM_8     // BH1750 va OLED dung chung bus I2C
#define PIN_I2C_SCL       GPIO_NUM_9
#define PIN_DHT22         GPIO_NUM_4     // DATA cua DHT22
#define PIN_RELAY_PUMP    GPIO_NUM_5     // IN1 relay -> may bom 12V
#define PIN_RELAY_FAN     GPIO_NUM_6     // IN2 relay -> quat 5V
#define SOIL_ADC_CHANNEL  ADC_CHANNEL_0  // GPIO 1 = ADC1_CH0 (cam bien do am dat)
#define OLED_I2C_ADDR     0x3C           // mot so module OLED dung 0x3D

/* ====================== RELAY ====================== */
// 1: module kich muc THAP (GPIO = 0 thi relay dong)  <- da so module relay
// 0: module kich muc CAO  (GPIO = 1 thi relay dong)
// Neu relay chay nguoc (bom/quat bat luc dang le phai tat) thi doi gia tri nay.
#define RELAY_ACTIVE_LOW  0

/* ====================== NGUONG DIEU KHIEN ====================== */
#define TEMP_FAN_ON       30.0f    // bat quat khi nhiet do > gia tri nay (do C)
#define TEMP_FAN_OFF      27.0f    // tat quat khi nhiet do < gia tri nay (do C)

#define SOIL_PUMP_ON      40.0f    // bat dau tuoi khi do am dat < gia tri nay (%)
#define SOIL_PUMP_OFF     65.0f    // ngung tuoi khi do am dat >= gia tri nay (%)

#define PUMP_ON_MS        5000U    // moi xung: bom 5 giay
#define PUMP_REST_MS      30000U   // nghi 30 giay de nuoc ngam xuong dat
#define PUMP_MAX_PULSES   6        // toi da so xung trong 1 dot tuoi
#define PUMP_LOCKOUT_MS   600000U  // khoa bom 10 phut neu bom het so xung ma dat van kho
                                   // (het nuoc, ong tac, cam bien hong...)

/* ====================== HIEU CHINH CAM BIEN DO AM DAT ====================== */
// GIA TRI TAM - PHAI DO LAI tren cam bien cua ban (xem huong dan trong README):
#define SOIL_RAW_DRY        3200   // gia tri ADC khi cam bien o ngoai khong khi (kho)
#define SOIL_RAW_WET        1500   // gia tri ADC khi nhung nuoc toi vach gioi han (uot)
#define SOIL_RAW_MIN_VALID  100    // ngoai khoang [MIN, MAX] coi la cam bien loi/roi day
#define SOIL_RAW_MAX_VALID  4050
#define SOIL_SAMPLES        10     // so mau ADC lay trung binh moi lan doc

/* ====================== CHU KY VA BAO VE ====================== */
#define LOOP_PERIOD_MS      100    // chu ky vong lap dieu khien
#define SENSOR_PERIOD_MS    2000U  // chu ky doc cam bien (DHT22 can toi thieu 2 giay)
#define FAULT_FAIL_COUNT    5      // so lan doc loi lien tiep de coi cam bien bi hong

// 1: khi DHT22 hong lien tuc thi bat quat de thong gio (an toan cho cay)
// 0: khi DHT22 hong thi tat quat
#define FAN_ON_SENSOR_FAULT 1
