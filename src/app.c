#include "pebble.h"
#include "presets.h"
#include "preset.h"
#include "windows/window-presets.h"
#include "windows/window-preset.h"
#include "windows/window-error.h"
#include "libs/message-queue/message-queue.h"

/**
 * Function to test if persistent storage is working.
 * @return If persist is working.
 */
static bool persist_working(){
	int test = persist_write_bool(100, true);
	persist_delete(100);
	if (test < 0)
		return false;
	return true;
}

/**
 * Initialization method. Sets up the app. Creates message queue, presets list, and main app window.
 * Also displays an error window if persistent storage is broken.
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

/**
 * Deinitiliazation method. Destroys everything in this file's scope.
 */
static void deinit()
{

	presets_deinit();
	window_error_destroy();
	window_presets_destroy();
}

/**
 * Main!
 */
int main()
{
	init();
	app_event_loop();
	deinit();
}