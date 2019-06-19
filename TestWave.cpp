#include "TestWave.h"

TestWave::TestWave(SIDC64 *sid)
{
  this->sid= sid;
  lfo.create(Instrument::LFO::Type_Sinus,65536,0,44100);
  lfo.setFreq(1000);

  for ( unsigned int i= 0; i < BUFSIZE; i++ )
  {
    lfo.clock();
    printf("%i\r\n",lfo.getValue());
  }
}

void CALLBACK TestWave::waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
  if ( uMsg == WOM_DONE )
  {
    TestWave *inst= (TestWave *)dwInstance;
    inst->playNext();
  }
}

void TestWave::playNext()
{
  for ( unsigned int i= 0; i < BUFSIZE; i++ )
  {
    //sid->clock();
    //buffer[i]= sid->output();
    lfo.clock();
    buffer[i]= (float)lfo.getValue();
  }

  waveOutPrepareHeader(waveOut,&waveHdr,sizeof(WAVEHDR));
  waveOutWrite(waveOut,&waveHdr,sizeof(WAVEHDR));
}

void TestWave::create()
{
  WAVEFORMATEX format;

  memset(&format,0,sizeof(WAVEFORMATEX));

  format.wFormatTag     = WAVE_FORMAT_PCM;
  format.nChannels      = 1;
  format.wBitsPerSample = 16;
  format.nSamplesPerSec = 44100;
  format.nBlockAlign    = format.nChannels * format.wBitsPerSample / 8;
  format.nAvgBytesPerSec= format.nBlockAlign * format.nSamplesPerSec ;   
  format.cbSize         = 0;

  waveOutOpen(&waveOut,WAVE_MAPPER,&format,(DWORD_PTR)&waveOutProc,(DWORD_PTR)this,CALLBACK_FUNCTION);
  
  memset(&waveHdr,0,sizeof(WAVEHDR));

  waveHdr.lpData= (LPSTR)&buffer;
  waveHdr.dwBufferLength= sizeof(buffer);
  waveHdr.dwBytesRecorded= 0;
  waveHdr.dwUser= 0;
  waveHdr.dwFlags= WHDR_ENDLOOP;
  waveHdr.dwLoops= 1;
  waveHdr.lpNext= 0;
  waveHdr.reserved= 0;
}