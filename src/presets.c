#include <pebble.h>
#include <math.h>
#include "presets.h"
#include "libs/linked-list/linked-list.h"
#include "libs/message-queue/message-queue.h"
#include "libs/data-processor/data-processor.h"
#include "windows/window-presets.h"
#include "windows/window-preset.h"
#include "preset.h"

#define STORAGE_PRESET_START 2
#define PRESET_BLOCK_SIZE 2
static void eta_handler(char* operation, char* data);
static void process_eta_data(char* data);
static void process_preset_data(char* data);
static void tick_callback(struct tm *tick_time, TimeUnits units_changed);

/**
 * Preset block struct used for storing Presets in persistent storage.
 */
typedef struct PresetBlock {
	Preset presets[PRESET_BLOCK_SIZE];
	uint8_t count;
	int time;
} PresetBlock;

static LinkedRoot* presets = NULL;

/**
 * Initialization function. Registers preset messages, creates the presets list, and sets up the tick timer for preset ETA countdown.
 */
void presets_init(void){
	mqueue_register_handler("PRESET", eta_handler);
	presets = linked_list_create_root();
	tick_timer_service_subscribe(MINUTE_UNIT, tick_callback);
}

/**
 * Deinitialization function. Destroys the tick timer service and saves the presets.
 */
void presets_deinit(){
	tick_timer_service_unsubscribe();
	presets_save();
}

/**
 * Function to get the Preset at a given position in the list.
 * @param  pos position of the wanted Preset.
 * @return     Preset at position pos.
 */
Preset* presets_get(int pos){
	return (Preset*) linked_list_get(presets, pos);
}

/**
 * Function to get the number of presets in the list.
 * @return number of presets in the preset list.
 */
int presets_get_count(){
	return linked_list_count(presets);
}

/**
 * Adds a preset to the preset list.
 * @param preset The preset to be added to the list.
 */
void presets_add(Preset* preset){
	linked_list_append(presets, preset);
}

/**
 * Clears the entire list of presets.
 */
void presets_clear(){
	while (presets_get_count() > 0){
		presets_remove(0);
	}
}

/**
 * Function to remove the preset at a given position.
 * @param pos Position of the preset to delete.
 */
void presets_remove(int pos){
	Preset* preset = presets_get(pos);
	free(preset);
	linked_list_remove(presets, pos);
}

/**
 * Function to restore the presets saved in persistent storage into the list.
 */
void presets_restore(void){
	presets_clear();
	if (!persist_exists(STORAGE_PRESET_START))
		return;
	int block = 0;
	PresetBlock* presetBlock = malloc(sizeof(PresetBlock));
	persist_read_data(STORAGE_PRESET_START, presetBlock, sizeof(PresetBlock));
	uint8_t preset_count = presetBlock->count;
	int save_time = presetBlock->time;
	int now = time(NULL);
	int seconds_elapsed = now-save_time;
	int minutes_elapse = (int)(floorl(seconds_elapsed / 60));
	for (int i = 0; i < preset_count; i++){
		if (i > 0 && i % PRESET_BLOCK_SIZE == 0){
			block += 1;
			free(presetBlock);
			presetBlock = malloc(sizeof(PresetBlock));
			persist_read_data(STORAGE_PRESET_START + block, presetBlock, sizeof(PresetBlock));
		}
		Preset *preset = preset_clone(&presetBlock->presets[i % PRESET_BLOCK_SIZE]);
		preset->eta -= minutes_elapse;
		if (preset->eta <= 0)
			preset->eta = PRESET_REFRESHING_ETA;
		presets_add(preset);
	}
	free(presetBlock);
	send_all_eta_req();
	return;
}

/**
 * Function to restore the presets from the localStorage on the phone.
 * Only is used when persistent storage is broken, as loading the presets
 * from the phone is much slower and battery draining than persistent storage.
 */
void presets_restore_from_phone(){
	mqueue_add("PRESET", "PRESET_RESTORE"," ");
}

/**
 * Clears all the presets in localStorage on the phone. This is used when the user clears
 * all presets from inside the app, so that the phone doesn't remember the saved presets the
 * next time the user goes to the Settings page.
 */
void presets_clear_from_phone(){
	mqueue_add("PRESET", "PRESET_CLEAR", " ");
}

/**
 * Function that saves the presets into persistent storage.
 */
