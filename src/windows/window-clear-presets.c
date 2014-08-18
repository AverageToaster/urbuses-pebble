#include <pebble.h>
#include "../presets.h"

static void window_load();
static void window_unload();
static void click_config_provider(void *context);
static void up_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_click_handler(ClickRecognizerRef recognizer, void *context);

static Window *window;
static TextLayer *text_layer;
static ActionBarLayer *action_bar;
static GBitmap *confirm_bitmap;
static GBitmap *cancel_bitmap;
static uint8_t times_confirmed = 0;

void window_clear_presets_init(){
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers){
		.load = window_load,
		.unload = window_unload
	});
}

void window_clear_presets_show(){
	window_stack_push(window, true);
}

void window_clear_presets_destroy(){
	window_destroy(window);
}

static void window_load(){
	times_confirmed = 0;
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);
	text_layer = text_layer_create(GRect(0, 36, 124, 108));
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_text(text_layer, "Are you sure you want to clear all presets?");
	layer_add_child(window_layer, text_layer_get_layer(text_layer));

	confirm_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CONFIRM);
	cancel_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CANCEL);

	action_bar = action_bar_layer_create();
	action_bar_layer_add_to_window(action_bar, window);
	action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
	action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, confirm_bitmap);
	action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, cancel_bitmap);
}

static void window_unload(){
	text_layer_destroy(text_layer);
	action_bar_layer_destroy(action_bar);
	gbitmap_destroy(confirm_bitmap);
	gbitmap_destroy(cancel_bitmap);
}

static void click_config_provider(void* context)
{
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}


static void up_click_handler(ClickRecognizerRef recognizer, void *context){
	switch (times_confirmed){
		case 0:
			times_confirmed++;
			layer_set_frame(text_layer_get_layer(text_layer), GRect(0,12,124,132));
			text_layer_set_text(text_layer, "Are you really sure? You will have to set them again manually.");
			break;
		case 1:
			times_confirmed++;
			layer_set_frame(text_layer_get_layer(text_layer), GRect(0,60,124,84));
			text_layer_set_text(text_layer, "Absolutely sure?");
			break;
		case 2:
			presets_clear();
			presets_clear_from_phone();
			window_stack_pop(true);
			break;
		default:
			window_stack_pop(true);
	}
}

static void down_click_handler(ClickRecognizerRef recognizer, void* context){
	window_stack_pop(true);
}