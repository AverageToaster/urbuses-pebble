#include <pebble.h>
#include "preset.h"

void preset_destroy(Preset *preset){
	if (!preset) return;
	free(preset);
}

Preset* preset_clone(Preset *preset){
	Preset* clone = malloc(sizeof(Preset));
	memcpy(clone->stop_id, preset->stop_id, 32);
	memcpy(clone->stop_name, preset->stop_name, 32);
	memcpy(clone->route_id, preset->route_id, 32);
	memcpy(clone->route_name, preset->route_name, 32);
	return clone;
}