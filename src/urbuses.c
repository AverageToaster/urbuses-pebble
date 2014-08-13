#include "pebble.h"
#include "windows/window-presets.h"

// Set the number of presets.
#define NUM_OF_PRESETS 5
// Cases for the received key from the phone.
// enum
// {
// 	CURRENT_VIEW_TIME= 0,
// 	PRESET_NUMBER= 1,
// 	PRESET_ROUTE_ID= 2,
// 	PRESET_ROUTE_NAME= 3,
// 	PRESET_STOP_ID= 4,
// 	PRESET_STOP_NAME= 5,
// 	CURRENT_VIEW_PERSIST= 6
// };

// static Window *window;

// static TextLayer *stop_layer;
// static TextLayer *route_layer;
// static TextLayer *time_layer;
// static TextLayer *minute_text_layer;

// // various global variables
// static int most_recent_preset = 0;
// static int current_view = 1;
// // Set initial time estimate to -1, so it automatically refreshes on load.
// static int time_estimate = -1;
// static char time_buffer[sizeof(" -- ")];
// static char stop_buffer[64];
// static char route_buffer[64];
// static char minute_text_buffer[32];
// // Array for checking which presets exist without having to access persistant storage multiple times.
// static bool isset[NUM_OF_PRESETS];
// // Boolean variable for going from no preset display to preset display.
// static bool not_preset_layout;
// static bool persist_storage_working = true;

// /*
//  * Function to check if any presets are set.
//  * Array is set during appmessage reception and on init.
//  */
// static bool presets_exist()
// {
// 	for (int i = 0; i < NUM_OF_PRESETS; i++)
// 		if (isset[i])
// 			return true;
// 	return false;
// }
// /*
//  * Function to test if persist storage is working.
//  * If not, then set the global boolean to false. 
//  * Display of error is shown elsewhere.
//  */
// static void test_if_persist_works()
// {
// 	int test = persist_write_bool(100, true);
// 	if (test < 0)
// 		persist_storage_working = false;
// }
// /*
// * Function to update time and minute text layers to show that the time is refreshing.
// */
// static void refreshing_text_layer()
// {
// 	if (presets_exist())
// 	{
// 		snprintf(time_buffer, sizeof(" XX "), " -- ");
// 		text_layer_set_text(time_layer, (char*) &time_buffer);
// 		snprintf(minute_text_buffer, 32, "Refreshing...");
// 		text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);
// 	}
// }
// /*
// * Sends an app_message to the phone to request a time update.
// */
// static void update_time()
// {
// 	refreshing_text_layer();
// 	DictionaryIterator *iter;
// 	app_message_outbox_begin(&iter);

// 	char read_route_buffer[64];
// 	char read_stop_buffer[64];
// 	persist_read_string(current_view*10+PRESET_ROUTE_ID, read_route_buffer, 64);
// 	dict_write_cstring(iter, PRESET_ROUTE_ID, read_route_buffer);
// 	persist_read_string(current_view*10+PRESET_STOP_ID, read_stop_buffer, 64);
// 	dict_write_cstring(iter, PRESET_STOP_ID, read_stop_buffer);
// 	app_message_outbox_send();
// }
// /*
//  * Function to display when persistant storage is broken.
//  */
// static void persist_storage_broken()
// {
// 	text_layer_set_size(stop_layer, GSize(layer_get_bounds(text_layer_get_layer(stop_layer)).size.w, 52));
// 	snprintf(stop_buffer, 64, "Persist Storage Broken.");
// 	text_layer_set_text(stop_layer, (char*) &stop_buffer);
// 	layer_set_frame(text_layer_get_layer(route_layer), GRect(0,47, (layer_get_bounds(text_layer_get_layer(route_layer))).size.w, 52));
// 	snprintf(route_buffer, 64, "Please Factory Reset Watch.");
// 	text_layer_set_text(route_layer, (char*) &route_buffer);
// 	snprintf(time_buffer, sizeof(" "), " ");
// 	layer_set_frame(text_layer_get_layer(time_layer), GRect(0, 99, (layer_get_bounds(text_layer_get_layer(time_layer))).size.w, 0));
// 	text_layer_set_text(time_layer, (char*) &time_buffer);
// 	snprintf(minute_text_buffer, 32, "For Info: bit.ly/ urbpebstorage");
// 	layer_set_frame(text_layer_get_layer(minute_text_layer), GRect(0,100,(layer_get_bounds(text_layer_get_layer(minute_text_layer))).size.w, 52));
// 	text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);
// }
// /*
//  * Function to display when user has no presets. 
//  * Reorganizes the text layers to display nicely.
//  */
// static void no_presets_text()
// {
// 	not_preset_layout = true;
// 	text_layer_set_size(stop_layer, GSize(layer_get_bounds(text_layer_get_layer(stop_layer)).size.w, 28));
// 	snprintf(stop_buffer, 64, "No Presets Set.");
// 	text_layer_set_text(stop_layer, (char*) &stop_buffer);
// 	layer_set_frame(text_layer_get_layer(route_layer), GRect(0,23, (layer_get_bounds(text_layer_get_layer(route_layer))).size.w, 52));
// 	snprintf(route_buffer, 64, "Please set presets via settings page.");
// 	text_layer_set_text(route_layer, (char*) &route_buffer);
// 	snprintf(time_buffer, sizeof(":("), ":(");
// 	text_layer_set_text(time_layer, (char*) &time_buffer);
// 	snprintf(minute_text_buffer, 32, " ");
// 	text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);
// }
// /*
//  * Function to fix the layout to return to normal ETA display after no_presets_text has been called.
//  */
// static void fix_layout()
// {
// 	not_preset_layout = false;
// 	text_layer_set_size(stop_layer, GSize(layer_get_bounds(text_layer_get_layer(stop_layer)).size.w,52));
// 	layer_set_frame(text_layer_get_layer(route_layer), GRect(0,47, (layer_get_bounds(text_layer_get_layer(route_layer))).size.w, 28));
// 	// move current view to the first available preset.
// 	for (int i = 1; i <= NUM_OF_PRESETS; i++)
// 	{
// 		if (isset[i])
// 		{
// 			current_view = i;
// 			break;
// 		}
// 	}
// }

