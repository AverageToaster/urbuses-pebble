#pragma once

#include "pebble.h"

void window_preset_init(void);
void window_preset_show(void);
void window_preset_destroy(void);
void window_preset_set_preset(Preset *preset, uint8_t pos);