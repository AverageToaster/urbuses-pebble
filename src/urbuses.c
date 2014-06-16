#include "pebble.h"

// Cases for the received key from the phone.

enum{
	CURRENT_VIEW_TIME= 0,
    PRESET_NUMBER= 1,
    PRESET_ROUTE_ID= 2,
    PRESET_ROUTE_NAME= 3,
    PRESET_STOP_ID= 4,
    PRESET_STOP_NAME= 5,
    CURRENT_VIEW_PERSIST= 6
};

static Window *window;

static TextLayer *stop_layer;
static TextLayer *route_layer;
static TextLayer *time_layer;
static TextLayer *minute_text_layer;

// various global variables
static int most_recent_preset = 0;
static int current_view = 1;
static int time_estimate = -54; /*Random number that'll never be reached. Used to prevent the tick timer from messing stuff up on init*/
static char time_buffer[sizeof(" -- ")];
static char stop_buffer[64];
static char route_buffer[64];
static char minute_text_buffer[32];

/*
* Sends an app_message to the phone to request a time update.
*/
static void update_time(){
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);

	Tuplet route = TupletInteger(PRESET_ROUTE_ID, persist_read_int(current_view*10+PRESET_ROUTE_ID));
	dict_write_tuplet(iter, &route);
	Tuplet stop = TupletInteger(PRESET_STOP_ID, persist_read_int(current_view*10+PRESET_STOP_ID));
	dict_write_tuplet(iter, &stop);

	app_message_outbox_send();
}

/*
* function to update the stop and route text layers.
*/
static void update_text_layers(){
	/*
	* If the Stop/Route exists for the current view, then load it into the buffer. 
	* If not, then load the default "STOP PRESET #" text.
	*/
	if (persist_exists(current_view*10+PRESET_STOP_NAME)){
		persist_read_string(current_view*10+PRESET_STOP_NAME, stop_buffer, 64);
	}
	else
	{
		snprintf(stop_buffer, 64, "STOP PRESET %d", current_view);
	}
	// Update the text layer with the new text.
	text_layer_set_text(stop_layer, (char*) &stop_buffer);

	// Same as above.
	if (persist_exists(current_view*10+PRESET_ROUTE_NAME)){
		persist_read_string(current_view*10+PRESET_ROUTE_NAME, route_buffer, 64);
	}
	else
	{
		snprintf(route_buffer, 64, "ROUTE PRESET %d", current_view);
	}
	text_layer_set_text(route_layer, (char*) &route_buffer);
}

/*
* Function to handle the up click while inside the app. 
* When the user hits up, move the current view up by 1, looping around at 6 to 1,
* then update the text layers and current time for the new preset. 
*/
static void up_click_handler(ClickRecognizerRef recognizer, void* context){
	current_view++;
	if (current_view == 6)
		current_view = 1;
	update_text_layers();
	update_time();
}

/*
* Function to handle the down click while inside the app. 
* When the user hits down, move the current view down by 1, looping around at 0 to 5,
* then update the text layers and current time for the new preset. 
*/
static void down_click_handler(ClickRecognizerRef recognizer, void* context){
	current_view--;
	if (current_view == 0)
		current_view = 5;
	update_text_layers();
	update_time();
}

/*
* Function to handle the select click while inside the app. 
* When the user hits select, refresh the time estimate for the current preset.
*/
static void select_click_handler(ClickRecognizerRef recognizer, void* context){
	update_time();
}

/*
* Process the tuple from the dict received from the phone.
* The function parses out the key and the value from the tuple, 
* then switches on the key, based on the enum above.
*/
static void process_tuple(Tuple *t){
	int key = t->key;
	int value = t->value->int32;	
	char string_value[64];
	strcpy(string_value, t->value->cstring);
	switch(key){
	case CURRENT_VIEW_TIME:
		/*
		* received a time update. If its above 0, simply put the time in the
		* time buffer and update the time layer (with some grammar handling for 1 minute vs >1 minutes)
		*/
		if (value > 0){
			snprintf(time_buffer, sizeof (" XX "), " %d ", value);			
			text_layer_set_text(time_layer, (char*) &time_buffer);
			if (value == 1){
				snprintf(minute_text_buffer, 32, "minute");
		  		text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);
		  	}
		  	else{
		  		snprintf(minute_text_buffer, 32, "minutes");
		  		text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);
		  	}
	  	}
	  	/*
	  	* If it is -1, then the service has no available arrival estimates for the given
	  	* route/stop combination. Update the time layer to show nothing, and update the
	  	* minute layer to inform the user that there are no arrival times available.
	  	*/
		else if (value == -1)
		{
			snprintf(time_buffer, sizeof (" -- "), " -- ");
			text_layer_set_text(time_layer, (char*) &time_buffer);
			snprintf(minute_text_buffer, 32, "No Arrival Times");
			text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);	
		}
		/*
		* Last case is 0, for which the words "NOW" are simply shown.
		*/
		else{
			text_layer_set_text(time_layer, "NOW");
			text_layer_set_text(minute_text_layer, "");
		}
		time_estimate = value;
		break;
	/*
	* The remaining cases are all for recieving new presets from the phone.
	* Each case simply writes the received preset value into persistant storage for access later.
	*/
	case PRESET_NUMBER:
		most_recent_preset = value;
		break;
	case PRESET_ROUTE_ID:
		/*
		* The key for a given preset value is the preset_number * 10 + PRESET_INFO_IDENTIFIER, 
		* making the key read as a double digit number unique to that preset's information.
		* (the logic behind number*10+IDENT means that the range 12-15 will be the information for preset 1).
		*/
		persist_write_int(most_recent_preset*10+PRESET_ROUTE_ID, value);
		break;
	case PRESET_ROUTE_NAME:
		persist_write_string(most_recent_preset*10+PRESET_ROUTE_NAME, string_value);
		break;
	case PRESET_STOP_ID:
		persist_write_int(most_recent_preset*10+PRESET_STOP_ID, value);
		break;
	case PRESET_STOP_NAME:
		persist_write_string(most_recent_preset*10+PRESET_STOP_NAME, string_value);
		// If this preset update was the current screen, update the current screen to reflect the changes.
		if (most_recent_preset == current_view){
			update_text_layers();
			update_time();
		}
		break;
	}
}
/*
* Handler function for receiving the information from the phone.
* Information is received as a dictionary and split apart into Tuples.
* These are send to process_tuple for the information to be processed.
*/
static void in_received_handler(DictionaryIterator *iter, void *context)
{
	Tuple *t = dict_read_first(iter);
	if (t){
		process_tuple(t);
	}
	while (t != NULL){
		t= dict_read_next(iter);
		if (t){
			process_tuple(t);
		}
	}
}

