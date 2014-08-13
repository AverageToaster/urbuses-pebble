#include <pebble.h>

typedef struct Preset{
	char stop_id[32];
	char stop_name[32];
	char route_id[32];
	char route_name[32];
} __attribute__((__packed__)) Preset;