// /*
// * function to update the stop and route text layers.
// */
// static void update_text_layers()
// {
// 	// First check and redirect to error screens if necessary.
// 	if (!persist_storage_working)
// 		persist_storage_broken();
// 	else if (!presets_exist())
// 		no_presets_text();
// 	else{
// 		// Check to see if we need to fix the layout if coming from no_presets_text.
// 		if (not_preset_layout)
// 			fix_layout();
// 		// Set the current view's stop name into the buffer.
// 		persist_read_string(current_view*10+PRESET_STOP_NAME, stop_buffer, 64);
// 		// Update the text layer with the new text.
// 		text_layer_set_text(stop_layer, (char*) &stop_buffer);

// 		// Same as above.
// 		persist_read_string(current_view*10+PRESET_ROUTE_NAME, route_buffer, 64);
// 		text_layer_set_text(route_layer, (char*) &route_buffer);
// 		update_time();	
// 	}
// }



// /*
// * Function to handle the up click while inside the app. 
// * When the user hits up, move the current view up to the next available preset, looping around at NUM_OF_PRESET+1 to 1,
// * then update the text layers and current time for the new preset. 
// */
// static void up_click_handler(ClickRecognizerRef recognizer, void* context)
// {
// 	if (persist_storage_working && presets_exist()){
// 		int next_view = current_view;
// 		do
// 		{
// 			next_view++;
// 			if (next_view == NUM_OF_PRESETS+1)
// 				next_view = 1;
// 			if (next_view == current_view)
// 				break;
// 		}
// 		while (!isset[next_view-1]);
// 		current_view = next_view;
// 		update_text_layers();
// 	}
// }

// /*
// * Function to handle the down click while inside the app. 
// * When the user hits down, move the current view down to the next available preset, looping around at 0 to NUM_OF_PRESETS,
// * then update the text layers and current time for the new preset. 
// */
// static void down_click_handler(ClickRecognizerRef recognizer, void* context)
// {
// 	if (persist_storage_working && presets_exist())
// 	{
// 		int next_view = current_view;
// 		do
// 		{
// 			next_view--;
// 			if (next_view == 0)
// 				next_view = NUM_OF_PRESETS;
// 			if (next_view == current_view)
// 				break;
// 		}
// 		while (!isset[next_view-1]);
// 		current_view = next_view;
// 		update_text_layers();
// 	}
// }

// /*
// * Function to handle the select click while inside the app. 
// * When the user hits select, refresh the time estimate for the current preset.
// */
// static void select_click_handler(ClickRecognizerRef recognizer, void* context)
// {
// 	if (persist_storage_working && presets_exist())
// 		refreshing_text_layer();
// }

