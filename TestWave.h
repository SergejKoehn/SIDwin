#pragma once
#include "Windows.h"
#include "stdio.h"
#include "stdafx.h"
#include <mmsystem.h>
#include <amstream.h>

#include "TestSID.h"

#define BUFSIZE 256

class TestWave
{
public:
  HWAVEOUT     waveOut;
  WAVEHDR      waveHdr;
  SIDC64      *sid;
  float        buffer[BUFSIZE];
  Instrument::LFO lfo;

public:
  void create();
  void playNext();
  static void CALLBACK TestWave::waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
  HRESULT RenderStreamToDevice(IMultiMediaStream *pMMStream);
  TestWave(SIDC64 *sid);
};