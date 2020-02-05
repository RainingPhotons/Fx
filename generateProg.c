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

/*
export LD_LIBRARY_PATH=/usr/local/lib64/
export PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig
gcc generateProg.c `pkg-config fluidsynth --libs` -lpthread -g
*/

pthread_mutex_t m_lock;
pthread_t m_thread;
int iActive = 0;

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

char* printNote(unsigned int note)
{
    int iBaseNote = note %12;
    switch(iBaseNote){
        case (0) :
            return "C";
        case (1):
            return "C#/Db";
        case (2) :
            return "D";
        case (3):
            return "D#/Eb";
        case (4) :
            return "E";
        case (5):
            return "F";
        case (6) :
            return "F#/Gb";
        case (7):
            return "G";
        case (8) :
            return "G#/Ab";
        case (9):
            return "A";
        case (10) :
            return "A#/Bb";
        case (11):
            return "B";
    }
}

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

void * readNotes(void* arg)
{
    pthread_mutex_lock(&m_lock);
    
    pthread_mutex_unlock(&m_lock);
    return NULL;
}

int playArp(int aChord[3], int iBeats, int iMeasureTime, int iBaseNote, int bMajor, int iSubGroup, int iOverRep, int iSGBeats,int *aiBeatsProg, int *aiScaleOffProg)
{
    int iAct = 0;
    pthread_mutex_lock(&m_lock);
    iAct = iActive;
    pthread_mutex_unlock(&m_lock);
    if(0 != iAct)
    {
        int iBeat;
        for(iBeat = 0; iBeat < iSubGroup; iBeat++)
        {
            aiBeatsProg[iBeat] =  iBeats/8;
            aiScaleOffProg[iBeat] += (rand() %3) - 1;
        }
        
        int bOctaveUp = 0;
        fluid_event_t *ev = new_fluid_event();
        fluid_event_set_source(ev, -1);
        fluid_event_set_dest(ev, synth_destination);

        printf("Arp, %s, %s, %s, iSubGroup, %d, iOverArp, %d\n", printNote(aChord[0]), printNote(aChord[1]), printNote(aChord[2]), iSubGroup, iOverRep);
        int iSubArp;
        int iOverArp;
        for(iOverArp = 0; iOverArp < iOverRep; iOverArp++)
        {
            for(iSubArp = 0; iSubArp < iSubGroup; iSubArp++)
            {
                int iNoteIdx = ((iOverArp * iSubGroup) + iSubArp + aiScaleOffProg[iSubArp]) % 8;
                int iModNote = ((bMajor == ectMajor) ? major_scale[iNoteIdx] : minor_scale[iNoteIdx]) + iBaseNote + (12 * bOctaveUp);
                fluid_event_noteon(ev, 4, iModNote, 127);
                fluid_sequencer_send_now(sequencer, ev);
                
                printf("OverArp, %d, SubArp, %d, iModNote, %d\n", iOverArp,iSubArp, iModNote);
                fluid_event_noteoff(ev, 4, iModNote);
                fluid_sequencer_send_now(sequencer, ev);
                usleep(aiBeatsProg[iSubArp] * 1000);
                if(iNoteIdx > 6)
                {
                    if(bOctaveUp < 2)
                        bOctaveUp ++;
                    else if (bOctaveUp > 0)
                        bOctaveUp--;
                }
            }
            usleep(iBeats * 500);
        }
    }
    return 0;
}

