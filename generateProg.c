#include <stdlib.h>
#include <stdio.h>
#include <fluidsynth.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define SCALE_NOTES 8
#define TRI_CHORD 3
#define NOTES_SCALE 12
#define MID_C (5 * NOTES_SCALE)
#define TICKS 1440*50 //TIme in ms
#define ACT_CHAN 7
                             
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
                                      11, //B
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


//SynthRelated
fluid_synth_t *synth;
fluid_audio_driver_t *audiodriver;
fluid_sequencer_t *sequencer;
short synth_destination, client_destination;

void playNow(unsigned int note)
{
    fluid_event_t *ev = new_fluid_event();
    unsigned int gT = fluid_sequencer_get_tick(sequencer);
//    int noteG = fluid_event_get_key(ev);
    printf("GetNote, %d\n", gT);
    fluid_event_set_source(ev, -1);
    fluid_event_set_dest(ev, synth_destination);
    fluid_event_noteon(ev, ACT_CHAN, note, 127);
    fluid_sequencer_send_now(sequencer, ev);
    fluid_event_sustain(ev, ACT_CHAN,127);
    fluid_sequencer_send_now(sequencer, ev);

    printf("now, %d\n", note);
    //fluid_sequencer_send_now(sequencer, ev);
    delete_fluid_event(ev);
}


void playChord(int aChord[3], unsigned int time_marker)
{
    int iChordNote;

    for(iChordNote = 0; iChordNote < 3; iChordNote++)
    {
        fluid_event_t *ev = new_fluid_event();
        fluid_event_set_source(ev, -1);
        fluid_event_set_dest(ev, synth_destination);
        
        fluid_event_noteon(ev, iChordNote, aChord[iChordNote], 127);
        fluid_sequencer_send_at(sequencer, ev, time_marker, 1);
        fluid_event_reverb_send(ev, iChordNote, 127);
        fluid_sequencer_send_at(sequencer, ev, time_marker, 1);

        fluid_event_noteoff(ev, iChordNote, aChord[iChordNote]);
        fluid_sequencer_send_at(sequencer, ev, time_marker + (4 * TICKS), 1);
        
        delete_fluid_event(ev);
    }
    printf("chord, %d, %d\n", time_marker,aChord[0]);

}

void playNotesAndChord2(int aChord[3], int bChord[3], int aSleep[3], int bSleep[3])
{
    int iArp1;
    fluid_event_t *ev = new_fluid_event();
    fluid_event_set_source(ev, -1);
    fluid_event_set_dest(ev, synth_destination);
    
    fluid_event_noteon(ev, 0, aChord[2], 127);
    fluid_sequencer_send_now(sequencer, ev);
    
    fluid_event_noteon(ev, 0, bChord[2], 127);
    fluid_sequencer_send_now(sequencer, ev);

    usleep(TICKS * aSleep[0]);
    
    for(iArp1 = 0; iArp1 < 3; iArp1++)
    {
        fluid_event_noteon(ev, 2, aChord[0], 127);
        fluid_sequencer_send_now(sequencer, ev);
        fluid_event_noteon(ev, 1, aChord[1], 127);
        fluid_sequencer_send_now(sequencer, ev);

        fluid_event_noteon(ev, 2, bChord[0], 127);
        fluid_sequencer_send_now(sequencer, ev);
        fluid_event_noteon(ev, 1, bChord[1], 127);
        fluid_sequencer_send_now(sequencer, ev);

        usleep(TICKS * aSleep[1]);
        
        fluid_event_noteon(ev, 2, aChord[0], 127);
        fluid_sequencer_send_now(sequencer, ev);
        
        fluid_event_noteoff(ev, 1, aChord[1]);
        fluid_sequencer_send_now(sequencer, ev);
        
        fluid_event_noteon(ev, 2, bChord[0], 127);
        fluid_sequencer_send_now(sequencer, ev);
        
        fluid_event_noteoff(ev, 1, bChord[1]);
        fluid_sequencer_send_now(sequencer, ev);
        usleep(TICKS * aSleep[2]);
    }
    fluid_event_noteoff(ev, 0, aChord[2]);
    fluid_sequencer_send_now(sequencer, ev);
    
    fluid_event_noteoff(ev, 0, bChord[2]);
    fluid_sequencer_send_now(sequencer, ev);
    {
        int iNote;
        printf("A, ");
        for(iNote = 0; iNote < 3; iNote++)
        {
              printf("%d, ", aChord[iNote]);
        }
        printf("\n");
        
        printf("B, ");
        for(iNote = 0; iNote < 3; iNote++)
        {
              printf("%d, ", aChord[iNote]);
        }
        printf("\n");

    }
    delete_fluid_event(ev);    
}

void findCommonTones(int aiChordNew[3], int aiChordOld[3])
{
    int aiTemp[3];
    memcpy(aiTemp, aiChordNew, 3 * sizeof(int));
    
}

