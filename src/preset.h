#include <pebble.h>

#pragma once

typedef struct Preset{
	char stop_id[32];
	char stop_name[32];
	char route_id[32];
	char route_name[32];
	int eta;
} __attribute__((__packed__)) Preset;

void preset_destroy(Preset* preset);
Preset* preset_clone(Preset* preset);