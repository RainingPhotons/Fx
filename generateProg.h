#ifndef generateProg_h
#define generateProg_h

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

pthread_mutex_t m_lock;
int m_iActive = 0;

//SynthRelated
fluid_synth_t *synth;
fluid_audio_driver_t *audiodriver;
fluid_sequencer_t *sequencer;
short synth_destination, client_destination;
fluid_settings_t *settings;

                             
enum eChordTyoes  {
    ectMajor = 0,
    ectMinor = 1,
    ectDim = 2,
};
//Based on C Major                             
unsigned int major_scale[] = {
                                      0, //C
                                      2, //D
                                      4, //E
                                      5, //F
                                      7, //G
                                      9, //A
                                      11,//B
                                      12,//C
                                      } ;

//Based on A minor
unsigned int minor_scale[] = { 
                                      0, //A
                                      2, //B
                                      3, //C
                                      5, //D
                                      7, //E
                                      8, //F
                                      10, //G
                                      12, //A
                                      };

//0 = major, 1 = Minor, 2 = Dimnished
unsigned int major_chord_quality[] = {ectMajor,ectMinor,ectMinor,ectMajor,ectMajor,ectMinor,ectDim,ectMajor};
unsigned int minor_chord_quality[] = {ectMinor,ectDim,ectMajor,ectMinor,ectMinor,ectMajor,ectMajor,ectMinor};


unsigned int major_chord[] = {0, 4, 7};
unsigned int minor_chord[] = {0, 3, 7};
unsigned int dimin_chord[]  = {0, 3, 6};

void readEvents(int iStrength);
void startMusic();

#endif