#include <fluidsynth.h>

#define ACT_CHAN 0

static fluid_synth_t *synth = 0;
static fluid_audio_driver_t *audiodriver = 0;
static fluid_sequencer_t *sequencer = 0;
static fluid_settings_t *settings;
static short synth_destination, client_destination;

int initialize_sound() {
  settings = new_fluid_settings();
  synth = new_fluid_synth(settings);
      /* load a SoundFont */
  int n = fluid_synth_sfload(synth, "default.sf2", 1);

  sequencer = new_fluid_sequencer();
  /* register the synth with the sequencer */
  synth_destination = fluid_sequencer_register_fluidsynth(sequencer, synth);
  /* register the client name and callback */

  //Setting up instruments
  int iSfontID, iBankNo, preset_num;
  int ret = fluid_synth_get_program(synth,0, &iSfontID, &iBankNo, &preset_num);        
  ret = fluid_synth_program_select(synth, 0, iSfontID ,iBankNo, 0);
  ret = fluid_synth_program_select(synth, 1, iSfontID ,iBankNo, 24);
  ret = fluid_synth_program_select(synth, 2, iSfontID ,iBankNo, 48);
  ret = fluid_synth_program_select(synth, 3, iSfontID ,iBankNo, 3);
  ret = fluid_synth_program_select(synth, 4, iSfontID ,iBankNo, 3);
  ret = fluid_synth_program_select(synth, 7, iSfontID ,1, 123);
  printf("Info: %d\n", ret);

  client_destination = fluid_sequencer_register_client(sequencer, "chords", NULL, NULL);

  audiodriver = new_fluid_audio_driver(settings, synth);

  return 1;
}

void play_note(unsigned int note) {
  fluid_event_t *ev = new_fluid_event();
  unsigned int gT = fluid_sequencer_get_tick(sequencer);
  printf("GetNote, %d\n", gT);
  fluid_event_set_source(ev, -1);
  fluid_event_set_dest(ev, synth_destination);
  fluid_event_noteon(ev, ACT_CHAN, note, 127);
  fluid_sequencer_send_now(sequencer, ev);
  fluid_event_sustain(ev, ACT_CHAN,127);
  fluid_sequencer_send_now(sequencer, ev);

  delete_fluid_event(ev);
}

void destroy_sound() {
  delete_fluid_audio_driver(audiodriver);
  delete_fluid_sequencer(sequencer);
  delete_fluid_synth(synth);
  delete_fluid_settings(settings);
}
