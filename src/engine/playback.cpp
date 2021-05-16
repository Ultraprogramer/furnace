#include "engine.h"

void DivEngine::nextOrder() {
  curRow=0;
  if (++curOrder>=song.ordersLen) {
    curOrder=0;
  }
}

const char* notes[12]={
  "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
};

const char* formatNote(unsigned char note, unsigned char octave) {
  static char ret[4];
  if (note==100) {
    return "OFF";
  } else if (octave==0 && note==0) {
    return "---";
  }
  snprintf(ret,4,"%s%d",notes[note%12],octave+note/12);
  return ret;
}

bool DivEngine::perSystemEffect(int ch, unsigned char effect, unsigned char effectVal) {
  switch (song.system) {
    case DIV_SYSTEM_GENESIS:
      switch (effect) {
        case 0x17: // DAC enable
          dispatch->dispatch(DivCommand(DIV_CMD_SAMPLE_MODE,ch,(effectVal>0)));
          break;
        case 0x20: // SN noise mode
          dispatch->dispatch(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        default:
          return false;
      }
      break;
    case DIV_SYSTEM_SMS:
      switch (effect) {
        case 0x20: // SN noise mode
          dispatch->dispatch(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        default:
          return false;
      }
      break;
    default:
      return false;
  }
  return true;
}

bool DivEngine::perSystemPostEffect(int ch, unsigned char effect, unsigned char effectVal) {
  switch (song.system) {
    case DIV_SYSTEM_GENESIS:
      switch (effect) {
        case 0x11: // FB
          dispatch->dispatch(DivCommand(DIV_CMD_FM_FB,ch,effectVal&7));
          break;
        case 0x12: // TL op1
          dispatch->dispatch(DivCommand(DIV_CMD_FM_TL,ch,0,effectVal&0x7f));
          break;
        case 0x13: // TL op2
          dispatch->dispatch(DivCommand(DIV_CMD_FM_TL,ch,1,effectVal&0x7f));
          break;
        case 0x14: // TL op3
          dispatch->dispatch(DivCommand(DIV_CMD_FM_TL,ch,2,effectVal&0x7f));
          break;
        case 0x15: // TL op4
          dispatch->dispatch(DivCommand(DIV_CMD_FM_TL,ch,3,effectVal&0x7f));
          break;
        case 0x16: // MULT
          if ((effectVal>>4)>0 && (effectVal>>4)<5) {
            dispatch->dispatch(DivCommand(DIV_CMD_FM_MULT,ch,(effectVal>>4)-1,effectVal&15));
          }
          break;
        case 0x18: // EXT
          dispatch->dispatch(DivCommand(DIV_CMD_FM_EXTCH,ch,effectVal));
          break;
        case 0x19: // AR global
          dispatch->dispatch(DivCommand(DIV_CMD_FM_AR,ch,-1,effectVal&31));
          break;
        case 0x1a: // AR op1
          dispatch->dispatch(DivCommand(DIV_CMD_FM_AR,ch,0,effectVal&31));
          break;
        case 0x1b: // AR op2
          dispatch->dispatch(DivCommand(DIV_CMD_FM_AR,ch,1,effectVal&31));
          break;
        case 0x1c: // AR op3
          dispatch->dispatch(DivCommand(DIV_CMD_FM_AR,ch,2,effectVal&31));
          break;
        case 0x1d: // AR op4
          dispatch->dispatch(DivCommand(DIV_CMD_FM_AR,ch,3,effectVal&31));
          break;
        default:
          return false;
      }
      break;
    default:
      return false;
  }
  return true;
}

void DivEngine::processRow(int i, bool afterDelay) {
  DivPattern* pat=song.pat[i]->data[curOrder];
  // pre effects
  if (!afterDelay) for (int j=0; j<song.pat[i]->effectRows; j++) {
    short effect=pat->data[curRow][4+(j<<1)];
    short effectVal=pat->data[curRow][5+(j<<1)];

    if (effectVal==-1) effectVal=0;
    if (effect==0xed) {
      chan[i].rowDelay=effectVal+1;
      return;
    }
  }

  // instrument
  if (pat->data[curRow][2]!=-1) {
    dispatch->dispatch(DivCommand(DIV_CMD_INSTRUMENT,i,pat->data[curRow][2]));
  }
  // note
  if (pat->data[curRow][0]==100) {
    chan[i].note=-1;
    dispatch->dispatch(DivCommand(DIV_CMD_NOTE_OFF,i));
  } else if (!(pat->data[curRow][0]==0 && pat->data[curRow][1]==0)) {
    chan[i].note=pat->data[curRow][0]+pat->data[curRow][1]*12;
    chan[i].doNote=true;
  }

  // volume
  if (pat->data[curRow][3]!=-1) {
    chan[i].volume=pat->data[curRow][3]<<8;
    dispatch->dispatch(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
  }

  // effects
  for (int j=0; j<song.pat[i]->effectRows; j++) {
    short effect=pat->data[curRow][4+(j<<1)];
    short effectVal=pat->data[curRow][5+(j<<1)];

    if (effectVal==-1) effectVal=0;

    // per-system effect
    if (!perSystemEffect(i,effect,effectVal)) switch (effect) {
      case 0x09: // speed 1
        song.speed1=effectVal;
        break;
      case 0x0f: // speed 2
        song.speed2=effectVal;
        break;
      case 0x0b: // change order
        changeOrd=effectVal;
        changePos=0;
        break;
      case 0x0d: // next order
        changeOrd=curOrder+1;
        changePos=effectVal;
        break;
      case 0x08: // panning
        dispatch->dispatch(DivCommand(DIV_CMD_PANNING,i,effectVal));
        break;
      case 0x01: // ramp up
        if (effectVal==0) {
          chan[i].portaNote=-1;
          chan[i].portaSpeed=-1;
        } else {
          chan[i].portaNote=0x60;
          chan[i].portaSpeed=effectVal;
        }
        break;
      case 0x02: // ramp down
        if (effectVal==0) {
          chan[i].portaNote=-1;
          chan[i].portaSpeed=-1;
        } else {
          chan[i].portaNote=0x00;
          chan[i].portaSpeed=effectVal;
        }
        break;
      case 0x03: // portamento
        if (effectVal==0) {
          chan[i].portaNote=-1;
          chan[i].portaSpeed=-1;
        } else {
          chan[i].portaNote=chan[i].note;
          chan[i].portaSpeed=effectVal;
          chan[i].doNote=false;
        }
        break;
      case 0x04: // vibrato
        chan[i].vibratoDepth=effectVal&15;
        chan[i].vibratoRate=effectVal>>4;
        dispatch->dispatch(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos])>>4)));
        break;
      case 0x0a: // volume ramp
        if (effectVal!=0) {
          if ((effectVal&15)!=0) {
            chan[i].volSpeed=-(effectVal&15)*64;
          } else {
            chan[i].volSpeed=(effectVal>>4)*64;
          }
        } else {
          chan[i].volSpeed=0;
        }
        break;
      case 0x00: // arpeggio
        chan[i].arp=effectVal;
        break;
      
      case 0xe1: // portamento up
        chan[i].portaNote=chan[i].note+(effectVal&15);
        chan[i].portaSpeed=(effectVal>>4)*3;
        break;
      case 0xe2: // portamento down
        chan[i].portaNote=chan[i].note-(effectVal&15);
        chan[i].portaSpeed=(effectVal>>4)*3;
        break;
      case 0xe5: // pitch
        chan[i].pitch=effectVal-0x80;
        dispatch->dispatch(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos])>>2)));
        break;
      case 0xea: // legato mode
        chan[i].legato=effectVal;
        break;
      case 0xec: // delayed note cut
        chan[i].cut=effectVal;
        break;
    }
  }

  if (chan[i].doNote) {
    chan[i].vibratoPos=0;
    dispatch->dispatch(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos])>>4)));
    if (chan[i].legato) {
      dispatch->dispatch(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
    } else {
      dispatch->dispatch(DivCommand(DIV_CMD_NOTE_ON,i,chan[i].note));
    }
    chan[i].doNote=false;
  }

  // post effects
  for (int j=0; j<song.pat[i]->effectRows; j++) {
    short effect=pat->data[curRow][4+(j<<1)];
    short effectVal=pat->data[curRow][5+(j<<1)];

    if (effectVal==-1) effectVal=0;
    perSystemPostEffect(i,effect,effectVal);
  }
}

