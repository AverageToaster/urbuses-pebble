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

/**
 * Initialization method. Creates window and assigns handlers.
 */
void window_error_init(void){
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers){
		.load = window_load,
		.unload = window_unload
	});
}

/**
 * Sets the error text of the window.
 * @param text Text to assign
 */
void window_error_set_text(char* text){
	if (message != NULL)
		free(message);
	message = malloc(sizeof(char) * strlen(text));
	strncpy(message, text, strlen(text));
	scroll_text_layer_set_text(scroll_layer, message);
}

/**
 * Shows the window, with animation.
 */
void window_error_show(void){
	window_stack_push(window, true);
}

/**
 * Deinitialization method. Destroys the window.
 */
void window_error_destroy(){
	if (message != NULL)
		free(message);
	window_destroy(window);
}

/**
 * Window load method. Creates the various layers inside the window.
 * @param window Window being loaded.
 */
static void window_load(Window *window){
	scroll_layer = scroll_text_layer_create_fullscreen(window);
	scroll_text_layer_set_font(scroll_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	scroll_text_layer_add_to_window(scroll_layer, window);
	window_error_set_text(message);
	scroll_layer_set_callbacks(scroll_text_layer_get_scroll_layer(scroll_layer), (ScrollLayerCallbacks){
		.click_config_provider = click_config_provider
	});
}

/**
 * Window unload method. Destroys the various layers inside the window.
 * @param window Window being destroyed.
 */
static void window_unload(Window *window){
	scroll_text_layer_destroy(scroll_layer);
}

/**
 * Function to allow the window to recognize when buttons have been pressed.
 * This assigned SELECT to the click handler 'click_select'
 * @param context Application speicific data, not used in this app.
 */
static void click_config_provider(void *context){
	window_single_click_subscribe(BUTTON_ID_SELECT, click_select);
}

/**
 * Function to handle when the SELECT button has been clicked.
 * In this window, that just exits the error screen.
 * @param recognizer Used to recognize that SELECT has been pressed.
 * @param context    Application speicific data, not used in this app.
 */
static void click_select(ClickRecognizerRef recognizer, void *context){
	window_stack_pop(true);
}