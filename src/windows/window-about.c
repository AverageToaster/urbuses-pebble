#include "pebble.h"
#include "window-about.h"
#include "libs/scroll-text-layer/scroll-text-layer.h"

static void window_load(Window *window);
static void window_unload(Window *window);
static void layer_header_update(Layer *layer, GContext *ctx);

static Window *window;
static ScrollTextLayer *scroll_layer;
static Layer *layer_header;
static char* about = "UR Buses is a Pebble application developed by TJ Stein.\nIf you want to see what else I'm working on, feel free to check out my github (github.com /tstein4) or follow my twitter account @TheOfficialTJS";

/**
 * Initialization method. Creates window and assigns handlers.
 */
void window_about_init(void){
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers){
		.load = window_load,
		.unload = window_unload
	});
}

/**
 * Deinitialization method. Destroys the window.
 */
void window_about_destroy(void){
	window_destroy(window);
}

/**
 * Shows the window, with animation.
 */
void window_about_show(){
	window_stack_push(window, true);
}

/**
 * Window load method. Creates the various layers inside the window.
 * @param window Window being loaded.
 */
static void window_load(Window *window){
	scroll_layer = scroll_text_layer_create(GRect(0,26,144,130));
	scroll_text_layer_set_font(scroll_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	scroll_text_layer_set_text(scroll_layer, about);
	scroll_text_layer_add_to_window(scroll_layer, window);

	layer_header = layer_create(GRect(0,0,144,26));
	layer_set_update_proc(layer_header, layer_header_update);
	layer_add_child(window_get_root_layer(window), layer_header);
}

/**
 * Window unload method. Destroys the various layers inside the window.
 * @param window Window being destroyed.
 */
static void window_unload(Window *window){
	scroll_text_layer_destroy(scroll_layer);
	layer_destroy(layer_header);
}

/**
 * Function to draw the header of the window.
 * @param layer Layer being updated
 * @param ctx   Graphics context of the given layer.
 */
static void layer_header_update(Layer *layer, GContext *ctx){
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, "UR Buses v2.2", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0,0,144,22), 0, GTextAlignmentCenter, NULL);
}