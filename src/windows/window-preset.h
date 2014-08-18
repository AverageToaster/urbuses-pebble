#pragma once

#include "pebble.h"

void window_preset_init(void);
void window_preset_show(void);
void window_preset_destroy(void);
void window_preset_set_preset(Preset* preset, int8_t pos);


void update_time_text();