void DivEngine::nextRow() {
  static char pb[4096];
  static char pb1[4096];
  static char pb2[4096];
  static char pb3[4096];
  if (++curRow>=song.patLen) {
    nextOrder();
  }
  if (changeOrd>=0) {
    curRow=changePos;
    curOrder=changeOrd;
    if (curOrder>=song.ordersLen) {
      curOrder=0;
    }
    changeOrd=-1;
  }
  strcpy(pb1,"");
  strcpy(pb3,"");
  for (int i=0; i<chans; i++) {
    snprintf(pb,4095," %.2x",song.orders.ord[i][curOrder]);
    strcat(pb1,pb);
    
    DivPattern* pat=song.pat[i]->data[curOrder];
    snprintf(pb2,4095,"\x1b[37m %s",
             formatNote(pat->data[curRow][0],pat->data[curRow][1]));
    strcat(pb3,pb2);
    if (pat->data[curRow][3]==-1) {
      strcat(pb3,"\x1b[m--");
    } else {
      snprintf(pb2,4095,"\x1b[1;32m%.2x",pat->data[curRow][3]);
      strcat(pb3,pb2);
    }
    if (pat->data[curRow][2]==-1) {
      strcat(pb3,"\x1b[m--");
    } else {
      snprintf(pb2,4095,"\x1b[0;36m%.2x",pat->data[curRow][2]);
      strcat(pb3,pb2);
    }
    for (int j=0; j<song.pat[i]->effectRows; j++) {
      if (pat->data[curRow][4+(j<<1)]==-1) {
        strcat(pb3,"\x1b[m--");
      } else {
        snprintf(pb2,4095,"\x1b[1;31m%.2x",pat->data[curRow][4+(j<<1)]);
        strcat(pb3,pb2);
      }
      if (pat->data[curRow][5+(j<<1)]==-1) {
        strcat(pb3,"\x1b[m--");
      } else {
        snprintf(pb2,4095,"\x1b[1;37m%.2x",pat->data[curRow][5+(j<<1)]);
        strcat(pb3,pb2);
      }
    }
  }
  printf("| %.2x:%s | \x1b[1;33m%3d%s\x1b[m\n",curOrder,pb1,curRow,pb3);

  for (int i=0; i<chans; i++) {
    processRow(i,false);
  }
}

