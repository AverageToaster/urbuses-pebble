#include "pebble.h"

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

int most_recent_preset = 0;
int current_view = 1;
int time_estimate = -54; /*Random number that'll never be reached. Used to prevent the tick timer from messing stuff up on init*/
char time_buffer[sizeof(" -- ")];
char stop_buffer[64];
char route_buffer[64];
char minute_text_buffer[32];

void print_persists(){
	char* test = "";
	for (int i = 1; i<6; i++){
		if (persist_exists(PRESET_ROUTE_ID*10+i)){
			persist_read_string(PRESET_ROUTE_NAME*10+i, test, 64);
			APP_LOG(APP_LOG_LEVEL_DEBUG, test);
		}
		if (persist_exists(PRESET_STOP_ID*10+i)){
			persist_read_string(PRESET_STOP_NAME*10+i, test, 64);
			APP_LOG(APP_LOG_LEVEL_DEBUG, test);
		}
	}
}

void send_route_request(){
	// APP_LOG(APP_LOG_LEVEL_DEBUG, "Entering send_route_request (current view = %d)", current_view);
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);

	Tuplet route = TupletInteger(PRESET_ROUTE_ID, persist_read_int(PRESET_ROUTE_ID*10+current_view));
	dict_write_tuplet(iter, &route);
	Tuplet stop = TupletInteger(PRESET_STOP_ID, persist_read_int(PRESET_STOP_ID*10+current_view));
	dict_write_tuplet(iter, &stop);

	app_message_outbox_send();
}

void update_stop_and_route_buffers(){
	if (persist_exists(PRESET_STOP_NAME*10+current_view)){
		persist_read_string(PRESET_STOP_NAME*10+current_view, stop_buffer, 64);
	}
	else
	{
		snprintf(stop_buffer, sizeof("STOP PRESET X"), "STOP PRESET %d", current_view);
	}
	// APP_LOG(APP_LOG_LEVEL_DEBUG, "Updating stop_layer, value = %s", (char*) &stop_buffer);
	text_layer_set_text(stop_layer, (char*) &stop_buffer);
	if (persist_exists(PRESET_ROUTE_NAME*10+current_view)){
		persist_read_string(PRESET_ROUTE_NAME*10+current_view, route_buffer, 64);
	}
	
	else
	{
		snprintf(route_buffer, sizeof("ROUTE PRESET X"), "ROUTE PRESET %d", current_view);
	}
	// APP_LOG(APP_LOG_LEVEL_DEBUG, "Updating route_layer, value = %s", (char*) &route_buffer);
	text_layer_set_text(route_layer, (char*) &route_buffer);
}

void up_click_handler(ClickRecognizerRef recognizer, void* context){
	current_view++;
	if (current_view == 6)
		current_view = 1;
	update_stop_and_route_buffers();
	send_route_request();
}
void down_click_handler(ClickRecognizerRef recognizer, void* context){
	current_view--;
	if (current_view == 0)
		current_view = 5;
	update_stop_and_route_buffers();
	send_route_request();
}

static void process_tuple(Tuple *t){
	int key = t->key;
	int value = t->value->int32;	
	char string_value[64];
	strcpy(string_value, t->value->cstring);
	switch(key){
	case CURRENT_VIEW_TIME:
		if (value > 0){
			snprintf(time_buffer, sizeof (" XX "), " %d ", value);			
			text_layer_set_text(time_layer, (char*) &time_buffer);
			// APP_LOG(APP_LOG_LEVEL_DEBUG, "Updating time_layer, value = %s", (char*) &time_buffer);
			if (value == 1){
				snprintf(minute_text_buffer, sizeof("minute"), "minute");
		  		text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);
		  	}
		  	else{
		  		snprintf(minute_text_buffer, sizeof("minutes"), "minutes");
		  		text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);
		  	}
	  	}
		else if (value == -1)
		{
			// APP_LOG(APP_LOG_LEVEL_DEBUG, "yo");
			snprintf(time_buffer, sizeof (" -- "), " -- ");
			text_layer_set_text(time_layer, (char*) &time_buffer);
			snprintf(minute_text_buffer, sizeof("No Arrival Times"), "No Arrival Times");
			text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);	
		}
		else{
			// APP_LOG(APP_LOG_LEVEL_DEBUG, "hey :(");
			text_layer_set_text(time_layer, "NOW");
			text_layer_set_text(minute_text_layer, "");
		}
		time_estimate = value;
		break;
	case PRESET_NUMBER:
		most_recent_preset = value;
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Updating most_recent_preset to %d", value);
		break;
	case PRESET_ROUTE_ID:
		persist_write_int(PRESET_ROUTE_ID*10+most_recent_preset, value);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Writing route_id : %d", value);
		break;
	case PRESET_ROUTE_NAME:
		persist_write_string(PRESET_ROUTE_NAME*10+most_recent_preset, string_value);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Writing route_name : %s", string_value);
		break;
	case PRESET_STOP_ID:
		persist_write_int(PRESET_STOP_ID*10+most_recent_preset, value);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Writing stop_id : %d", value);
		break;
	case PRESET_STOP_NAME:
		persist_write_string(PRESET_STOP_NAME*10+most_recent_preset, string_value);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Writing stop_name : %s", string_value);
		if (most_recent_preset == current_view){
			update_stop_and_route_buffers();
			send_route_request();
		}
		break;
	}

}
static void in_received_handler(DictionaryIterator *iter, void *context)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "entering in_received_handler");
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

