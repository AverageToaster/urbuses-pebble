#include <pebble.h>
#include "../presets.h"
#include "../preset.h"
#include "window-preset.h"

static void window_load(Window *window);
static void window_unload(Window *window);
static void window_appear(Window *window);

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data);
static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data);
static void menu_select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data);
static void tick_callback(struct tm *tick_time, TimeUnits units_changed);

static Window *window;
static MenuLayer *menu_layer;

void window_presets_init(void){
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers){
		.load = window_load,
		.unload = window_unload,
		.appear = window_appear
	});
	tick_timer_service_subscribe(MINUTE_UNIT, tick_callback);
	window_preset_init();	
}

void window_presets_show(int preset_number){
	window_stack_push(window, true);
}

void window_presets_destroy(void){
	window_destroy(window);
	tick_timer_service_unsubscribe();
}

void refresh(){
	if (menu_layer != NULL){
		menu_layer_reload_data(menu_layer);
	}
}

static void window_load(Window *window){
	menu_layer = menu_layer_create(layer_get_bounds(window_get_root_layer(window)));
	menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
		.get_cell_height = menu_get_cell_height_callback,
    	.get_num_rows = menu_get_num_rows_callback,
    	.draw_row = menu_draw_row_callback,
    	.select_click = menu_select_click_callback
    });
	menu_layer_set_click_config_onto_window(menu_layer, window);
	layer_add_child(window_get_root_layer(window), menu_layer_get_layer(menu_layer));
}

static void window_unload(Window *window){
	menu_layer_destroy(menu_layer);
}

static void window_appear(Window *window){
	menu_layer_reload_data(menu_layer);
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data){
	return presets_get_count();
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data){
	// Preset *preset = presets_get(cell_index->row);
	// char* row_label = malloc(32);
	// char* sub_label = malloc(32);
	// memcpy(row_label, preset->stop_name, 32);
	// memcpy(sub_label, preset->route_name, 32);
	// GSize title_size = graphics_text_layout_get_content_size(row_label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(0,0,144,26), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);
	// GSize sub_size = graphics_text_layout_get_content_size(sub_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0, 0, 144, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft);
	// GSize min_size = graphics_text_layout_get_content_size("10 minutes", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0, 46, 144, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft);
	// int size = title_size.h + sub_size.h+min_size.h+10;
	// free(row_label);
	// free(sub_label);
	// return size;
	return 76;
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data){
	char* row_label = malloc(32);
	char* sub_label = malloc(32);
	int preset_number = cell_index->row+1;
	Preset *preset = presets_get(cell_index->row);
	memcpy(row_label, preset->stop_name, 32);
	memcpy(sub_label, preset->route_name, 32);
	graphics_context_set_text_color(ctx, GColorBlack);
	// menu_cell_basic_draw(ctx, cell_layer, row_label, sub_label, NULL);
	graphics_draw_text(ctx, row_label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(0,0,144, 26), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
	graphics_draw_text(ctx, sub_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0, 26, 144, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
	graphics_draw_text(ctx, "10 minutes", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0, 46, 144, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
	free(row_label);
	free(sub_label);
}

static void menu_select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context){
	Preset *preset = presets_get(cell_index->row);
	window_preset_set_preset(preset, cell_index->row);
	window_preset_show();
}

void set_selected_index(int pos){
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Set Selected Index to %d", pos);
	menu_layer_set_selected_index(menu_layer, (MenuIndex){.row=pos, .section=1}, MenuRowAlignCenter, false);
}

static void tick_callback(struct tm *tick_time, TimeUnits units_changed){
	layer_mark_dirty(menu_layer_get_layer(menu_layer));
}