int main(int argc, char *argv[])
{
    fluid_settings_t *settings;
    settings = new_fluid_settings();
    synth = new_fluid_synth(settings);
        /* load a SoundFont */
    int n = fluid_synth_sfload(synth, "default.sf2", 1);
    
    unsigned int time_marker = 0;

    sequencer = new_fluid_sequencer();
    /* register the synth with the sequencer */
    synth_destination = fluid_sequencer_register_fluidsynth(sequencer, synth);
    /* register the client name and callback */
    
    //Setting up instruments
    int iSfontID, iBankNo, preset_num;
    int ret = fluid_synth_get_program(synth,0, &iSfontID, &iBankNo, &preset_num);
    ret = fluid_synth_program_select(synth, 0, iSfontID ,iBankNo, 0);
    ret = fluid_synth_program_select(synth, 1, iSfontID ,iBankNo, 0);
    ret = fluid_synth_program_select(synth, 2, iSfontID ,iBankNo, 0);
    ret = fluid_synth_program_select(synth, 3, iSfontID ,iBankNo, 3);
    ret = fluid_synth_program_select(synth, 4, iSfontID ,iBankNo, 3);
    ret = fluid_synth_program_select(synth, 7, iSfontID ,0, 3);
    printf("Info: %d\n", ret);
    
    client_destination = fluid_sequencer_register_client(sequencer, "chords", NULL, NULL);

    audiodriver = new_fluid_audio_driver(settings, synth);

    /* get the current time in ticks */
    time_marker = fluid_sequencer_get_tick(sequencer);
    
    int aaChords[SCALE_NOTES][TRI_CHORD];
    int iKeyOf = 0;//Lowest C
    int iKeyNote;
    int iChordNote;
    char cInput = 0;
    for(iKeyNote = 0; iKeyNote < SCALE_NOTES; iKeyNote++)
    {
        for(iChordNote = 0; iChordNote < 3; iChordNote++)
        {
            if(ectMajor == major_chord_quality[iKeyNote] )
            {
                aaChords[iKeyNote][iChordNote] = ((iKeyOf + major_scale[iKeyNote])  + major_chord[iChordNote]) % 12;
            }
             else if(ectMinor ==major_chord_quality[iKeyNote] )
            {
                aaChords[iKeyNote][iChordNote] = ((iKeyOf + major_scale[iKeyNote])  + minor_chord[iChordNote]) % 12;
            }
            else
            {
                 aaChords[iKeyNote][iChordNote] = ((iKeyOf + major_scale[iKeyNote])  + dimin_chord[iChordNote]) % 12;
            }
        }
    }
    
    {
        int iChordProg[] = {1-1, 3-1, 2-1, 6-1, 2-1, 5-1, 1-1};
        int iChord;
        int iIdx;
        
        int aiRChordPrev[3] = {0};
        int aiChordPrevA[3] = {0};
        int aiChordPrevB[3] = {0};
        
        int aiCreateArpgPattern[3] = {0};
        srand(time(NULL));
        for(iIdx = 0; iIdx < 3; iIdx++)
        {
            aiCreateArpgPattern[iIdx] = rand() % 8;
        }
        
        for(iChord = 0; iChord < 7; iChord++)
        {
            int aiRootChord[3];
            int aiBassChord[3];
            int aiMelodyChord [3];
            
            aiRootChord[0] = aaChords[iChordProg[iChord]][0] + MID_C;
            aiRootChord[1] = aaChords[iChordProg[iChord]][1] + MID_C;
            aiRootChord[2] = aaChords[iChordProg[iChord]][2] + MID_C;
                        
            aiMelodyChord[0] = aiRootChord[0];
            aiMelodyChord[1] = aiRootChord[1];
            aiMelodyChord[2] = aiRootChord[2];

            aiBassChord[0] = aiRootChord[2] - 12;
            aiBassChord[1] = aiRootChord[0] - 12;
            aiBassChord[2] = aiRootChord[1] - 12;
            
            playNotesAndChord2(aiMelodyChord, aiBassChord, aiCreateArpgPattern, aiCreateArpgPattern);
            playNotesAndChord2(aiMelodyChord, aiBassChord, aiCreateArpgPattern, aiCreateArpgPattern);
            
            memcpy(aiChordPrevA, aiMelodyChord, sizeof(int) * 3);
            memcpy(aiChordPrevB, aiBassChord, sizeof(int) * 3);
            memcpy(aiRChordPrev, aiRootChord, sizeof(int) * 3);
        }
        time_marker = fluid_sequencer_get_tick(sequencer);
        usleep(TICKS);
        playChord(aiChordPrevA, time_marker);
        playChord(aiChordPrevB, time_marker);
    }
    
    while('q' != cInput)
    {
        cInput = getchar();
        if(cInput && ('\n' != cInput))
        {
            playNow(60);
        }
    }
    delete_fluid_audio_driver(audiodriver);
    delete_fluid_sequencer(sequencer);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return 1;
}