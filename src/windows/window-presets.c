#include <pebble.h>
#include "../presets.h"
#include "../preset.h"

static void window_load(Window *window);
static void window_unload(Window *window);

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data);
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data);
static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data);
static void menu_select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data);

static Window *window;
static MenuLayer *menu_layer;

void window_presets_init(void){
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers){
		.load = window_load,
		.unload = window_unload
	});
}

void window_presets_show(int preset_number){
	window_stack_push(window, true);
}

void window_presets_destroy(void){
	window_destroy(window);
}

static void window_load(Window *window){
	menu_layer = menu_layer_create(layer_get_bounds(window_get_root_layer(window)));
	menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
		.get_cell_height = menu_get_cell_height_callback,
    	.get_num_rows = menu_get_num_rows_callback,
    	.draw_row = menu_draw_row_callback,
    });
	menu_layer_set_click_config_onto_window(menu_layer, window);
	layer_add_child(window_get_root_layer(window), menu_layer_get_layer(menu_layer));
}

static void window_unload(Window *window){
	menu_layer_destroy(menu_layer);
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data){
	return 5;
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data){
	return 36;
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data){
	char* row_label = malloc(20);
	int preset_number = cell_index->row+1;
	snprintf(row_label, 20, "Preset %d", preset_number);
	graphics_context_set_text_color(ctx, GColorBlack);
	menu_cell_basic_draw(ctx, cell_layer, row_label, NULL, NULL);
}

static void menu_select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context){
	Preset *preset = presets_get(cell_index->row);
	window_preset_set_preset(preset, cell_index->row);	
}