// /*
// * Process the tuple from the dict received from the phone.
// * The function parses out the key and the value from the tuple, 
// * then switches on the key, based on the enum above.
// */
// static void process_tuple(Tuple *t)
// {
// 	int key = t->key;
// 	int value = t->value->int32;	
// 	char string_value[64];
// 	strcpy(string_value, t->value->cstring);
// 	int bytes = 1;
// 	switch(key)
// 	{
// 		case CURRENT_VIEW_TIME:
// 			/*
// 			* received a time update. If its above 0, simply put the time in the
// 			* time buffer and update the time layer (with some grammar handling for 1 minute vs >1 minutes)
// 			*/
// 			if (value > 0)
// 			{
// 				snprintf(time_buffer, sizeof (" XX "), " %d ", value);			
// 				text_layer_set_text(time_layer, (char*) &time_buffer);
// 				if (value == 1)
// 				{
// 					snprintf(minute_text_buffer, 32, "minute");
// 			  		text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);
// 			  	}
// 			  	else
// 			  	{
// 			  		snprintf(minute_text_buffer, 32, "minutes");
// 			  		text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);
// 			  	}
// 		  	}
// 		  	/*
// 		  	* If it is -1, then the service has no available arrival estimates for the given
// 		  	* route/stop combination. Update the time layer to show nothing, and update the
// 		  	* minute layer to inform the user that there are no arrival times available.
// 		  	*/
// 			else if (value == -1)
// 			{
// 				if (presets_exist())
// 				{
// 					snprintf(time_buffer, sizeof (" -- "), " -- ");
// 					text_layer_set_text(time_layer, (char*) &time_buffer);
// 					snprintf(minute_text_buffer, 32, "No Arrival Times");
// 					text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);	
// 				}
// 			}
// 			/*
// 			* Last case is 0, for which the words "NOW" are simply shown.
// 			*/
// 			else
// 			{
// 				text_layer_set_text(time_layer, "NOW");
// 				text_layer_set_text(minute_text_layer, "");
// 			}
// 			time_estimate = value;
// 			break;
		
// 		* The remaining cases are all for recieving new presets from the phone.
// 		* Each case writes the received preset value into persistant storage, if there is something to write.
// 		* If a -1 is received, that means the preset should return to default, and the persistant storage at that
// 		* preset is deleted.
		
// 		case PRESET_NUMBER:
// 			most_recent_preset = value;
// 			break;
// 		/*
// 		* The key for a given preset value is the preset_number * 10 + PRESET_INFO_IDENTIFIER, 
// 		* making the key read as a double digit number unique to that preset's information.
// 		* (the logic behind number*10+IDENT means that the range 12-15 will be the information for preset 1).
// 		*/
// 		case PRESET_ROUTE_ID:
// 			/*
// 			 * This is where the global boolean array of available presets is changed during the life of the app.
// 			 * If the preset is added during an appmessage, set it true to mark it available,
// 			 * otherwise mark it false.
// 			 */
// 			if (value != -1){
// 				bytes = persist_write_string(most_recent_preset*10+PRESET_ROUTE_ID, string_value);
// 				isset[most_recent_preset-1] = true;
// 			}
// 			else{
// 				persist_delete(most_recent_preset*10+PRESET_ROUTE_ID);
// 				isset[most_recent_preset-1] = false;
// 			}
// 			break;
// 		case PRESET_ROUTE_NAME:
// 			if (value != -1)
// 				bytes = persist_write_string(most_recent_preset*10+PRESET_ROUTE_NAME, string_value);
// 			else
// 				persist_delete(most_recent_preset*10+PRESET_ROUTE_NAME);
// 			break;
// 		case PRESET_STOP_ID:
// 			if (value != -1)
// 				bytes = persist_write_string(most_recent_preset*10+PRESET_STOP_ID, string_value);
// 			else
// 				persist_delete(most_recent_preset*10+PRESET_STOP_ID);
// 			break;
// 		case PRESET_STOP_NAME:
// 			if (value != -1)
// 				bytes = persist_write_string(most_recent_preset*10+PRESET_STOP_NAME, string_value);
// 			else{
// 				persist_delete(most_recent_preset*10+PRESET_STOP_NAME);
// 				/*
// 				 * If there are still presets available when the current view is deleted,
// 				 * search for the next available one and move to it.
// 				 */
// 				if (presets_exist() && most_recent_preset == current_view){
// 					for (int i = current_view-1; i < NUM_OF_PRESETS; i++){
// 						if (isset[i]){
// 							current_view = i;
// 							break;
// 						}
// 					}
// 					update_text_layers();
// 				}
// 			}
// 			// If this preset update was the current screen, update the current screen to reflect the changes.
// 			break;
// 	}
// 	/*
// 	 * If at any point during these writes to persistant storage a negative number is returned,
// 	 * Then that indicated that persistant storage is broken and the error screen needs to be shown.
// 	 */
// 	if (bytes <= 0){
// 		APP_LOG(APP_LOG_LEVEL_DEBUG, "Error writing");
// 		persist_storage_working = false;
// 		persist_storage_broken();
// 	}
// }
// /*
// * Function to help with processing the dictionary received from the phone.
// * Sorts the dictionary back into ascending order for correct processing.
// */
// static void fix_dict_order(DictionaryIterator *iter)
// {
// 	Tuple *tArr[NUM_OF_PRESETS];
// 	Tuple *t = dict_read_first(iter);