int playMelody(int aChord[3], int iBeats, int iMeasureTime, int iBaseNote, int bMajor, int iSubGroup, int iOverRep,int iSGBeats,int *aiBeatsProg, int *aiScaleOffProg)
{
    fluid_event_t *ev = new_fluid_event();
    fluid_event_set_source(ev, -1);
    fluid_event_set_dest(ev, synth_destination);

    int iNoteNum = 0;
    int iRand = rand();
    int iNote = aChord[0];
    int bChordNote = 1;
    unsigned int iMelTick = fluid_sequencer_get_tick(sequencer) + iBeats;
    unsigned int iStartTime = iMelTick;
    int bOctaveUpDown = 0;
    printf("melodyStart, %d, uMelodyEnd, %d, %s, %s, %s\n", iMelTick, iMeasureTime + iMelTick,printNote(aChord[0]), printNote(aChord[1]), printNote(aChord[2]));
    while(iMelTick < (iMeasureTime + iStartTime))
    {
        int iRand = rand();
        int iModNote = iNote;//Enable for certain instruments + (bOctaveUpDown * 12); //Ocate up or down
        fluid_event_noteon(ev, 4, iModNote, 127);
        fluid_sequencer_send_now(sequencer, ev);
        
        fluid_event_sustain(ev, 4, 127);
        fluid_sequencer_send_now(sequencer, ev);
        
        int iNoteWait = ((iBeats) * ((iRand % 4) + 0.5)); // wait some random amount of beat ;//+= (unsigned int)
        iMelTick += iNoteWait;
        iNoteNum++;
        printf("%d, %d, noteNum: %d, note: %s, tick, %d, %d\n", iNoteNum, iModNote, iNote,printNote(iModNote), iMelTick, bOctaveUpDown);
        
        playArp(aChord, iBeats, iMeasureTime, iBaseNote, bMajor, iSubGroup, iOverRep, iSGBeats,aiBeatsProg, aiScaleOffProg);
        
        usleep(iNoteWait * 1000);
        fluid_event_noteoff(ev, 4, iModNote);
        fluid_sequencer_send_now(sequencer, ev);
        
        if(iNote == aChord[0] || iNote == aChord[1] || iNote == aChord[2])
            iNote =  ((bMajor == ectMajor) ? major_scale[(iRand % 8)] : minor_scale[(iRand % 8)]) + iBaseNote;
        else
        {
            int bFound = 0;//Found the closest note
            int iBaseNote = iNote % 12;
            int iChordNote;
            for(iChordNote = 0; iChordNote < 3 && (!bFound); iChordNote++)
            {
                int bOver = abs((aChord[iChordNote] % 12) - iBaseNote);
                //printf("bOver, %d, ", bOver);
                if (bOver <=2 || bOver >= 10)
                {
                    iNote = aChord[iChordNote]; // Falling back on the chord note
                    bFound = 1;
                }
            }
            if(!bFound)
            {
                printf("Error\n");
            }
        }
        if((iNote % 12) >= 10 && (bOctaveUpDown < 1))
            bOctaveUpDown ++;
        else if (((iNote % 12) <= 2) && (bOctaveUpDown > 0))
            bOctaveUpDown --;
    }
    
    delete_fluid_event(ev);
    return iMelTick - iStartTime + iBeats;
}

void playChord(int aiBassChord[3], int aChord[3], int iPlayMelody, int iBeats, int iMeasureTime, int iBaseNote, int bMajor,int iSubGroup, int iOverRep,int iSGBeats,int *aiBeatsProg, int *aiScaleOffProg)
{
    
    int iChordNote;
    int64_t uWait =  iMeasureTime;
    fluid_event_t *ev = new_fluid_event();
    fluid_event_set_source(ev, -1);
    fluid_event_set_dest(ev, synth_destination);
    unsigned int time_marker = fluid_sequencer_get_tick(sequencer);

    for(iChordNote = 0; iChordNote < 3; iChordNote++)
    {
        fluid_event_noteon(ev, 0, aiBassChord[iChordNote], 32);//16 for violin 0-40
        fluid_sequencer_send_now(sequencer, ev);
//        printf("Base, %d,  %d\n", iNoteToplay, time_marker + iTimePlay);
        fluid_event_reverb_send(ev, 0, 127);
        fluid_sequencer_send_now(sequencer, ev);                
    }
    if(iPlayMelody)
        uWait = playMelody(aChord, iBeats, iMeasureTime, iBaseNote, bMajor, iSubGroup, iOverRep,iSGBeats,aiBeatsProg,  aiScaleOffProg);
    for(iChordNote = 0; iChordNote < 3; iChordNote++)
    {
        fluid_event_noteoff(ev, 0, aiBassChord[iChordNote]);
        fluid_sequencer_send_now(sequencer, ev);// + iTimePlay, 1);
    }
    delete_fluid_event(ev);
    time_marker = fluid_sequencer_get_tick(sequencer);
}