char *translate_error(AppMessageResult result) {
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
static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped! Reason = %s", translate_error(reason));
}
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send! %d : %s", reason, translate_error(reason));
}

void click_config_provider(void* context){
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
} 
static void app_message_init(){
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_failed(out_failed_handler);

	app_message_open(app_message_inbox_size_maximum(),app_message_outbox_size_maximum());
}
static void window_load(Window *window)
{
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);
	stop_layer = text_layer_create(GRect(0, 0, bounds.size.w /* width */, 52 /* height */));
	if (persist_exists(PRESET_STOP_NAME*10+current_view)){
		persist_read_string(PRESET_STOP_NAME*10+current_view, stop_buffer, 64);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "UPDATING STOP : %s", (char*) &stop_buffer);
	}
	else{
		snprintf(stop_buffer, sizeof("STOP PRESET X"), "STOP PRESET %d", current_view);
	}
	text_layer_set_font(stop_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(stop_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(stop_layer));
	
	route_layer = text_layer_create(GRect(0,52, bounds.size.w, 24));
	if (persist_exists(PRESET_ROUTE_NAME*10+current_view)){
		persist_read_string(PRESET_ROUTE_NAME*10+current_view, route_buffer, 64);
	}
	else{
		snprintf(route_buffer, sizeof("ROUTE PRESET X"), "ROUTE PRESET %d", current_view);
	}
	text_layer_set_font(route_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(route_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(route_layer));
	update_stop_and_route_buffers();

	snprintf(time_buffer, sizeof(" XX "), " -- ");

	time_layer = text_layer_create(GRect(0,76, bounds.size.w, 42));
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text(time_layer, (char*) &time_buffer);

	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	
	layer_add_child(window_layer, text_layer_get_layer(time_layer));

	minute_text_layer = text_layer_create(GRect(0,118, bounds.size.w, 28));

	snprintf(minute_text_buffer, sizeof("minutes"), "minutes");
	text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);
	text_layer_set_font(minute_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(minute_text_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(minute_text_layer));

	print_persists();
}

void send_int(uint8_t key, uint8_t cmd)
{
	DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
 
    Tuplet value = TupletInteger(key, cmd);
    dict_write_tuplet(iter, &value);
 
    app_message_outbox_send();
}



void tick_callback(struct tm *tick_time, TimeUnits units_changed)
{
	// APP_LOG(APP_LOG_LEVEL_DEBUG, "entering tick_callback");
	if (tick_time->tm_min % 5 == 0){
		// APP_LOG(APP_LOG_LEVEL_DEBUG,"5 min refresh");
		send_route_request();

	}
	else{
		time_estimate--;
		// APP_LOG(APP_LOG_LEVEL_DEBUG, "Subtracting minute");
		if (time_estimate == -55){
			// send_route_request();
		}
		else if (time_estimate <= 0)
		{
			// APP_LOG(APP_LOG_LEVEL_DEBUG, "0 minutes, getting new estimate");
			send_route_request();
		}
		else{
			// APP_LOG(APP_LOG_LEVEL_DEBUG, "Updating minute");
			snprintf(time_buffer, sizeof(" XX "), " %d ", time_estimate);
			text_layer_set_text(time_layer, (char*) &time_buffer);
			if (time_estimate == 1){
				snprintf(minute_text_buffer, sizeof("minute"), "minute");
				text_layer_set_text(minute_text_layer, (char*) &minute_text_buffer);
			}
		}
	}
}

static void init()
{
	if (persist_exists(CURRENT_VIEW_PERSIST)){
		current_view = persist_read_int(CURRENT_VIEW_PERSIST);
		// APP_LOG(APP_LOG_LEVEL_DEBUG, "Updating current view to %d", current_view);

	}
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers){
		.load = window_load
	});
	app_message_init();
	window_set_click_config_provider(window, click_config_provider);
	window_stack_push(window, true);
	tick_timer_service_subscribe(MINUTE_UNIT, tick_callback);
}

static void deinit(){
	text_layer_destroy(stop_layer);
	text_layer_destroy(route_layer);
	text_layer_destroy(time_layer);
	text_layer_destroy(minute_text_layer);
	window_destroy(window);

	persist_write_int(CURRENT_VIEW_PERSIST, current_view);

	tick_timer_service_unsubscribe();
}

int main()
{
	init();
	app_event_loop();
	deinit();
}