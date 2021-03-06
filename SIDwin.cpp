#include "stdio.h"
#include "stdafx.h"

#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>
#include <amstream.h>

//#include "wave/file.h"
//#include "TestWave.h"
//#include "TestSound.h"
#include "SampleWave.h"

//#include "TestSID.h"

// using namespace System;


void test()
{
  IMultiMediaStream *pMMStream;

  CoInitialize(NULL);
  HRESULT hr = RenderFileToMMStream("sin_500Hz.wav", &pMMStream);
  if (SUCCEEDED(hr))
  {
      RenderStreamToDevice(pMMStream);
      pMMStream->Release();
  }
    
  CoUninitialize();
}

int main()
{
  testSID();
  //RenderBufferToDevice();
  //test();
  return 0;


  //TestWave wav(&t.sid);
  //wav.create();
  //wav.playNext();

  WAVEFORMATEX format;

  memset(&format,0,sizeof(WAVEFORMATEX));

  format.wFormatTag     = WAVE_FORMAT_PCM;
  format.nChannels      = 1;
  format.wBitsPerSample = 16;
  format.nSamplesPerSec = 44100;
  format.nBlockAlign    = format.nChannels * format.wBitsPerSample / 8;
  format.nAvgBytesPerSec= format.nBlockAlign * format.nSamplesPerSec ;   
  format.cbSize         = 0;

  CWaveOut WaveOut(&format, 4, 2048);
  
  BOOL once= FALSE;

  //Instrument::LFO lfo;
  //lfo.create(Instrument::LFO::Type_Sinus,1000,0,44100);
  //lfo.setFreq(800);

  while ( true )
  {
    
    short data[1024];

    for ( unsigned int i= 0; i < 1024; i++ )
    {
      //t.sid.clock();
      //data[i]= t.sid.output();

      //lfo.clock();
      //data[i]= lfo.getValue();

      if ( !once )
        printf("%i\r\n",data[i]);
    }

    once= TRUE;

    WaveOut.Write((PBYTE)data,sizeof(data));
    WaveOut.Wait();
    /*
    char c;
    printf("\r\n?");
    scanf("%c",&c);
    int key;

    switch (c)
    {
      case 1: key= 10; break;
      case 2: key= 11; break;
      case 3: key= 12; break;
      case 4: key= 13; break;
      case 5: key= 14; break;
      case 6: key= 15; break;
      case 7: key= 16; break;
      case 8: key= 17; break;
      case 9: key= 18; break;
      case 0: key= 19; break;
      default:
        break;
    }

    instr1.noteOn(key);
    Sleep(3000);
    instr1.noteOff();
    */
  }

  //wave::File wfile;

  //wfile.Open("test.wav", wave::kOut);
  //wfile.set_sample_rate(8000);
  //wfile.set_bits_per_sample(16);
  //wfile.set_channel_number(1);
  //wfile.Write(content);


  //int chunksize= 0;
  //FILE *f= fopen("test.wav","w");
  //fseek(f,0,0);
  //fwrite("RIFF",1,4,f);
  //fwrite(&chunksize,4,1,f);
  //fwrite("WAVE",1,4,f);
  //fwrite("WAVE",1,4,f);
  //fwrite(buf,sizeof (int),BUFSIZE,f);
  //fflush(f);
  //fclose(f);

  //printf("sizeof int = %u\r\n",sizeof (int));
  //char c;
  //printf("Done!\r\n");
  //scanf("%c",&c);
  
  return 0;
}
