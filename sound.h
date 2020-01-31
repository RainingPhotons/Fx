#pragma once

#include <fluidsynth.h>

int initialize_sound(const char *sf_file);
void play_note(unsigned int note);
void destroy_sound();
