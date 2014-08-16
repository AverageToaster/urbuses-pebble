#include "pebble.h"
#include "window-error.h"
#include "../libs/scroll-text-layer/scroll-text-layer.h"

static void window_load(Window *window);
static void window_unload(Window *window);
static void click_config_provider(void* context);
static void click_select(ClickRecognizerRef recognizer, void *context);

static Window *window;
static ScrollTextLayer *scroll_layer;
static char* message = NULL;

void window_error_init(void){
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers){
		.load = window_load,
		.unload = window_unload
	});
}

void window_error_set_text(char* text){
	if (message != NULL)
		free(message);
	message = malloc(sizeof(char) * strlen(text));
	strncpy(message, text, strlen(text));
	scroll_text_layer_set_text(scroll_layer, message);
}

void window_error_show(void){
	window_stack_push(window, true);
}

void window_error_destroy(){
	if (message != NULL)
		free(message);
	window_destroy(window);
}

static void window_load(Window *window){
	scroll_layer = scroll_text_layer_create_fullscreen(window);
	scroll_text_layer_set_font(scroll_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	scroll_text_layer_add_to_window(scroll_layer, window);
	window_error_set_text(message);
	scroll_layer_set_callbacks(scroll_text_layer_get_scroll_layer(scroll_layer), (ScrollLayerCallbacks){
		.click_config_provider = click_config_provider
	});
}

static void window_unload(Window *window){
	scroll_text_layer_destroy(scroll_layer);
}

static void click_config_provider(void *context){
	window_single_click_subscribe(BUTTON_ID_SELECT, click_select);
}

static void click_select(ClickRecognizerRef recognizer, void *context){
	window_stack_pop(true);
}