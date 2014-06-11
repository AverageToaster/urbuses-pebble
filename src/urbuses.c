#include "pebble.h"

#define NUM_STOPS 14
#define NUM_ROUTES 1

static Window *window;

static TextLayer *stop_layer;
static TextLayer *route_layer;
static TextLayer *time_layer;
static TextLayer *under_time_layer;

int time_estimate = -54;
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
static char* routes[NUM_STOPS] = {"4004990"};
/*// static int times[NUM_STOPS];


 Going a different route.

static void routes_window_unload(Window *window) 
{
  simple_menu_layer_destroy(simple_menu_layer_routes);
}

static void routes_window_load(Window *window){
  char* text[5] = {"route1","route2","route3","route4","route5"};

  for (int i = 0; i < NUM_ROUTES; i++){
	menu_items_routes[i] = (SimpleMenuItem){
	  .title = routes[i],
	  .subtitle = "## minutes",
	};
  }
  menu_sections_routes[0] = (SimpleMenuSection){
  .num_items = NUM_ROUTES,
  .items = menu_items_routes,
  };

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  simple_menu_layer_routes = simple_menu_layer_create(bounds, window, menu_sections_routes, 1, NULL);

  layer_add_child(window_layer, simple_menu_layer_get_layer(simple_menu_layer_routes));
}
static void menu_select_callback_stops(int index, void* ctx)
{
  sub_window = window_create();

  window_set_window_handlers(sub_window, (WindowHandlers){
	.load = routes_window_load,
	.unload = routes_window_unload,
  });
  window_stack_push(sub_window, true);
}

static void stops_window_load(Window *window){
  for (int i = 0; i <NUM_STOPS; i++)
  {
	menu_items_stops[i] = (SimpleMenuItem){
	  .title = stops_name[i],
	  .callback = menu_select_callback_stops,
	};
  }
  menu_sections_stops[0] = (SimpleMenuSection){
	.num_items = NUM_STOPS,
	.items = menu_items_stops,
  };

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  simple_menu_layer_stops = simple_menu_layer_create(bounds, window, menu_sections_stops, 1, NULL);

  layer_add_child(window_layer, simple_menu_layer_get_layer(simple_menu_layer_stops));

}

static void stops_window_unload(Window *window) 
{
  simple_menu_layer_destroy(simple_menu_layer_stops);
}


  // for (int i = 0; i < 32; i++){
  // if (stops_input[i] != NULL){
  //   // strcpy(test, stops_input[i]);
  //   // test = stops_input[i];
  //   // snprintf(test, 60, "%s %d", test, i);
  //   // APP_LOG(APP_LOG_LEVEL_DEBUG, stops_input[i]);
  // }
  // else
  // {
  //   char* test = "NULL";
  //   snprintf(test, 60, "%s test", test);
  //   APP_LOG(APP_LOG_LEVEL_DEBUG, test); 
  // }
  // }
// }
*/

static void process_tuple(Tuple *t){
  int key = t->key;
  int value = t->value->int32;
  snprintf(time_buffer, sizeof (" XX "), " %d ", value);
  text_layer_set_text(time_layer, (char*) &time_buffer);
  if (value == 1){
  	text_layer_set_text(under_time_layer, "minute");
  }
  else{
  	text_layer_set_text(under_time_layer, "minutes");
  }
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

	under_time_layer = text_layer_create(GRect(0,111, bounds.size.w, 28));
	text_layer_set_text(under_time_layer, "minutes");
	text_layer_set_font(under_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(under_time_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(under_time_layer));
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
	if (tick_time->tm_min % 5 == 0){
		send_int(5,5);
	}
	else{
		time_estimate--;
		if (time_estimate == -55){}
		else if (time_estimate <= 0)
		{
			send_int(5,5);
		}
		else{
			snprintf(time_buffer, sizeof(" XX "), " %d ", time_estimate);
			text_layer_set_text(time_layer, (char*) &time_buffer);
			if (time_estimate == 1){
				text_layer_set_text(under_time_layer, "minute");
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
	text_layer_destroy(under_time_layer);
	window_destroy(window);

	tick_timer_service_unsubscribe();
}

int main()
{
	init();
	app_event_loop();
	deinit();
}