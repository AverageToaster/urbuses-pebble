#include "pebble.h"
#include "../presets.h"
#include "../preset.h"

static void window_load(Window *window);
static void window_unload(Window *window);
static void click_config_provider(void *ctx);

static Window *window;
static TextLayer *stop_layer;
static TextLayer *route_layer;
static TextLayer *time_layer;
static TextLayer *minute_text_layer;
static Preset *preset;
static uint8_t preset_pos = -1;


void window_preset_init(void){
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers){
		.load = window_load,
		.unload = window_unload
	});
	window_set_click_config_provider(window, click_config_provider);
}

void window_preset_show(void){
	window_stack_push(window, true);
}

void window_preset_destroy(void){
	window_destroy(window);
}

void window_preset_set_preset(Preset* preset_arg, uint8_t pos){
	preset = preset_arg;
	preset_pos = pos;
}

/*
* Function called when the app loads. 
* Creates the text layers and assigns them their default values.
*/
static void window_load(Window *window)
{
	// Get window layer for adding the text layers and bounds for sizing.
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);

	// Create the layer by giving it a GRect for size.
	stop_layer = text_layer_create(GRect(0, -5, bounds.size.w /* width */, 52 /* height */));
	// Set the font. Went with 24 Bold so its readable, yet small enough to fit the whole stop.
	text_layer_set_font(stop_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	// Align the text. Went with center for looks.
	text_layer_set_text_alignment(stop_layer, GTextAlignmentCenter);
	// Finally add the text layer to the screen.
	layer_add_child(window_layer, text_layer_get_layer(stop_layer));
	
	// Same with route.
	route_layer = text_layer_create(GRect(0,47, bounds.size.w, 28));
	text_layer_set_font(route_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(route_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(route_layer));

	char* time_buffer = malloc(20);
	// Set the char* buffer for time.
	snprintf(time_buffer, sizeof(" XX "), " -- ");
	// Same deal with time text layer.
	time_layer = text_layer_create(GRect(0,75, bounds.size.w, 48));
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	// Actually set the text in this method for time_layer.
	text_layer_set_text(time_layer, (char*) &time_buffer);
	free(time_buffer);

	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(time_layer));

	char* minute_text_buffer = malloc(32);
	// And create and add the final layer, the "minutes" text under the time.
	snprintf(minute_text_buffer, 32, "minutes");
	minute_text_layer = text_layer_create(GRect(0,123, bounds.size.w, 28));
	text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);
	text_layer_set_font(minute_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(minute_text_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(minute_text_layer));

	free(minute_text_buffer);
	// Update the text for Stop and Route.

}
static void window_unload(Window *window){
	text_layer_destroy(stop_layer);
	text_layer_destroy(route_layer);
	text_layer_destroy(time_layer);
	text_layer_destroy(minute_text_layer);
}

static void click_config_provider(void* context)
{
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
} 

static void up_click_handler(ClickRecognizerRef recognizer, void* context)
{
	if (preset_pos == presets_get_count()-1)
		preset_pos = 0;
	else
		preset_pos++;
	preset = presets_get(preset_pos);
	update_preset_text();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context)
{
	if (preset_pos == 0)
		preset_pos = presets_get_count() -1;
	else
		preset_pos--;
	preset = presets_get(preset_pos);
	update_preset_text();
}

static void update_preset_text(){
	char* route_buffer = malloc(32);
	char* stop_buffer = malloc(32);
	strcpy(route_buffer, preset->route_name);
	strcpy(stop_buffer, preset->stop_name);
	text_layer_set_text(stop_layer, (char*) &stop_buffer);
	text_layer_set_text(route_layer, (char*) &route_buffer);
	free(route_buffer);
	free(stop_buffer);
}

static void refreshing_text(){
	text_layer_set_text(time_layer, " -- ");
	text_layer_set_text(minute_text_layer, "refreshing...");
}

static void send_eta_req(){
	
}

static void eta_handler(int pos, int eta){
	if (pos == preset_pos)
		display_eta(eta);
}

static void display_eta(int eta){
	// TODO: something
}