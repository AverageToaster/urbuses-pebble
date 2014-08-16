#pragma once

#include "pebble.h"

void window_presets_init(void);
void window_presets_show(void);
void window_presets_destroy(void);
void refresh();
void reset_selected_index();
void set_selected_index(int pos);