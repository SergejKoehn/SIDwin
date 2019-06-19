#pragma once
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include "DSound.h"

class CTestSound
{
protected:
  DSBUFFERDESC bufParams;
  LPDIRECTSOUNDBUFFER bufPtr;
  uint8_t *data1,*data2;
  uint32_t size1,size2;


public:
  void create(unsigned int bufSize);
  void lock();
  void release();
  inline void getBuffer1(unsigned char **data1, unsigned int &size1) { *data1= this->data1; size1= this->size1; }
  inline void getBuffer2(unsigned char **data2, unsigned int &size2) { *data2= this->data2; size2= this->size2; }
  void play();
  void stop();

  CTestSound();
  virtual ~CTestSound();
};
