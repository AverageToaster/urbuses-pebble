#include <pebble.h>
#include "libs/linked-list/linked-list.h"

#define STORAGE_PRESET_START 2
#define PRESET_BLOCK_SIZE 5

typedef struct PresetBlock {
	Preset presets[PRESET_BLOCK_SIZE];
	uint8_t count;
} PresetBlock;

static LinkedRoot* presets = NULL;

void presets_init(void){
	mqueue_register_handler("ETA", eta_message_handler);
	presets = linked_list_create_root();
}

Preset* presets_get(int pos){
	return (Preset*) linked_list_get(presets, pos);
}

int presets_get_count(){
	return linked_list_count(presets);
}

void presets_add(Preset* preset){
	linked_list_append(presets, preset);
}

void presets_clear(){
	while (presets_get_count() > 0){
		presets_remove(0);
	}
}

void presets_remove(int pos){
	Preset* preset = presets_get(pos);
	free(preset);
	linked_list_remove(presets, pos);
}

void presets_restore(void){
	presets_clear();

	if (!persist_exists(STORAGE_PRESET_START)){
		return 0;
	}

	int block = 0;
	PresetBlock* presetBlock = malloc(sizeof(PresetBlock));
	persist_read_data(STORAGE_PRESET_START, presetBlock, sizeOf(PresetBlock));

	uint8_t preset_count = presetBlock->count;

	for (int i = 0; i < preset_count; i++){
		if (i > 0 && i % PRESET_BLOCK_SIZE == 0){
			block += 1;
			free(presetBlock);
			presetBlock = malloc(sizeof(PresetBlock));
			persist_read_data(STORAGE_PRESET_START + block, presetBlock, sizeof(PresetBlock));
		}
		Preset *preset = preset_clone(&presetBlock->presets[i % PRESET_BLOCK_SIZE]);
		presets_add(preset);
	}
	free(presetBlock);
	return 0;
}

void presets_save(void){
	int block = 0;
	uint8_t num_presets = presets_get_count();
	if (num_presets == 0){
		persist_delete(STORAGE_PRESET_START);
		return 0;
	}
	for (int i = 0; i < num_presets; i+= PRESET_BLOCK_SIZE){
		PresetBlock *presetBlock = malloc(sizeof(PresetBlock));
		presetBlock->count = num_presets;
		for (int j = 0; j < PRESET_BLOCK_SIZE; j++){
			if (i+j >= num_presets)
				break;
			presetBlock->presets[j] = *presets_get(i+j);
		}
		persist_write_data(STORAGE_PRESET_START+block, presetBlock, sizeof(PresetBlock));
		free(presetBlock);
		block++;
	}
	return 0;
}
