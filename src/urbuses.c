#include "pebble.h"

#define NUM_STOPS 14
#define NUM_ROUTES 1

enum{
	CURRENT_VIEW_TIME= 0,
    PRESET_NUMBER= 1,
    PRESET_ROUTE_ID= 2,
    PRESET_ROUTE_NAME= 3,
    PRESET_STOP_ID= 4,
    PRESET_STOP_NAME= 5
};

static Window *window;

static TextLayer *stop_layer;
static TextLayer *route_layer;
static TextLayer *time_layer;
static TextLayer *minute_text_layer;

int most_recent_preset = 0;
int current_view = 1;
int time_estimate = -54; /*Random number that'll never be reached. Used to prevent the tick timer from messing stuff up on init*/
char time_buffer[32];
char stop_buffer[32];
char route_buffer[32];



static char* stops_name[NUM_STOPS] = 
{
	"Rush Rhees",
	"Wilmot",
	"Goler Bus Stop",
	"Helen Wood",
	"Southside",
	"Laser Lab",
	"Whipple",
	"Goler Bus Shelter ",
	"Wilson&McLean",
	"South&Alex",
	"Monroe&Alex",
	"East&Alex",
	"Eastman",
	"MtHope&McLean"
};
static char* stops[NUM_STOPS] = 
{
	"4148446",
	"4130838",
	"4136314",
	"4112150",
	"4140914",
	"4140918",
	"4140922",
	"4140938",
	"4140926",
	"4130858",
	"4130862",
	"4112178",
	"4140930",
	"4140934"
};
static char* routes[NUM_ROUTES] = {"4004990"};
static char* routes_name[NUM_ROUTES] = {"Summer Line"};


