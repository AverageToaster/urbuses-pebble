#include "pebble.h"

#define NUM_STOPS 14
#define NUM_ROUTES 1

static Window *window;

static TextLayer *stop_layer;
static TextLayer *route_layer;
static TextLayer *time_layer;
static TextLayer *minute_text_layer;

int time_estimate = -54; /*Random number that'll never be reached. Used to prevent the tick timer from messing stuff up on init*/
char time_buffer[32];


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

static void process_tuple(Tuple *t){
  int key = t->key;
  int value = t->value->int32;
  if (value > 0){
	  snprintf(time_buffer, sizeof (" XX "), " %d ", value);
	  text_layer_set_text(time_layer, (char*) &time_buffer);
	  if (value == 1){
	  	text_layer_set_text(minute_text_layer, "minute");
	  }
	  else{
	  	text_layer_set_text(minute_text_layer, "minutes");
	  }
  }
  else{
  	text_layer_set_text(time_layer, "NOW");
  	text_layer_set_text(minute_text_layer, "");
  }

  APP_LOG(APP_LOG_LEVEL_DEBUG, "");
  time_estimate = value;
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
	text_layer_set_text(stop_layer, stops_name[0]);
	text_layer_set_font(stop_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(stop_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(stop_layer));

	route_layer = text_layer_create(GRect(0,33, bounds.size.w, 28));
	text_layer_set_text(route_layer, "Summer Line");
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
	APP_LOG(APP_LOG_LEVEL_DEBUG, "entering tick_callback");
	if (tick_time->tm_min % 5 == 0){
		APP_LOG(APP_LOG_LEVEL_DEBUG,"5 min refresh");
		send_int(5,5);

	}
	else{
		time_estimate--;
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Subtracting minute");
		if (time_estimate == -55){}
		else if (time_estimate <= 0)
		{
			APP_LOG(APP_LOG_LEVEL_DEBUG, "0 minutes, getting new estimate");
			send_int(5,5);
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