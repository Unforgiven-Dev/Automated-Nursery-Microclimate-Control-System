/*
 * actuators.h - Dieu khien relay: bom (kenh 1) va quat (kenh 2)
 */
#pragma once

#include <stdbool.h>
#include "esp_err.h"

// Dat ca hai relay o trang thai TAT roi moi cau hinh chan thanh ngo ra.
esp_err_t actuators_init(void);

void actuators_set_pump(bool on);
void actuators_set_fan(bool on);
bool actuators_pump_is_on(void);
bool actuators_fan_is_on(void);
