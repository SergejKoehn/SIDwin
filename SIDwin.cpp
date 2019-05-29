#include "stdio.h"
#include "stdafx.h"

#include "SID/SID.h"
#include "SID/Instrument.h"

using namespace System;

int main(array<System::String ^> ^args)
{
  SID sid;
  Instrument instrument;
  instrument.activate(&sid,0);
  instrument.noteOn(16);

  //Instrument::Control *ctrl= instrument.getControl(Instrument::Control::Type::Type_Frequency);
  //ctrl->switchLFO(1);
  //ctrl->getLFO()->create(Instrument::LFO::Type_Pulse,100,0,100);

  Instrument::LFO *lfo= new Instrument::LFO();
  lfo->create(Instrument::LFO::Type_Sinus,100,0,100);
  lfo->setFreq(1);

  for (unsigned int i = 0; i < 100; i++)
  {
    lfo->clock();
    int value= lfo->getValue();
    printf("%i\r\n",value);
  }

  char c;
  scanf("%c",&c);
  
  return 0;
}