void startMusic()
{
    settings = new_fluid_settings();
    synth = new_fluid_synth(settings);
    
        //start thread
    if(0 != pthread_mutex_init(&m_lock,NULL))
    {
        printf("Mutex init failed\n");
    }
    if(0 != pthread_create(&m_thread, NULL, readNotes, NULL))
    {
        printf("Thread create error\n");
    }

    /* load a SoundFont */
    int n = fluid_synth_sfload(synth, "default2.sf2", 1);
    srand(time(NULL));

    unsigned int time_marker = 0;

    sequencer = new_fluid_sequencer();
    /* register the synth with the sequencer */
    synth_destination = fluid_sequencer_register_fluidsynth(sequencer, synth);
    /* register the client name and callback */
    
    //Setting up instruments
    int iSfontID, iBankNo, preset_num;
    int ret = fluid_synth_get_program(synth,0, &iSfontID, &iBankNo, &preset_num);
    ret = fluid_synth_program_select(synth, 0, iSfontID ,8, 63);//8-63:vol32,0-40:vol16,0-50:vol 50,0-69:16
    ret = fluid_synth_program_select(synth, 4, iSfontID ,0,46);//46, harp=best//0-72:vol 80, reverb, low bpm//0-98, crystal
    printf("Info: %d\n", ret);
    
    client_destination = fluid_sequencer_register_client(sequencer, "chordsAndStuff", NULL, NULL);

    audiodriver = new_fluid_audio_driver(settings, synth);

    int iSubGroup = (rand()%4) + 5; //Number of notes in this subgroup
    int iOverRep = (rand() % 3) + 2;
    int iSGBeats = iSubGroup;
    int *aiBeatsProg = malloc(sizeof(int) * iSubGroup);//BeatsForEachNote
    int *aiScaleOffProg = malloc(sizeof(int) * iSubGroup);//Offset from base note

    /* get the current time in ticks */
    time_marker = fluid_sequencer_get_tick(sequencer);
    int iMajMin = ectMajor;//rand() % 2;
    int aaChords[SCALE_NOTES][TRI_CHORD];
    int iKeyOf = 0;//Lowest C
    int iBaseNote = iKeyOf + MID_C;
    int iKeyNote;
    int iChordNote;
    char cInput = 0;
    if(ectMajor == iMajMin) //0 == major
    {
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
    }
    else if(ectMinor == iMajMin)
    {
        for(iKeyNote = 0; iKeyNote < SCALE_NOTES; iKeyNote++)
        {
            for(iChordNote = 0; iChordNote < 3; iChordNote++)
            {
                if(ectMajor == minor_chord_quality[iKeyNote] )
                {
                    aaChords[iKeyNote][iChordNote] = ((iKeyOf + minor_scale[iKeyNote])  + major_chord[iChordNote]) % 12;
                }
                 else if(ectMinor ==minor_chord_quality[iKeyNote] )
                {
                    aaChords[iKeyNote][iChordNote] = ((iKeyOf + minor_scale[iKeyNote])  + minor_chord[iChordNote]) % 12;
                }
                else
                {
                     aaChords[iKeyNote][iChordNote] = ((iKeyOf + minor_scale[iKeyNote])  + dimin_chord[iChordNote]) % 12;
                }
            }
        }
    }
    
    {
        //Tempo
        int iGrandRand = rand();
        int iBPM = 120 + ((iGrandRand % 4)* 10);
        int iBPMilli = 60000/iBPM;
        int iBPMeasure = ((rand() % 8) + 4) * 2; //Beats per measure At least 4 beats in a measure [4, 12]
        int iMsrMilli = iBPMeasure * iBPMilli; //Measure time in milliseconds
        
        int iNumChrdMvmt = ((iGrandRand % 5) * 3) + 8;
        
        printf("Init, iBPM: %d, BPMMilli: %d, BPMMeas %d, MeasMilli %d, iNumChrdMvmt: %d\n",iBPM,iBPMilli,iBPMeasure,iMsrMilli,iNumChrdMvmt);
        int *aiChordProg = malloc(sizeof(int) * iNumChrdMvmt);//{1-1, 3-1, 2-1, 6-1, 2-1, 5-1, 1-1};
        
        int iChord;
        int iPrevChord;
        int iPrevChordGroup = 0;
        aiChordProg[0] =  iPrevChord =1 - 1; //Always start with the root chord of the scale
        aiChordProg[iNumChrdMvmt - 1] = 1 -1; //Always end with the root chord of the scale 
        aiChordProg[iNumChrdMvmt - 2] = 5 - 1; // Always have the 5th chord of the scale right before the end
        
        //Chord grouping, number 9 = N/A, do not use 7th chord
        int aaiL[4][2] = {{1 - 1, 9}, {3 - 1, 6 - 1}, {2 - 1, 4 - 1}, {5 -1, 9}};
        //start at 1, and do not do last two chords, all three are already fixed above
        for(iChord = 1; iChord < iNumChrdMvmt - 2; iChord++)
        {
            int iNextGroup = ((iPrevChordGroup + 1) - ((rand() % 4) == 0)) % 4; //Sometimes, stay at the same chord
            //printf("iNextGroup, %d, %d\n", iChord, iNextGroup);
            if(aaiL[iNextGroup][1] == 9)
            {
                aiChordProg[iChord] = aaiL[iNextGroup][0];
            }
            else
            {
                aiChordProg[iChord] = aaiL[iNextGroup][rand() % 2];
            }
            iPrevChordGroup = iNextGroup;
        }
        
#if 1
    {
        printf("ChordProge, ");
        for(iChord = 0; iChord < iNumChrdMvmt; iChord++)
        {
            printf("%d, ",aiChordProg[iChord] + 1);
        }
        printf("\n");
    }
#endif
        
        int iIdx;
        
        int aiRChordPrev[3] = {0};
        int aiChordPrevA[3] = {0};
        int aiChordPrevB[3] = {0};
        
        int aiCreateArpgPattern[3] = {0};
        for(iIdx = 0; iIdx < 3; iIdx++)
        {
            aiCreateArpgPattern[iIdx] = rand() % 8;
        }
        
        for(iChord = 0; iChord < iNumChrdMvmt; iChord++)
        {
            int aiRootChord[3];
            int aiBassChord[3];
            int aiMelodyChord [3];
            
            aiRootChord[0] = aaChords[aiChordProg[iChord]][0] + iBaseNote;
            aiRootChord[1] = aaChords[aiChordProg[iChord]][1] + iBaseNote;
            aiRootChord[2] = aaChords[aiChordProg[iChord]][2] + iBaseNote;

            aiMelodyChord[0] = aiRootChord[0];
            aiMelodyChord[1] = aiRootChord[1];
            aiMelodyChord[2] = aiRootChord[2];

            aiBassChord[0] = aiRootChord[2] - 12;
            aiBassChord[1] = aiRootChord[0] - 12;
            aiBassChord[2] = aiRootChord[1] - 12;
            
            //playChord(aiBassChord, aiMelodyChord, 1, iBPMilli, iMsrMilli, iBaseNote, iMajMin,iSubGroup, iOverRep,iSGBeats,aiBeatsProg, aiScaleOffProg);
            playArp(aiMelodyChord, iBPMilli, iMsrMilli,iBaseNote, iMajMin,iSubGroup, iOverRep,iSGBeats,aiBeatsProg, aiScaleOffProg);
            
            memcpy(aiChordPrevA, aiMelodyChord, sizeof(int) * 3);
            memcpy(aiChordPrevB, aiBassChord, sizeof(int) * 3);
            memcpy(aiRChordPrev, aiRootChord, sizeof(int) * 3);
        }
        free(aiChordProg);
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
    free(aiScaleOffProg);
    free (aiBeatsProg);

}

int main(int argc, char *argv[])
{
    startMusic();
    return 1;
}