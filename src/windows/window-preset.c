#include "pebble.h"
#include "../presets.h"
#include "../preset.h"
#include "window-preset.h"
#include "window-presets.h"

static void window_load(Window *window);
static void window_unload(Window *window);
static void click_config_provider(void *ctx);
static void up_click_handler(ClickRecognizerRef recognizer, void* context);
static void down_click_handler(ClickRecognizerRef recognizer, void* context);
static void select_click_handler(ClickRecognizerRef recognizer, void* context);
static void this_tick_callback(struct tm *tick_time, TimeUnits units_changed);
static void update_preset_text();
static void refreshing_text();
static void eta_handler(int pos, int eta);
static void display_eta(int eta);


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
	update_preset_text();
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

	// Set the char* buffer for time.
	// Same deal with time text layer.
	time_layer = text_layer_create(GRect(0,75, bounds.size.w, 48));
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	// Actually set the text in this method for time_layer.
	if (preset->eta > 0){
		char* time_buffer = malloc(10);
		snprintf(time_buffer, 10, " %d ", preset->eta);
		text_layer_set_text(time_layer, time_buffer);
		free(time_buffer);
	}
	else if (preset->eta > -5)
		text_layer_set_text(time_layer, "NOW");
	else
		text_layer_set_text(time_layer, " -- ");
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(time_layer));

	// And create and add the final layer, the "minutes" text under the time.
	minute_text_layer = text_layer_create(GRect(0,123, bounds.size.w, 28));
	if (preset->eta > 0)
		text_layer_set_text(minute_text_layer, (preset->eta != 1 ? "minutes" : "minute"));
	else if (preset->eta == PRESET_REFRESHING_ETA || preset->eta == PRESET_SENT_REQUEST)
		text_layer_set_text(minute_text_layer, "Getting ETA...");
	else if (preset->eta > PRESET_REFRESHING_ETA)
		text_layer_set_text(minute_text_layer, " ");
	else if (preset->eta == PRESET_NO_ETA)
		text_layer_set_text(minute_text_layer, "No Available ETA");
	text_layer_set_font(minute_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(minute_text_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(minute_text_layer));

	// Update the text for Stop and Route.

	update_time_text();
}
static void window_unload(Window *window){
	text_layer_destroy(stop_layer);
	text_layer_destroy(route_layer);
	text_layer_destroy(time_layer);
	text_layer_destroy(minute_text_layer);
	set_selected_index(preset_pos);
}

static void click_config_provider(void* context)
{
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void down_click_handler(ClickRecognizerRef recognizer, void* context)
{
	if (preset_pos == presets_get_count()-1)
		preset_pos = 0;
	else
		preset_pos++;
	preset = presets_get(preset_pos);
	update_time_text();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context)
{
	if (preset_pos == 0)
		preset_pos = presets_get_count() - 1;
	else
		preset_pos--;
	preset = presets_get(preset_pos);
	update_time_text();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context){
	send_eta_req(preset);
	refreshing_text();
}

static void update_preset_text(){
	if (preset != NULL){
		preset = presets_get(preset_pos);
		text_layer_set_text(stop_layer, preset->stop_name);
		text_layer_set_text(route_layer, preset->route_name);
	}
}

static void refreshing_text(){
	text_layer_set_text(time_layer, " -- ");
	text_layer_set_text(minute_text_layer, "Getting ETA...");
}

void update_time_text(){
	if (stop_layer != NULL){
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Test1");
		if (presets_get_count()-1 < preset_pos && window_is_loaded(window)){
			if (presets_get_count() == 0){
				APP_LOG(APP_LOG_LEVEL_DEBUG, "in stack pop method");
				preset_pos = -1;
				window_stack_pop(true);
			}
			else{
				APP_LOG(APP_LOG_LEVEL_DEBUG, "in else");
				preset_pos = 0;
				preset = presets_get(preset_pos);
			}
		}
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Test2");
		update_preset_text();
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Test3");
		if (preset != NULL && preset->eta > 0){
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Test4if");
			char* time_buffer = malloc(10);
			snprintf(time_buffer, 10, " %d ", preset->eta);
			text_layer_set_text(time_layer, time_buffer);
			if (preset->eta == 1)
				text_layer_set_text(minute_text_layer, "minute");
			else
				text_layer_set_text(minute_text_layer, "minutes");
			free (time_buffer);
		}
		else if (preset != NULL && preset->eta > PRESET_REFRESHING_ETA){
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Test4elseif1");
			text_layer_set_text(time_layer, "NOW");
			text_layer_set_text(minute_text_layer, " ");
		}
		else if (preset != NULL && (preset->eta == PRESET_REFRESHING_ETA || preset->eta == PRESET_SENT_REQUEST)){
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Test4elseif2");
			refreshing_text();
		}
		else{
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Test4else");
			if (time_layer != NULL){
				text_layer_set_text(time_layer, " -- ");
				text_layer_set_text(minute_text_layer, "No Available ETA");
			}
		}
		APP_LOG(APP_LOG_LEVEL_DEBUG, "test5");
	}
}