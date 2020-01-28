#pragma once

#include <fluidsynth.h>

int initialize_sound();
void play_note(unsigned int note);
void destroy_sound();
