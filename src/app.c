#include "pebble.h"
#include "presets.h"
#include "preset.h"
#include "windows/window-presets.h"
#include "windows/window-preset.h"
#include "windows/window-error.h"
#include "libs/message-queue/message-queue.h"


static bool persist_working(){
	int test = persist_write_bool(100, true);
	persist_delete(100);
	if (test < 0)
		return false;
	return true;
}

/*
* Initialization method. Creates the window and assigns the window, click, and tick_timer handlers.
* Also loads up the last selected preset as the current view.
*/
static void init()
{
	bool is_persist_working = persist_working();
	mqueue_init();
	presets_init();
	window_presets_init();
	if (is_persist_working)
		presets_restore();
	window_presets_show();
	if (!is_persist_working){
		window_error_init();
		window_error_set_text(
			"Error. Persist storage is broken.\nPlease factory reset device.\n\nPlease see bit.ly/ urpebstorage\nfor more information.\n\nLoading presets from phone instead.\n");
		window_error_show();
		presets_restore_from_phone();
	}
}

/*
* Deinitilization method. Destroy everything.
* Also, write the current view into storage so init() can load it next time.
*/
static void deinit()
{

	presets_deinit();
	window_error_destroy();
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