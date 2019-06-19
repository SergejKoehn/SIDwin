#include "TestSound.h"

CTestSound::CTestSound()
{

}

CTestSound::~CTestSound()
{

}

void CTestSound::create(unsigned int bufSize)
{
  LPDIRECTSOUND8  soundIF;

  DirectSoundCreate8(NULL,&soundIF,NULL);

  if (soundIF == NULL)
    return;
  
  WAVEFORMATEX format;

  memset(&format,0,sizeof(WAVEFORMATEX));

  format.wFormatTag     = WAVE_FORMAT_PCM;
  format.nChannels      = 1;
  format.wBitsPerSample = 16;
  format.nSamplesPerSec = 44100;
  format.nBlockAlign    = format.nChannels * format.wBitsPerSample / 8;
  format.nAvgBytesPerSec= format.nBlockAlign * format.nSamplesPerSec ;   
  format.cbSize         = 0;

   
  memset(&bufParams,0,sizeof(DSBUFFERDESC));

  bufParams.dwSize         = sizeof(bufParams);
  bufParams.dwBufferBytes  = bufSize;
  bufParams.dwFlags        = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 |DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2;
  bufParams.dwReserved     = 0;
  bufParams.guid3DAlgorithm= DS3DALG_DEFAULT;
  bufParams.lpwfxFormat    = &format;

  soundIF->SetCooperativeLevel(GetDesktopWindow(),DSSCL_PRIORITY);

  HRESULT hResult= soundIF->CreateSoundBuffer(&bufParams,&bufPtr,NULL);

  char *errText;
  switch (hResult)
  {
    case DSERR_ALLOCATED      : 
      errText= "DSERR_ALLOCATED";      
      break;
    case DSERR_CONTROLUNAVAIL : 
      errText= "DSERR_CONTROLUNAVAIL"; 
      break;
    case DSERR_BADFORMAT      : errText= "DSERR_BADFORMAT";      break;
    case DSERR_INVALIDPARAM   : errText= "DSERR_INVALIDPARAM";   break;
    case DSERR_NOAGGREGATION  : errText= "DSERR_NOAGGREGATION";  break;
    case DSERR_OUTOFMEMORY    : errText= "DSERR_OUTOFMEMORY";    break;
    case DSERR_UNINITIALIZED  : errText= "DSERR_UNINITIALIZED";  break;
    case DSERR_UNSUPPORTED    : errText= "DSERR_UNSUPPORTED";    break;
    default:
      break;
  }
}

void CTestSound::lock()
{
  IDirectSoundBuffer_Lock( bufPtr,
                            0,
                            bufParams.dwBufferBytes,
                            (LPVOID *)&data1,
                            (LPDWORD)&size1,
                            (LPVOID *)&data2,
                            (LPDWORD)&size2,
                            DSBLOCK_ENTIREBUFFER
                          );
}

void CTestSound::release()
{
  IDirectSoundBuffer_Unlock(bufPtr,(LPVOID) data1, (DWORD)size1,(LPVOID) data2, (DWORD)size2);
}

void CTestSound::play()
{
  IDirectSoundBuffer_Play(bufPtr,0,0,DSBPLAY_LOOPING); //DSBPLAY_LOOPING  
}

void CTestSound::stop()
{
  IDirectSoundBuffer_Stop(bufPtr); //DSBPLAY_LOOPING  
}