void DivEngine::nextTick() {
  if (song.customTempo) {
    cycles=dispatch->rate/song.hz;
  } else {
    if (song.pal) {
      cycles=dispatch->rate/60;
    } else {
      cycles=dispatch->rate/50;
    }
  }
  if (--ticks<=0) {
    nextRow();
    if (speedAB) {
      ticks=song.speed2*(song.timeBase+1);
    } else {
      ticks=song.speed1*(song.timeBase+1);
    }
    speedAB=!speedAB;
  }
  // process stuff
  for (int i=0; i<chans; i++) {
    if (chan[i].rowDelay>0) {
      if (--chan[i].rowDelay==0) {
        processRow(i,true);
      }
    }
    if (chan[i].volSpeed!=0) {
      chan[i].volume+=chan[i].volSpeed;
      if (chan[i].volume>0x7f00) chan[i].volume=0x7f00;
      if (chan[i].volume<0) chan[i].volume=0;
      dispatch->dispatch(DivCommand(DIV_CMD_VOLUME,i,chan[i].volume>>8));
    }
    if (chan[i].vibratoDepth>0) {
      chan[i].vibratoPos+=chan[i].vibratoRate;
      if (chan[i].vibratoPos>=64) chan[i].vibratoPos-=64;
      dispatch->dispatch(DivCommand(DIV_CMD_PITCH,i,chan[i].pitch+((chan[i].vibratoDepth*vibTable[chan[i].vibratoPos])>>4)));
    }
    if (chan[i].portaSpeed>0) {
      if (dispatch->dispatch(DivCommand(DIV_CMD_NOTE_PORTA,i,chan[i].portaSpeed,chan[i].portaNote))==2) {
        chan[i].portaSpeed=0;
      }
    }
    if (chan[i].cut>0) {
      if (--chan[i].cut<1) {
        chan[i].note=-1;
        dispatch->dispatch(DivCommand(DIV_CMD_NOTE_OFF,i));
      }
    }
    if (chan[i].arp!=0) {
      chan[i].arpStage++;
      if (chan[i].arpStage>2) chan[i].arpStage=0;
      switch (chan[i].arpStage) {
        case 0:
          dispatch->dispatch(DivCommand(DIV_CMD_LEGATO,i,chan[i].note));
          break;
        case 1:
          dispatch->dispatch(DivCommand(DIV_CMD_LEGATO,i,chan[i].note+(chan[i].arp>>4)));
          break;
        case 2:
          dispatch->dispatch(DivCommand(DIV_CMD_LEGATO,i,chan[i].note+(chan[i].arp&15)));
          break;
      }
    }
  }

  // system tick
  dispatch->tick();
}

void DivEngine::nextBuf(float** in, float** out, int inChans, int outChans, unsigned int size) {
  size_t runtotal=blip_clocks_needed(bb[0],size);
  for (size_t i=0; i<runtotal; i++) {
    if (--cycles<=0) {
      nextTick();
    }
    dispatch->acquire(temp[0],temp[1]);

    blip_add_delta(bb[0],i,temp[0]-prevSample[0]);
    blip_add_delta(bb[1],i,temp[1]-prevSample[1]);
    prevSample[0]=temp[0];
    prevSample[1]=temp[1];
  }

  blip_end_frame(bb[0],runtotal);
  blip_end_frame(bb[1],runtotal);

  blip_read_samples(bb[0],bbOut[0],size,0);
  blip_read_samples(bb[1],bbOut[1],size,0);

  for (size_t i=0; i<size; i++) {
    out[0][i]=(float)bbOut[0][i]/16384.0;
    out[1][i]=(float)bbOut[1][i]/16384.0;
  }
}