// 	if (t)
// 		tArr[(t->key)-1] = t;
// 	while (t != NULL)
// 	{
// 		t = dict_read_next(iter);
// 		if (t)
// 			tArr[(t->key)-1] = t;
// 	}

// 	for (int i = 0; i < NUM_OF_PRESETS; i++)
// 		process_tuple(tArr[i]);
// 	update_text_layers();
// }

// /*
// * Handler function for receiving the information from the phone.
// * If the dictionary is a time update, simply process the tuple.
// * Otherwise, send it to the above helper method to ensure the dictionary values
// * are in the correct order.
// */
// static void in_received_handler(DictionaryIterator *iter, void *context)
// {
// 	Tuple *t = dict_read_first(iter);
// 	if (t)
// 	{
// 		if (t->key == 0)
// 			process_tuple(t);
// 		else
// 			fix_dict_order(iter);
// 	}
// }

// /*
// * Helper function for translating an error number into a readable string. 
// * Used for debugging.
// */
// static char *translate_error(AppMessageResult result) 
// {
// 	switch (result)
// 	{
// 		case APP_MSG_OK: return "APP_MSG_OK";
// 		case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
// 		case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
// 		case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
// 		case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
// 		case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
// 		case APP_MSG_BUSY: return "APP_MSG_BUSY";
// 		case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
// 		case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
// 		case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
// 		case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
// 		case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
// 		case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
// 		case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
// 		default: return "UNKNOWN ERROR";
// 	}
// }
// /*
// * Function for printing out information when the app drops a response from the phone.
// * Used for debugging.
// */
// static void in_dropped_handler(AppMessageResult reason, void *context) 
// {
// 	APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped! Reason = %s", translate_error(reason));
// }
// /*
// * Function for printing out information when the app fails to send a message to the phone.
// * Used for debugging.
// */
// static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) 
// {
// 	APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send! %d : %s", reason, translate_error(reason));
// 	if (reason == APP_MSG_SEND_TIMEOUT)
// 	{
// 		APP_LOG(APP_LOG_LEVEL_DEBUG, "Resending App Message.");
// 		update_time();
// 	}
// }

// /*
// * Function to assign the button handlers to the buttons.
// */
// static void click_config_provider(void* context)
// {
// 	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
// 	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
// 	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
// } 
// /*
// * Function to initialize the app message handlers.
// */
// static void app_message_init()
// {
// 	app_message_register_inbox_received(in_received_handler);
// 	app_message_register_inbox_dropped(in_dropped_handler);
// 	app_message_register_outbox_failed(out_failed_handler);

// 	app_message_open(app_message_inbox_size_maximum(),app_message_outbox_size_maximum());
// }

// /*
// * Function called when the app loads. 
// * Creates the text layers and assigns them their default values.
// */
// static void window_load(Window *window)
// {
// 	not_preset_layout = false;
// 	// Get window layer for adding the text layers and bounds for sizing.
// 	Layer *window_layer = window_get_root_layer(window);
// 	GRect bounds = layer_get_frame(window_layer);

// 	// Create the layer by giving it a GRect for size.
// 	stop_layer = text_layer_create(GRect(0, -5, bounds.size.w /* width */, 52 /* height */));
// 	// Set the font. Went with 24 Bold so its readable, yet small enough to fit the whole stop.
// 	text_layer_set_font(stop_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
// 	// Align the text. Went with center for looks.
// 	text_layer_set_text_alignment(stop_layer, GTextAlignmentCenter);
// 	// Finally add the text layer to the screen.
// 	layer_add_child(window_layer, text_layer_get_layer(stop_layer));
	
