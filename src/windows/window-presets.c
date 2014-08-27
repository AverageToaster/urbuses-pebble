#include <pebble.h>
#include "../presets.h"
#include "../preset.h"
#include "window-preset.h"
#include "window-about.h"
#include "window-clear-presets.h"
#include "window-no-presets.h"

static void window_load(Window *window);
static void window_unload(Window *window);
static void window_appear(Window *window);
static void window_disappear(Window *window);

static void reset_cell_height_sizes();
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data);
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data);
static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data);
static void menu_select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data);
static void tick_callback(struct tm *tick_time, TimeUnits units_changed);
static void draw_preset_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data);
static void draw_footer_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data);
static void accel_tap_handler(AccelAxisType axis, int32_t direction);

static Window *window;
static MenuLayer *menu_layer;
static GBitmap *info_bitmap;
static GBitmap *question_bitmap;
static GBitmap *delete_bitmap;
static int sizes[5][2];

/**
 * Initialization method. Creates window and assigns handlers.
 */
void window_presets_init(void){
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers){
		.load = window_load,
		.unload = window_unload,
		.appear = window_appear,
		.disappear = window_disappear
	});
	window_preset_init();
	window_about_init();
	window_clear_presets_init();
	window_no_presets_init();
	accel_tap_service_subscribe(accel_tap_handler);
}

/**
 * Shows the window, with animation.
 */
void window_presets_show(int preset_number){
	window_stack_push(window, true);
}

/**
 * Deinitialization method. Destroys the window.
 */
void window_presets_destroy(void){
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Start of window_presets_destroy()");
	window_preset_destroy();
	window_about_destroy();
	window_clear_presets_destroy();
	window_no_presets_destroy();
	window_destroy(window);
	accel_tap_service_unsubscribe();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "End of window_presets_destroy()");
}

/**
 * Function to force the menu to redraw itself.
 */
void refresh(){
	if (menu_layer != NULL){
		reset_cell_height_sizes();
		menu_layer_reload_data(menu_layer);
	}
}

/**
 * Function to force the selected index to move back to the first item in the list.
 */
void reset_selected_index(){
	if (menu_layer != NULL && presets_get_count() > 0){
		menu_layer_set_selected_index(menu_layer, (MenuIndex){.row=0, .section=0}, MenuRowAlignTop, false);
	}
}

/**
 * Window load method. Creates the various layers inside the window.
 * @param window Window being loaded.
 */
static void window_load(Window *window){
	reset_cell_height_sizes();
	menu_layer = menu_layer_create(layer_get_bounds(window_get_root_layer(window)));
	menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
		.get_num_sections = menu_get_num_sections_callback,
		.get_cell_height = menu_get_cell_height_callback,
    	.get_num_rows = menu_get_num_rows_callback,
    	.draw_row = menu_draw_row_callback,
    	.select_click = menu_select_click_callback
    });
	menu_layer_set_click_config_onto_window(menu_layer, window);
	layer_add_child(window_get_root_layer(window), menu_layer_get_layer(menu_layer));

	info_bitmap = gbitmap_create_with_resource(RESOURCE_ID_INFO);
	question_bitmap = gbitmap_create_with_resource(RESOURCE_ID_QUESTION);
	delete_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DELETE);

}

/**
 * Window unload method. Destroys the various layers inside the window.
 * @param window Window being destroyed.
 */
static void window_unload(Window *window){
	gbitmap_destroy(info_bitmap);
	gbitmap_destroy(question_bitmap);
	gbitmap_destroy(delete_bitmap);
	menu_layer_destroy(menu_layer);
}

/**
 * Function to redraw the menu when the window reappears.
 * @param window Window to redraw.
 */
static void window_appear(Window *window){
	menu_layer_reload_data(menu_layer);
	accel_tap_service_subscribe(accel_tap_handler);
}

static void window_disappear(Window *window){
	accel_tap_service_unsubscribe();
}

static void reset_cell_height_sizes(){
	for (int i = 0; i < 5; i++){
		sizes[i][0] = -1;
		sizes[i][1] = -1;
	}
}

/**
 * Callback for getting the number of sections in the menu layer.
 * @param  menu_layer Specific menu layer to get the number of sections of.
 * @param  data       Application specific data, not used in this app.
 * @return            The number of sections in this menu.
 */
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data){
	return 2;
}

/**
 * Returns the number of rows in a given sections of a menu.
 * @param  menu_layer    Specific menu layer for this callback
 * @param  section_index Section to get the number of rows in
 * @param  data          Application specific data, not used in this app.
 * @return               The number of rows in the given section
 */
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data){
	if (section_index == 0)
		return presets_get_count();
	else{
		return 2;
	}
}

