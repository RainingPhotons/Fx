#ifndef generateProg_h
#define generateProg_h
#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>
#include <fluidsynth.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>

#define SCALE_NOTES 8
#define TRI_CHORD 3
#define NOTES_SCALE 12
#define MID_C (5 * NOTES_SCALE)
#define ACT_CHAN 7
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))


void readEvents(int iStrength);
void startMusic();
#ifdef __cplusplus
     }
#endif
#endif