void presets_save(void){
	int block = 0;
	uint8_t num_presets = presets_get_count();
	if (num_presets == 0){
		while (persist_exists(STORAGE_PRESET_START + block)){
			persist_delete(STORAGE_PRESET_START + block);
			block++;
		}
		return;
	}
	for (int i = 0; i < num_presets; i+= PRESET_BLOCK_SIZE){
		PresetBlock *presetBlock = malloc(sizeof(PresetBlock));
		presetBlock->count = num_presets;
		presetBlock->time = time(NULL);
		for (int j = 0; j < PRESET_BLOCK_SIZE; j++){
			if (i+j >= num_presets)
				break;
			presetBlock->presets[j] = *presets_get(i+j);
		}
		persist_write_data(STORAGE_PRESET_START+block, presetBlock, sizeof(PresetBlock));
		free(presetBlock);
		block++;
	}
	return;
}

/**
 * Function that decreases the ETAs of the presets by 1.
 * When a preset's ETA reaches 0, the bus will likely be there for
 * longer than a minute, so we wait 5 minutes before requesting a new
 * ETA from the phone.
 */
void decrement_etas(){
	for (int i = 0; i < presets_get_count(); i++){
		Preset *temp = presets_get(i);
		if (temp->eta > PRESET_REFRESHING_ETA)
			temp->eta -= 1;
		else if (temp->eta == PRESET_REFRESHING_ETA){
			send_eta_req(temp);
		}
	}
}

/**
 * Function to ask the phone for the ETAs of every preset.
 */
void send_all_eta_req(){
	mqueue_add("PRESET","PRESET_ETA", " ");
}

/**
 * Function to get the ETA of a specific preset from the phone.
 * @param preset Specific preset that will be sent to the phone.
 */
void send_eta_req(Preset *preset){
	if (preset->eta == PRESET_SENT_REQUEST)
		return;
	preset->eta = PRESET_SENT_REQUEST;
	char* data = malloc(21);
	snprintf(data, 21, "%s|%s", preset->stop_id, preset->route_id);
	mqueue_add("PRESET", "PRESET_ETA", data);
	free(data);
}

/**
 * Tick function that is called every minute.
 * @param tick_time     Time at which the tick even was triggered
 * @param units_changed Time interval at which the tick timer is triggered.
 */
static void tick_callback(struct tm *tick_time, TimeUnits units_changed)
{
	if (tick_time->tm_min % 10 == 0){
		send_all_eta_req();
	}
	else
	{
		decrement_etas();
	}
	refresh();
	update_time_text();
}

/**
 * Function to handle the messages returned from the phone.
 * Specifically the messages with the group code "PRESET"
 * @param operation "OPERATION" response of the given message. This will determine what data processor is called.
 * @param data      "DATA" response from the message.
 */
static void eta_handler(char* operation, char* data){
	if (strcmp(operation, "PRESET_ETA") == 0){
		process_eta_data(data);
	}
	else if (strcmp(operation, "PRESET_SET") == 0){
		process_preset_data(data);
	}
	else if (strcmp(operation, "PRESET_CLEAR") == 0){
		presets_clear();
	}
}

/**
 * Function to process the data of an ETA update message.
 * @param data the "DATA" response of the given method, which is formatted as "stop_id|route_id|eta"
 */
static void process_eta_data(char* data){
	data_processor_init(data, '|');
	char* stop_id = data_processor_get_string();
	char* route_id = data_processor_get_string();
	char* eta_string = data_processor_get_string();
	int eta = PRESET_NO_ETA;
	if (strcmp(eta_string, "NO ETA") != 0){
		eta = atoi(eta_string);
	}
	for (int i = 0; i < presets_get_count(); i++){
		Preset *preset = presets_get(i);
		if (strcmp(preset->stop_id, stop_id) == 0 && strcmp(preset->route_id, route_id) == 0){
			preset->eta = eta;
			break;
		}
	}
	free(stop_id);
	free(route_id);
	refresh();
	update_time_text();
}

/**
 * Function to process the data of a preset update.
 * @param data "DATA" response of the given message, formatted either
 * as "stop_id|stop_name|route_id|route_name", or as "END" to signify the end
 * of the preset messages.
 */
static void process_preset_data(char* data){
	if (strcmp(data, "END") != 0){
		data_processor_init(data, '|');
		Preset *preset = malloc(sizeof(Preset));
		strcpy(preset->stop_id, data_processor_get_string());
		strcpy(preset->stop_name, data_processor_get_string());
		strcpy(preset->route_id, data_processor_get_string());
		strcpy(preset->route_name, data_processor_get_string());
		preset->eta = -5;
		presets_add(preset);
	}
	else{
		send_all_eta_req();
		refresh();
		reset_selected_index();
		update_time_text();
	}
}