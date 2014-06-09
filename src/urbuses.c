#include "pebble.h"

static Window *window;

static SimpleMenuLayer *simple_menu_layer;

static SimpleMenuSection menu_sections[1];
static SimpleMenuItem menu_items[5];

static void window_load(Window *window){
  int num_a_items = 0;

  for (int i = 0; i < 5; i++)
  {
    menu_items[i] = (SimpleMenuItem){
      .title = "text",
    };
  }
  menu_sections[0] = (SimpleMenuSection){
    .num_items = 5,
    .items = menu_items,
  };

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  simple_menu_layer = simple_menu_layer_create(bounds, window, menu_sections, 1, NULL);

  layer_add_child(window_layer, simple_menu_layer_get_layer(simple_menu_layer));

}

void window_unload(Window *window) 
{
  simple_menu_layer_destroy(simple_menu_layer);
}

int main()
{
  window = window_create();

  window_set_window_handlers(window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload,
  });

  window_stack_push(window, true);

  app_event_loop();

  window_destroy(window);
}