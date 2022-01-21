#include "engine.h"
#include "wavetable.h"
#include "../ta-log.h"
#include "../fileutils.h"

void DivWavetable::putWaveData(SafeWriter* w) {
  w->write("WAVE",4);
  w->writeI(0);

  w->writeC(0); // name
  w->writeI(len);
  w->writeI(min);
  w->writeI(max);
  for (int j=0; j<len; j++) {
    w->writeI(data[j]);
  }
}

void DivWavetable::readWaveData(SafeReader& reader, short version) {
  reader.readI();
  reader.readI();

  reader.readString(); // ignored for now
  len=reader.readI();
  min=reader.readI();
  max=reader.readI();
  reader.read(data,4*len);
}

bool DivWavetable::save(const char* path) {
  SafeWriter* w=new SafeWriter();
  w->init();

  // write magic
  w->write("-Furnace waveta-",16);

  // write version
  w->writeS(DIV_ENGINE_VERSION);

  // reserved
  w->writeS(0);

  putWaveData(w);

  FILE* outFile=ps_fopen(path,"wb");
  if (outFile==NULL) {
    logE("could not save wavetable: %s!\n",strerror(errno));
    w->finish();
    return false;
  }
  if (fwrite(w->getFinalBuf(),1,w->size(),outFile)!=w->size()) {
    logW("did not write entire wavetable!\n");
  }
  fclose(outFile);
  w->finish();
  return true;
}
