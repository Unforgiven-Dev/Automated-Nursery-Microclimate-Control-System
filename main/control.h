/*
 * control.h - Logic dieu khien bom va quat 
 *
 * Quat: bat/tat co do tre theo nhiet do.
 * Bom : tuoi theo xung (bom - nghi - doc lai) bang may trang thai khong chan vong lap,
 *       co gioi han so xung va khoa tam thoi de bao ve khi het nuoc / cam bien hong.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "config.h"

/* ---------------------------- QUAT ---------------------------- */
// fan_on     : trang thai quat hien tai
// temp_valid : lan doc gan nhat hop le
// temp_fault : DHT22 loi lien tiep nhieu lan
// Tra ve true neu quat can bat.
static inline bool fan_ctrl_update(bool fan_on, bool temp_valid, float temp, bool temp_fault)
{
    if (temp_fault) {
        return FAN_ON_SENSOR_FAULT ? true : false;
    }
    if (!temp_valid) {
        return fan_on;               // loi thoang qua: giu nguyen trang thai
    }
    if (temp > TEMP_FAN_ON) {
        return true;
    }
    if (temp < TEMP_FAN_OFF) {
        return false;
    }
    return fan_on;                   // trong vung tre: giu nguyen
}

/* ---------------------------- BOM ---------------------------- */
typedef enum {
    PUMP_IDLE = 0,    // khong tuoi
    PUMP_ON,          // dang bom 1 xung
    PUMP_REST,        // nghi giua 2 xung, cho nuoc ngam
    PUMP_LOCKOUT      // bi khoa tam thoi do bom qua nhieu xung ma dat van kho
} pump_state_t;

typedef struct {
    pump_state_t state;
    uint32_t     t_start;   // thoi diem (ms) bat dau trang thai hien tai
    int          pulses;    // so xung da bom trong dot tuoi nay
} pump_ctrl_t;

static inline void pump_ctrl_reset(pump_ctrl_t *c)
{
    c->state = PUMP_IDLE;
    c->pulses = 0;
}

static inline void pump_ctrl_init(pump_ctrl_t *c)
{
    c->t_start = 0;
    pump_ctrl_reset(c);
}

// soil_valid: lan doc do am dat gan nhat hop le
// soil_fault: cam bien do am dat loi lien tiep nhieu lan (bom phai tat)
// soil      : do am dat (%)
// now       : thoi gian hien tai (ms)
// Tra ve true neu bom can bat.
static inline bool pump_ctrl_update(pump_ctrl_t *c, bool soil_valid, bool soil_fault,
                                    float soil, uint32_t now)
{
    if (soil_fault) {
        pump_ctrl_reset(c);          // khong co du lieu tin cay thi khong tuoi
        return false;
    }

    uint32_t elapsed = now - c->t_start;

    switch (c->state) {
    case PUMP_IDLE:
        if (soil_valid && soil < SOIL_PUMP_ON) {
            c->state = PUMP_ON;
            c->t_start = now;
            c->pulses = 1;
            return true;
        }
        return false;

    case PUMP_ON:
        if (soil_valid && soil >= SOIL_PUMP_OFF) {   // da du am giua chung: dung ngay
            pump_ctrl_reset(c);
            return false;
        }
        if (elapsed >= PUMP_ON_MS) {
            c->state = PUMP_REST;
            c->t_start = now;
            return false;
        }
        return true;

    case PUMP_REST:
        if (soil_valid && soil >= SOIL_PUMP_OFF) {   // da du am: ket thuc dot tuoi
            pump_ctrl_reset(c);
            return false;
        }
        if (elapsed >= PUMP_REST_MS) {
            if (!soil_valid) {
                return false;                        // cho co du lieu hop le roi moi quyet dinh
            }
            if (c->pulses >= PUMP_MAX_PULSES) {
                c->state = PUMP_LOCKOUT;
                c->t_start = now;
                return false;
            }
            c->pulses++;
            c->state = PUMP_ON;
            c->t_start = now;
            return true;
        }
        return false;

    case PUMP_LOCKOUT:
        if (elapsed >= PUMP_LOCKOUT_MS) {
            pump_ctrl_reset(c);
        }
        return false;
    }
    return false;
}
