#ifndef _ENGINE_H
#define _ENGINE_H
#include "song.h"
#include "dispatch.h"
#include "../audio/taAudio.h"
#include "blip_buf.h"

struct DivChannelState {
  std::vector<DivDelayedCommand> delayed;
  int note, pitch, portaSpeed, portaNote;
  int volume, volSpeed, cut, rowDelay;
  int vibratoDepth, vibratoRate, vibratoPos;
  int tremoloDepth, tremoloRate, tremoloPos;
  unsigned char arp, arpStage;
  bool doNote, legato;

  DivChannelState():
    note(-1),
    pitch(0),
    portaSpeed(-1),
    portaNote(-1),
    volume(0x7f00),
    volSpeed(0),
    cut(-1),
    rowDelay(0),
    vibratoDepth(0),
    vibratoRate(0),
    vibratoPos(0),
    tremoloDepth(0),
    tremoloRate(0),
    tremoloPos(0),
    arp(0),
    arpStage(-1),
    doNote(false), legato(false) {}
};

class DivEngine {
  DivDispatch* dispatch;
  TAAudio* output;
  TAAudioDesc want, got;
  int chans;
  bool playing;
  bool speedAB;
  int ticks, cycles, curRow, curOrder;
  int changeOrd, changePos;
  DivChannelState chan[17];

  short vibTable[64];

  blip_buffer_t* bb[2];
  int temp[2], prevSample[2];
  short* bbOut[2];

  void processRow(int i, bool afterDelay);
  void nextOrder();
  void nextRow();
  void nextTick();
  bool perSystemEffect(int ch, unsigned char effect, unsigned char effectVal);
  bool perSystemPostEffect(int ch, unsigned char effect, unsigned char effectVal);
  void renderSamples();

  public:
    DivSong song;
    void nextBuf(float** in, float** out, int inChans, int outChans, unsigned int size);
    // load a .dmf.
    bool load(void* f, size_t length);
    // save as .dmf.
    bool save(FILE* f);

    // play
    void play();

    // initialize the engine.
    bool init();

    DivEngine():
      chans(0),
      playing(false),
      speedAB(false),
      ticks(0),
      cycles(0),
      curRow(-1),
      curOrder(0),
      changeOrd(-1),
      changePos(0),
      temp{0,0},
      prevSample{0,0} {}
};
#endif