/*
* Helper function for translating an error number into a readable string. 
* Used for debugging.
*/
static char *translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}
/*
* Function for printing out information when the app drops a response from the phone.
* Used for debugging.
*/
static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped! Reason = %s", translate_error(reason));
}
/*
* Function for printing out information when the app fails to send a message to the phone.
* Used for debugging.
*/
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send! %d : %s", reason, translate_error(reason));
}

/*
* Function to assign the button handlers to the buttons.
*/
static void click_config_provider(void* context){
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
} 
/*
* Function to initialize the app message handlers.
*/
static void app_message_init(){
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_failed(out_failed_handler);

	app_message_open(app_message_inbox_size_maximum(),app_message_outbox_size_maximum());
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
	stop_layer = text_layer_create(GRect(0, 0, bounds.size.w /* width */, 52 /* height */));
	// Set the font. Went with 24 Bold so its readable, yet small enough to fit the whole stop.
	text_layer_set_font(stop_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	// Align the text. Went with center for looks.
	text_layer_set_text_alignment(stop_layer, GTextAlignmentCenter);
	// Finally add the text layer to the screen.
	layer_add_child(window_layer, text_layer_get_layer(stop_layer));
	
	// Same with route.
	route_layer = text_layer_create(GRect(0,52, bounds.size.w, 24));
	text_layer_set_font(route_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(route_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(route_layer));

	// Update the text for Stop and Route.
	update_text_layers();

	// Set the char* buffer for time.
	snprintf(time_buffer, sizeof(" XX "), " -- ");

	// Same deal with time text layer.
	time_layer = text_layer_create(GRect(0,76, bounds.size.w, 42));
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	// Actually set the text in this method for time_layer.
	text_layer_set_text(time_layer, (char*) &time_buffer);
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(time_layer));

	// And create and add the final layer, the "minutes" text under the time.
	snprintf(minute_text_buffer, 32, "minutes");
	minute_text_layer = text_layer_create(GRect(0,118, bounds.size.w, 28));
	text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);
	text_layer_set_font(minute_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(minute_text_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(minute_text_layer));
}

/*
* Function called every minute. Handles the updating of the time displayed for the current preset.
*/
static void tick_callback(struct tm *tick_time, TimeUnits units_changed)
{
	// Every 10 minutes, it refreshes the time from the phone.
	if (tick_time->tm_min % 10 == 0){
		update_time();
	}
	else{
		// Lower the current time_estime.
		time_estimate--;
		// If the app just loaded, refresh the time estimate.
		if (time_estimate == -55){
			update_time();
		}
		// If the time has hit 0 (or lower), refresh the time.
		else if (time_estimate <= 0)
		{
			update_time();
		}
		// Otherwise, just update the time and minute layers with the new time estimate.
		else{
			snprintf(time_buffer, sizeof(" XX "), " %d ", time_estimate);
			text_layer_set_text(time_layer, (char*) &time_buffer);
			if (time_estimate == 1){
				snprintf(minute_text_buffer, 32, "minute");
				text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);
			}
		}
	}
}

/*
* Initialization method. Creates the window and assigns the window, click, and tick_timer handlers.
* Also loads up the last selected preset as the current view.
*/
static void init()
{
	if (persist_exists(CURRENT_VIEW_PERSIST)){
		current_view = persist_read_int(CURRENT_VIEW_PERSIST);
	}
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers){
		.load = window_load
	});
	app_message_init();
	window_set_click_config_provider(window, click_config_provider);
	tick_timer_service_subscribe(MINUTE_UNIT, tick_callback);
	window_stack_push(window, true);
}

/*
* Deinitilization method. Destroy everything.
* Also, write the current view into storage so init() can load it next time.
*/
static void deinit(){
	text_layer_destroy(stop_layer);
	text_layer_destroy(route_layer);
	text_layer_destroy(time_layer);
	text_layer_destroy(minute_text_layer);
	window_destroy(window);

	persist_write_int(CURRENT_VIEW_PERSIST, current_view);

	tick_timer_service_unsubscribe();
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