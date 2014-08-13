#include <pebble.h>
#include "common.h"

typedef struct MessageHandler MessageHandler;
struct MessageHandler{

};

static void app_message_init()
{
	app_message_open(app_message_inbox_size_maximum(),app_message_outbox_size_maximum());
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_failed(out_failed_handler);
}

/*
* Handler function for receiving the information from the phone.
* If the dictionary is a time update, simply process the tuple.
* Otherwise, send it to the above helper method to ensure the dictionary values
* are in the correct order.
*/
static void in_received_handler(DictionaryIterator *iter, void *context)
{
	Tuple *t = dict_read_first(iter);
	if (t)
	{
		if (t->key == 0)
			process_tuple(t);
		else
			fix_dict_order(iter);
	}
}

/*
* Helper function for translating an error number into a readable string. 
* Used for debugging.
*/
static char *translate_error(AppMessageResult result) 
{
	switch (result)
	{
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
static void in_dropped_handler(AppMessageResult reason, void *context) 
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped! Reason = %s", translate_error(reason));
}
/*
* Function for printing out information when the app fails to send a message to the phone.
* Used for debugging.
*/
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) 
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send! %d : %s", reason, translate_error(reason));
	if (reason == APP_MSG_SEND_TIMEOUT)
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Resending App Message.");
		update_time();
	}
}