void send_route_request(){
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);

	Tuplet route = TupletInteger(PRESET_ROUTE_ID, persist_read_int(PRESET_ROUTE_ID*10+current_view));
	dict_write_tuplet(iter, &route);
	Tuplet stop = TupletInteger(PRESET_STOP_ID, persist_read_int(PRESET_STOP_ID*10+current_view));
	dict_write_tuplet(iter, &stop);

	app_message_outbox_send();
}
static void process_tuple(Tuple *t){
	int key = t->key;
	int value = t->value->int32;	
	char string_value[64];
	strcpy(string_value, t->value->cstring);
	switch(key){
	case CURRENT_VIEW_TIME:
		APP_LOG(APP_LOG_LEVEL_DEBUG,"Updating current time");
		if (value > 0){
			char text[64] = "";
			snprintf(time_buffer, sizeof (" XX "), " %d ", value);
			if (persist_read_string(PRESET_STOP_NAME*10+current_view, text,64) != E_DOES_NOT_EXIST){
				snprintf(stop_buffer, sizeof("possiblyrealllongstopnamemaybe"), "%s", text);
			}
			else
			{
				snprintf(stop_buffer, sizeof("STOP PRESET X"), "STOP PRESET %d", current_view);
			}
			
			if (persist_read_string(PRESET_ROUTE_NAME*10+current_view, text,64) != E_DOES_NOT_EXIST){
				snprintf(route_buffer, sizeof("possiblyrealllongroutenamemaybe"), "%s", text);
			}
			else
			{
				snprintf(route_buffer, sizeof("STOP PRESET X"), "ROUTE PRESET %d", current_view);
			}

			text_layer_set_text(stop_layer, (char*) &stop_buffer);
			text_layer_set_text(route_layer, (char*) &route_buffer);			
			text_layer_set_text(time_layer, (char*) &time_buffer);
			if (value == 1){
		  		text_layer_set_text(minute_text_layer, "minute");
		  	}
		  	else{
		  		text_layer_set_text(minute_text_layer, "minutes");
		  	}
	  	}
		else if (value == -1)
		{
			text_layer_set_text(time_layer, " -- ");
			text_layer_set_text(minute_text_layer, "No Arrival Times");	
		}
		else{
			text_layer_set_text(time_layer, "NOW");
			text_layer_set_text(minute_text_layer, "");
		}
		time_estimate = value;
		break;
	case PRESET_NUMBER:
		most_recent_preset = value;
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Updating most_recent_preset");
		break;
	case PRESET_ROUTE_ID:
		persist_write_int(PRESET_ROUTE_ID*10+most_recent_preset, value);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Writing route_id");
		break;
	case PRESET_ROUTE_NAME:
		persist_write_string(PRESET_ROUTE_NAME*10+most_recent_preset, string_value);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Writing route_name");
		break;
	case PRESET_STOP_ID:
		persist_write_int(PRESET_STOP_ID*10+most_recent_preset, value);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Writing stop_id");
		break;
	case PRESET_STOP_NAME:
		persist_write_string(PRESET_ROUTE_NAME*10+most_recent_preset, string_value);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Writing stop_name");
		send_route_request();
		break;
	}

}
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
static void app_message_init(){
	app_message_register_inbox_received(in_received_handler);

	app_message_open(app_message_inbox_size_maximum(),app_message_outbox_size_maximum());
}
static void window_load(Window *window)
{
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);

	stop_layer = text_layer_create(GRect(0, 5, bounds.size.w /* width */, 28 /* height */));
	
	snprintf(stop_buffer, sizeof("STOP PRESET X"), "STOP PRESET %d", current_view);
	text_layer_set_text(stop_layer, (char*) &stop_buffer);
	text_layer_set_font(stop_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(stop_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(stop_layer));

	route_layer = text_layer_create(GRect(0,33, bounds.size.w, 28));

	snprintf(route_buffer, sizeof("ROUTE PRESET X"), "ROUTE PRESET %d", current_view);
	text_layer_set_text(route_layer, (char*) &route_buffer);
	text_layer_set_font(route_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(route_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(route_layer));

	snprintf(time_buffer, sizeof(" XX "), " -- ");

	time_layer = text_layer_create(GRect(0,61, bounds.size.w, 50));
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text(time_layer, (char*) &time_buffer);

	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	
	layer_add_child(window_layer, text_layer_get_layer(time_layer));

	minute_text_layer = text_layer_create(GRect(0,111, bounds.size.w, 28));
	text_layer_set_text(minute_text_layer, "minutes");
	text_layer_set_font(minute_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(minute_text_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(minute_text_layer));

	send_route_request();
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
	send_route_request();
	// APP_LOG(APP_LOG_LEVEL_DEBUG, "entering tick_callback");
	if (tick_time->tm_min % 5 == 0){
		// APP_LOG(APP_LOG_LEVEL_DEBUG,"5 min refresh");
		send_route_request();

	}
	else{
		time_estimate--;
		// APP_LOG(APP_LOG_LEVEL_DEBUG, "Subtracting minute");
		if (time_estimate == -55){}
		else if (time_estimate <= 0)
		{
			// APP_LOG(APP_LOG_LEVEL_DEBUG, "0 minutes, getting new estimate");
			send_route_request();
		}
		else{
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Updating minute");
			snprintf(time_buffer, sizeof(" XX "), " %d ", time_estimate);
			text_layer_set_text(time_layer, (char*) &time_buffer);
			if (time_estimate == 1){
				text_layer_set_text(minute_text_layer, "minute");
			}
		}
	}
}

static void init()
{
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers){
		.load = window_load
	});
	tick_timer_service_subscribe(MINUTE_UNIT, tick_callback);
	app_message_init();

	window_stack_push(window, true);
}

static void deinit(){
	text_layer_destroy(stop_layer);
	text_layer_destroy(route_layer);
	text_layer_destroy(time_layer);
	text_layer_destroy(minute_text_layer);
	window_destroy(window);

	tick_timer_service_unsubscribe();
}

int main()
{
	init();
	app_event_loop();
	deinit();
}