/**
 * Returns the height of a given cell in a menu row.
 * @param  menu_layer Specific menu layer for this callback
 * @param  cell_index The section and row combination who's height is being requested
 * @param  data       Application specific data, not used in this app.
 * @return            The height of the cell
 */
static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data){
	if (cell_index->section == 0){
		if (sizes[cell_index->row][0] == -1){
			Preset *preset = presets_get(cell_index->row);
			sizes[cell_index->row][0] = graphics_text_layout_get_content_size(preset->stop_name, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(0,0,144,26), GTextOverflowModeWordWrap, GTextAlignmentCenter).h;
			sizes[cell_index->row][1] = graphics_text_layout_get_content_size(preset->route_name, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0,26,144,20), GTextOverflowModeWordWrap, GTextAlignmentCenter).h;
		}
		return sizes[cell_index->row][0] + sizes[cell_index->row][1] + 30;
	}
	return 36;
}

/**
 * Function to draw the cells of a menu layer.
 * @param ctx        Graphics context of the cell
 * @param cell_layer Specific layer being drawn
 * @param cell_index The section and row combination for the drawing cell.
 * @param data       Application specific data, not used in this app.
 */
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data){
	if (cell_index->section == 0)
		draw_preset_row(ctx, cell_layer, cell_index, data);
	else
		draw_footer_row(ctx, cell_layer, cell_index, data);
}

/**
 * Function to draw a Preset cell.
 * @param ctx        Graphics context of the cell
 * @param cell_layer Specific layer being drawn
 * @param cell_index The section and row combination for the drawing cell.
 * @param data       Application specific data, not used in this app.
 */
static void draw_preset_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data){
	char* eta_label = malloc(32);
	Preset *preset = presets_get(cell_index->row);
	if (preset->eta > 0)
		snprintf(eta_label, 32, (preset->eta != 1 ? "%d minutes" : "%d minute"), preset->eta);
	else if (preset->eta == PRESET_REFRESHING_ETA || preset->eta == PRESET_SENT_REQUEST)
		snprintf(eta_label, 32, "Getting ETA...");
	else if (preset->eta == 0)
		snprintf(eta_label, 32, "NOW");
	else if (preset->eta <= PRESET_NO_ETA)
		snprintf(eta_label, 32, "No Available ETA");
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, preset->stop_name, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(0,0,144, 26), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	graphics_draw_text(ctx, preset->route_name, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0, sizes[cell_index->row][0], 144, 20), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	graphics_draw_text(ctx, eta_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(0, sizes[cell_index->row][0]+sizes[cell_index->row][1], 144, 20), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

	free(eta_label);
}
/**
 * Function to draw a footer cell
 * @param ctx        Graphics context of the cell
 * @param cell_layer Specific layer being drawn
 * @param cell_index The section and row combination for the drawing cell
 * @param data       Application specific data, not used in this app.
 */
static void draw_footer_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data){
	char* row_label = malloc(20);
	if (cell_index->row == 1){
		strcpy(row_label, "About");
		graphics_draw_bitmap_in_rect(ctx, info_bitmap, GRect(4,4,28,28));
	}
	else if (presets_get_count() == 0){
		strcpy(row_label, "No Presets");
		graphics_draw_bitmap_in_rect(ctx, question_bitmap, GRect(4,4,28,28));
	}
	else{
		strcpy(row_label, "Clear Presets");
		graphics_draw_bitmap_in_rect(ctx, delete_bitmap, GRect(4,4,28,28));
	}
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, row_label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(36,0,122,28), 0, GTextAlignmentLeft, NULL);
	free(row_label);
}



/**
 * Function to handle when a menu cell is selected
 * @param menu_layer       Specific menu layer for this callback
 * @param cell_index       The section and row combination for the selected cell
 * @param data Application specific data, not used in this app.
 */
static void menu_select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data){
	if (cell_index->section == 0){
		if (cell_index->row >= presets_get_count())
			return;
		Preset *preset = presets_get(cell_index->row);
		window_preset_set_preset(preset, cell_index->row);
		window_preset_show();
	}
	else{
		if (cell_index->row == 0){
			if (presets_get_count() == 0)
				window_no_presets_show();
			else
				window_clear_presets_show();
		}
		else{
			window_about_show();
		}
	}
}


static void accel_tap_handler(AccelAxisType axis, int32_t direction)
{
	menu_layer_reload_data(menu_layer);
	send_all_eta_req();
}
/**
 * Function to move the menu layer to the specific position
 * @param pos Index for the menu to be moved to.
 */
void set_selected_index(int pos){
	menu_layer_set_selected_index(menu_layer, (MenuIndex){.row=pos, .section=0}, MenuRowAlignCenter, false);
}