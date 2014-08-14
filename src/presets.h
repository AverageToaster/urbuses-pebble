#pragma once

#include "preset.h"

void presets_init(void);
void presets_deinit(void);
Preset* presets_get(int pos);
int presets_get_count();
void presets_clear();
void presets_add(Preset* preset);
void presets_remove(int pos);

void presets_restore(void);
void presets_save(void);

void send_eta_req(Preset *preset);