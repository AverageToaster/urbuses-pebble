#include "pebble.h"
#include "window-no-presets.h"
#include "libs/scroll-text-layer/scroll-text-layer.h"

static void window_load(Window *window);
static void window_unload(Window *window);
static void layer_header_update(Layer *layer, GContext *ctx);

static Window *window;
static ScrollTextLayer *scroll_layer;
static Layer *layer_header;
static char* message = "You are seeing this because you haven't set up any presets yet. :(\n\nPlease visit the UR Buses Settings page in your Pebble app locker to set presets.";

void window_no_presets_init(void){
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers){
		.load = window_load,
		.unload = window_unload
	});
}

void window_no_presets_destroy(void){
	window_destroy(window);
}

void window_no_presets_show(){
	window_stack_push(window, true);
}

static void window_load(Window *window){
	scroll_layer = scroll_text_layer_create(GRect(0,26,144,130));
	scroll_text_layer_set_font(scroll_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	scroll_text_layer_set_text(scroll_layer, message);
	scroll_text_layer_add_to_window(scroll_layer, window);

	layer_header = layer_create(GRect(0,0,144,26));
	layer_set_update_proc(layer_header, layer_header_update);
	layer_add_child(window_get_root_layer(window), layer_header);
}

static void window_unload(Window *window){
	scroll_text_layer_destroy(scroll_layer);
	layer_destroy(layer_header);
}

static void layer_header_update(Layer *layer, GContext *ctx){
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, "No Presets Set", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0,0,144,22), 0, GTextAlignmentCenter, NULL);
}