// 	// Same with route.
// 	route_layer = text_layer_create(GRect(0,47, bounds.size.w, 28));
// 	text_layer_set_font(route_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
// 	text_layer_set_text_alignment(route_layer, GTextAlignmentCenter);
// 	layer_add_child(window_layer, text_layer_get_layer(route_layer));


// 	// Set the char* buffer for time.
// 	snprintf(time_buffer, sizeof(" XX "), " -- ");
// 	// Same deal with time text layer.
// 	time_layer = text_layer_create(GRect(0,75, bounds.size.w, 48));
// 	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
// 	// Actually set the text in this method for time_layer.
// 	text_layer_set_text(time_layer, (char*) &time_buffer);
// 	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
// 	layer_add_child(window_layer, text_layer_get_layer(time_layer));

// 	// And create and add the final layer, the "minutes" text under the time.
// 	snprintf(minute_text_buffer, 32, "minutes");
// 	minute_text_layer = text_layer_create(GRect(0,123, bounds.size.w, 28));
// 	text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);
// 	text_layer_set_font(minute_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
// 	text_layer_set_text_alignment(minute_text_layer, GTextAlignmentCenter);
// 	layer_add_child(window_layer, text_layer_get_layer(minute_text_layer));

// 	// Update the text for Stop and Route.
// 	update_text_layers();

// }

// /*
// * Function called every minute. Handles the updating of the time displayed for the current preset.
// */
// static void tick_callback(struct tm *tick_time, TimeUnits units_changed)
// {
// 	// If there are no presets or persistant storage is broken, we don't want to have anything happening in here.
// 	if (presets_exist() && persist_storage_working){
// 	// Every 10 minutes, it refreshes the time from the phone.
// 		if (tick_time->tm_min % 10 == 0)
// 			update_time();
// 		else
// 		{
// 			// Lower the current time_estime.
// 			time_estimate--;
// 			// If the app just loaded, or have gone into the negative, refresh the time.
// 			if (time_estimate <= 0)
// 				update_time();
// 			// Otherwise, just update the time and minute layers with the new time estimate.
// 			else
// 			{
// 				snprintf(time_buffer, sizeof(" XX "), " %d ", time_estimate);
// 				text_layer_set_text(time_layer, (char*) &time_buffer);
// 				if (time_estimate == 1)
// 				{
// 					snprintf(minute_text_buffer, 32, "minute");
// 					text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);
// 				}
// 			}
// 		}
// 	}
// }

/*
* Initialization method. Creates the window and assigns the window, click, and tick_timer handlers.
* Also loads up the last selected preset as the current view.
*/
static void init()
{
	// Start off by checking that persistant storage works. Nothing's going to happen if it doesnt.
	// test_if_persist_works();
	// if (persist_storage_working)
	// {
	// 	if (persist_exists(CURRENT_VIEW_PERSIST))
	// 		current_view = persist_read_int(CURRENT_VIEW_PERSIST);
	// 	// Initialize the available presets from persistant storage.
	// 	for (int i = 0; i < NUM_OF_PRESETS; i++){
	// 		if (persist_exists((i+1)*10+PRESET_ROUTE_ID))
	// 			isset[i] = true;
	// 		else
	// 			isset[i] = false;
	// 	}
	// 	window = window_create();
	// 	window_set_window_handlers(window, (WindowHandlers){
	// 		.load = window_load
	// 	});
	// 	app_message_init();
	// 	window_set_click_config_provider(window, click_config_provider);
	// 	tick_timer_service_subscribe(MINUTE_UNIT, tick_callback);
	// 	window_stack_push(window, true);	
	// }
	// else{
	// 	// If peristant storage is broken, we just want to create the window and display the error.
	// 	window = window_create();
	// 	window_set_window_handlers(window, (WindowHandlers){
	// 		.load = window_load
	// 	});
	// 	window_stack_push(window, true);
	// }
	window_presets_init();
	window_presets_show();
}

/*
* Deinitilization method. Destroy everything.
* Also, write the current view into storage so init() can load it next time.
*/
static void deinit()
{
	// text_layer_destroy(stop_layer);
	// text_layer_destroy(route_layer);
	// text_layer_destroy(time_layer);
	// text_layer_destroy(minute_text_layer);
	// window_destroy(window);

	// persist_write_int(CURRENT_VIEW_PERSIST, current_view);

	// tick_timer_service_unsubscribe();

	window_presets_destroy();
}

/*
* Main!
*/
int main()
{
	init();
	app_event_loop();
	deinit();
}