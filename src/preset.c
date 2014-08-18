#include <pebble.h>
#include "preset.h"

/**
 * function to destroy a Preset.
 * @param preset Preset to free and destroy.
 */
void preset_destroy(Preset *preset){
	if (!preset) return;
	free(preset);
}

/**
 * Function to clone a preset.
 * @param  preset Preset to clone
 * @return        Cloned preset.
 */
Preset* preset_clone(Preset *preset){
	Preset* clone = malloc(sizeof(Preset));
	memcpy(clone->stop_id, preset->stop_id, 32);
	memcpy(clone->stop_name, preset->stop_name, 32);
	memcpy(clone->route_id, preset->route_id, 32);
	memcpy(clone->route_name, preset->route_name, 32);
	clone->eta = preset->eta;
	return clone;
}