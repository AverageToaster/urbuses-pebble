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
static int8_t preset_pos = -1;

/**
 * Initialization method. Creates window and assigns handlers.
 */
void window_preset_init(void){
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers){
		.load = window_load,
		.unload = window_unload
	});
	window_set_click_config_provider(window, click_config_provider);
}

/**
 * Shows the window, with animation.
 */
void window_preset_show(void){
	window_stack_push(window, true);
	update_time_text();
}

/**
 * Deinitialization method. Destroys the window.
 */
void window_preset_destroy(void){
	window_destroy(window);
}

/**
 * Function to assign the window's preset, which is the basis for all info shown on this window.
 * @param preset_arg Preset being assigned
 * @param pos        Index of preset_arg in the Presets list.
 */
void window_preset_set_preset(Preset* preset_arg, int8_t pos){
	preset = preset_arg;
	preset_pos = pos;
}

/**
 * Removes the preset from the window, and sets the position back to -1
 */
void window_preset_clear_preset(){
	preset = NULL;
	preset_pos = -1;
}

/**
 * Window load method. Creates the various layers inside the window.
 * @param window Window being loaded.
 */
static void window_load(Window *window)
{
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);

	stop_layer = text_layer_create(GRect(0, -5, bounds.size.w /* width */, 52 /* height */));
	text_layer_set_font(stop_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(stop_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(stop_layer));

	route_layer = text_layer_create(GRect(0,47, bounds.size.w, 28));
	text_layer_set_font(route_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(route_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(route_layer));

	time_layer = text_layer_create(GRect(0,75, bounds.size.w, 48));
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	if (preset->eta > 0){
		char* time_buffer = malloc(10);
		snprintf(time_buffer, 10, " %d ", preset->eta);
		text_layer_set_text(time_layer, time_buffer);
		free(time_buffer);
	}
	else if (preset->eta > PRESET_REFRESHING_ETA)
		text_layer_set_text(time_layer, "NOW");
	else
		text_layer_set_text(time_layer, " -- ");
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(time_layer));

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

	update_time_text();
}

/**
 * Window unload method. Destroys the various layers inside the window.
 * @param window Window being destroyed.
 */
static void window_unload(Window *window){
	text_layer_destroy(stop_layer);
	text_layer_destroy(route_layer);
	text_layer_destroy(time_layer);
	text_layer_destroy(minute_text_layer);
	set_selected_index(preset_pos);
}
/**
 * Function to set how the window handles various button clicks.
 * @param context pointer to application specific data, not used in this application.
 */
static void click_config_provider(void* context)
{
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

/**
 * Function that handles when the DOWN button is clicked. In this window, that means
 * that the user is cycling down the list of presets, and the preset list position and preset
 * are updated to represent that.
 * @param recognizer Used to recognize that DOWN has been pressed.
 * @param context    Application specific context, not used in this app.
 */
static void down_click_handler(ClickRecognizerRef recognizer, void* context)
{
	if (preset_pos == presets_get_count()-1)
		preset_pos = 0;
	else
		preset_pos++;
	preset = presets_get(preset_pos);
	update_time_text();
}

/**
 * Function that handles when the UP button is clicked. In this window, that means
 * that the user is cycling up through the list of presets, and the preset list position and preset
 * are updated to represent that.
 * @param recognizer Used to recognize that UP has been pressed.
 * @param context    Application specific context, not used in this app.
 */
static void up_click_handler(ClickRecognizerRef recognizer, void *context)
{
	if (preset_pos <= 0)
		preset_pos = presets_get_count() - 1;
	else
		preset_pos--;
	preset = presets_get(preset_pos);
	update_time_text();
}

/**
 * Function that handles when the SELECT button is clicked. In this window, that means
 * that the user requesting an update of the preset.
 * @param recognizer Used to recognize that SELECT has been pressed.
 * @param context    Application specific context, not used in this app.
 */
static void select_click_handler(ClickRecognizerRef recognizer, void *context){
	send_eta_req(preset);
	refreshing_text();
}

/**
 * Function to update the route and stop layers to the preset's stop and route names.
 */
static void update_preset_text(){
	if (window_is_loaded(window) && preset != NULL && preset_pos != -1){
		preset = presets_get(preset_pos);
		text_layer_set_text(stop_layer, preset->stop_name);
		text_layer_set_text(route_layer, preset->route_name);
	}
}

/**
 * Function to change the time and sub time text layers to show
 * that the app is waiting on an ETA message from the phone.
 */
static void refreshing_text(){
	text_layer_set_text(time_layer, " -- ");
	text_layer_set_text(minute_text_layer, "Getting ETA...");
}

/**
 * Function to display the ETA when a PRESET ETA message is recieved from the phone.
 */
void update_time_text(){
	if (stop_layer != NULL){
		if (presets_get_count()-1 < preset_pos && window_is_loaded(window)){
			if (presets_get_count() == 0){
				preset_pos = -1;
				window_stack_pop(true);
			}
			else{
				preset_pos = 0;
				preset = presets_get(preset_pos);
			}
		}
		update_preset_text();
		if (window_is_loaded(window)){
			if (preset != NULL && preset->eta > 0){
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
				text_layer_set_text(time_layer, "NOW");
				text_layer_set_text(minute_text_layer, " ");
			}
			else if (preset != NULL && (preset->eta == PRESET_REFRESHING_ETA || preset->eta == PRESET_SENT_REQUEST))
				refreshing_text();
			else{
				if (time_layer != NULL){
					text_layer_set_text(time_layer, " -- ");
					text_layer_set_text(minute_text_layer, "No Available ETA");
				}
			}
		}
	}
}