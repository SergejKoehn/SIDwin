#include <memory.h>
#include <stdlib.h>

#include "TestSID.h"

unsigned char TestSID::sidorder[] =
  {0x15,0x16,0x18,0x17,
   0x02,0x03,0x06,0x05,0x00,0x01,0x04,
   0x09,0x0a,0x0d,0x0c,0x07,0x08,0x0b,
   0x10,0x11,0x14,0x13,0x0e,0x0f,0x12};

unsigned char TestSID::altsidorder[] =
  {0x15,0x16,0x18,0x17,
   0x04,0x00,0x01,0x02,0x03,0x05,0x06,
   0x0b,0x07,0x08,0x09,0x0a,0x0c,0x0d,
   0x12,0x0e,0x0f,0x10,0x11,0x13,0x14};


TestSID::FILTERPARAMS TestSID::filterparams =
  {0.50f, 3.3e6f, 1.0e-4f,
   1147036.4394268463f, 274228796.97550374f, 1.0066634233403395f, 16125.154840564108f,
   5.5f, 20.f,
   0.9613160610660189f};

//extern unsigned residdelay;
//extern unsigned adparam;

unsigned TestSID::residdelay = 0;
unsigned TestSID::adparam    = 0x0f00;

TestSID::TestSID()
{
  sid= 0;
  sidfp= 0;
  memset(sidreg,0,NUMSIDREGS);

  instrument[0].activate(this,0);
  instrument[1].activate(this,1);
  instrument[2].activate(this,2);
}

void TestSID::init(int speed, unsigned model, unsigned ntsc, unsigned interpolate, unsigned customclockrate, unsigned usefp)
{
  int c;

  if (ntsc) clockrate = NTSCCLOCKRATE;
    else clockrate = PALCLOCKRATE;

  if (customclockrate)
    clockrate = customclockrate;

  samplerate = speed;

  if (!usefp)
  {
    if (sidfp)
    {
      delete sidfp;
      sidfp = NULL;
    }

    if (!sid) sid = new SIDC64;
  }
  else
  {
    if (sid)
    {
      delete sid;
      sid = NULL;
    }
    
    if (!sidfp) sidfp = new SIDFP;
  }

  switch(interpolate)
  {
    case 0:
    if (sid) sid->set_sampling_parameters(clockrate, SAMPLE_FAST, speed);
    if (sidfp) sidfp->set_sampling_parameters(clockrate, SAMPLE_INTERPOLATE, speed);
    break;

    default:
    if (sid) sid->set_sampling_parameters(clockrate, SAMPLE_INTERPOLATE, speed);
    if (sidfp) sidfp->set_sampling_parameters(clockrate, SAMPLE_RESAMPLE_INTERPOLATE, speed);
    break;
  }

  if (sid) sid->reset();
  if (sidfp) sidfp->reset();
  for (c = 0; c < NUMSIDREGS; c++)
  {
    sidreg[c] = 0x00;
  }
  if (model == 1)
  {
    if (sid) sid->set_chip_model(MOS8580);
    if (sidfp) sidfp->set_chip_model(MOS8580);
  }
  else
  {
    if (sid) sid->set_chip_model(MOS6581);
    if (sidfp) sidfp->set_chip_model(MOS6581);
  }

  if (sidfp)
  {
    sidfp->get_filter().set_distortion_properties(
      filterparams.distortionrate,
      filterparams.distortionpoint,
      filterparams.distortioncfthreshold);
    sidfp->get_filter().set_type3_properties(
      filterparams.type3baseresistance,
      filterparams.type3offset,
      filterparams.type3steepness,
      filterparams.type3minimumfetresistance);
    sidfp->get_filter().set_type4_properties(
      filterparams.type4k,
      filterparams.type4b);
    sidfp->set_voice_nonlinearity(
      filterparams.voicenonlinearity);
  }
}

unsigned char TestSID::getOrder(unsigned char index)
{
  if (adparam >= 0xf000)
    return altsidorder[index];
  else
    return sidorder[index];
}

void TestSID::writeRegister(unsigned char address, unsigned char value)
{
  sidreg[address]= value;
}

void TestSID::clock(int cycleCount)
{
  for( int i = 0; i < cycleCount; i++)
  {
    instrument[0].clock();
    instrument[1].clock();
    instrument[2].clock();
  }
}

int TestSID::fillBuffer(short *ptr, int samples)
{
  int tdelta;
  int tdelta2;
  int result = 0;
  int total = 0;
  int c;

  int badline = rand() % NUMSIDREGS;

  tdelta = clockrate * samples / samplerate;
  if (tdelta <= 0) return total;

  for (c = 0; c < NUMSIDREGS; c++)
  {
    unsigned char o = getOrder(c);

  	// Extra delay for loading the waveform (and mt_chngate,x)
  	if ((o == 4) || (o == 11) || (o == 18))
  	{
  	  tdelta2 = SIDWAVEDELAY;
      clock(tdelta2);
      if (sid) result = sid->clock(tdelta2, ptr, samples);
      if (sidfp) result = sidfp->clock(tdelta2, ptr, samples);
      total += result;
      ptr += result;
      samples -= result;
      tdelta -= SIDWAVEDELAY;
    }

    // Possible random badline delay once per writing
    if ((badline == c) && (residdelay))
  	{
      tdelta2 = residdelay;
      clock(tdelta2);
      if (sid) result = sid->clock(tdelta2, ptr, samples);
      if (sidfp) result = sidfp->clock(tdelta2, ptr, samples);
      total += result;
      ptr += result;
      samples -= result;
      tdelta -= residdelay;
    }

    if (sid) sid->write(o, sidreg[o]);
    if (sidfp) sidfp->write(o, sidreg[o]);

    tdelta2 = SIDWRITEDELAY;
    clock(tdelta2);
    if (sid) result = sid->clock(tdelta2, ptr, samples);
    if (sidfp) result = sidfp->clock(tdelta2, ptr, samples);
    total += result;
    ptr += result;
    samples -= result;
    tdelta -= SIDWRITEDELAY;

    if (tdelta <= 0) return total;
  }

  clock(tdelta);
  if (sid) result = sid->clock(tdelta, ptr, samples);
  if (sidfp) result = sidfp->clock(tdelta, ptr, samples);
  total += result;
  ptr += result;
  samples -= result;

  // Loop extra cycles until all samples produced
  while (samples)
  {
    tdelta = clockrate * samples / samplerate;
    if (tdelta <= 0) return total;

    //for (int i = 0; i < tdelta; i++)
    //{
    //  clock(1);
    //  if (sid) 
    //  {
    //    sid->clock();
    //    ptr[0]= sid->output();
    //  }

    //  if (sidfp) 
    //  {
    //    sidfp->clock();
    //    ptr[0]= sidfp->output();
    //  }
    //}

    //total += 1;
    //ptr += 1;
    //samples -= 1;

    clock(tdelta);
    if (sid) result = sid->clock(tdelta, ptr, samples);
    if (sidfp) result = sidfp->clock(tdelta, ptr, samples);
    total += result;
    ptr += result;
    samples -= result;
  }

  return total;
}