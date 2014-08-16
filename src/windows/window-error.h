#pragma once
#include "pebble.h"

void window_error_init(void);
void window_error_set_text(char* text);
void window_error_show(void);
void window_error_destroy();