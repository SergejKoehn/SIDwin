#pragma once

#include "GT/resid/sid.h"
#include "GT/resid-fp/sidfp.h"

#include "SID/SIDIF.h"
#include "SID/Instrument.h"

#define NUMSIDREGS 0x19
#define SIDWRITEDELAY 9 // lda $xxxx,x 4 cycles, sta $d400,x 5 cycles
#define SIDWAVEDELAY 4 // and $xxxx,x 4 cycles extra

#define PALFRAMERATE 50
#define PALCLOCKRATE 985248
#define NTSCFRAMERATE 60
#define NTSCCLOCKRATE 1022727

class TestSID : public SIDIF
{
public:

typedef struct
{
  float distortionrate;
  float distortionpoint;
  float distortioncfthreshold;
  float type3baseresistance;
  float type3offset;
  float type3steepness;
  float type3minimumfetresistance;
  float type4k;
  float type4b;
  float voicenonlinearity;
} FILTERPARAMS;


public:
  static unsigned char sidorder[];
  static unsigned char altsidorder[];
  static FILTERPARAMS  filterparams;
  static unsigned      residdelay;
  static unsigned      adparam;

  Instrument instrument[3];

protected:
  int clockrate;
  int samplerate;

  unsigned char sidreg[NUMSIDREGS];

  SIDC64 *sid;
  SIDFP  *sidfp;

public:
  TestSID();
  
  void clock(int cycleCount);

  void init(int speed, unsigned model, unsigned ntsc, unsigned interpolate, unsigned customclockrate, unsigned usefp);
  unsigned char getOrder(unsigned char index);

  virtual void writeRegister(unsigned char address, unsigned char value);
  virtual int  fillBuffer(short *ptr, int samples);
};
