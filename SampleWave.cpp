#include <math.h>

#include "SampleWave.h"

//#include "SID/SIDIF.h"
//#include "SID/Instrument.h"
#include "TestSID.h"

unsigned char freqtbllo[] = {
  0x17,0x27,0x39,0x4b,0x5f,0x74,0x8a,0xa1,0xba,0xd4,0xf0,0x0e,
  0x2d,0x4e,0x71,0x96,0xbe,0xe8,0x14,0x43,0x74,0xa9,0xe1,0x1c,
  0x5a,0x9c,0xe2,0x2d,0x7c,0xcf,0x28,0x85,0xe8,0x52,0xc1,0x37,
  0xb4,0x39,0xc5,0x5a,0xf7,0x9e,0x4f,0x0a,0xd1,0xa3,0x82,0x6e,
  0x68,0x71,0x8a,0xb3,0xee,0x3c,0x9e,0x15,0xa2,0x46,0x04,0xdc,
  0xd0,0xe2,0x14,0x67,0xdd,0x79,0x3c,0x29,0x44,0x8d,0x08,0xb8,
  0xa1,0xc5,0x28,0xcd,0xba,0xf1,0x78,0x53,0x87,0x1a,0x10,0x71,
  0x42,0x89,0x4f,0x9b,0x74,0xe2,0xf0,0xa6,0x0e,0x33,0x20,0xff,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

unsigned char freqtblhi[] = {
  0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02,
  0x02,0x02,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x03,0x03,0x04,
  0x04,0x04,0x04,0x05,0x05,0x05,0x06,0x06,0x06,0x07,0x07,0x08,
  0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0c,0x0d,0x0d,0x0e,0x0f,0x10,
  0x11,0x12,0x13,0x14,0x15,0x17,0x18,0x1a,0x1b,0x1d,0x1f,0x20,
  0x22,0x24,0x27,0x29,0x2b,0x2e,0x31,0x34,0x37,0x3a,0x3e,0x41,
  0x45,0x49,0x4e,0x52,0x57,0x5c,0x62,0x68,0x6e,0x75,0x7c,0x83,
  0x8b,0x93,0x9c,0xa5,0xaf,0xb9,0xc4,0xd0,0xdd,0xea,0xf8,0xff,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

#include "GT/gsid.h"

BOOL CWaveBuffer::Init(HWAVEOUT hWave, int Size)
{
    m_hWave  = hWave;
    m_nBytes = 0;

    /*  Allocate a buffer and initialize the header. */
    m_Hdr.lpData = (LPSTR)LocalAlloc(LMEM_FIXED, Size);
    if (m_Hdr.lpData == NULL) 
    {
        return FALSE;
    }
    m_Hdr.dwBufferLength  = Size;
    m_Hdr.dwBytesRecorded = 0;
    m_Hdr.dwUser = 0;
    m_Hdr.dwFlags = 0;
    m_Hdr.dwLoops = 0;
    m_Hdr.lpNext = 0;
    m_Hdr.reserved = 0;

    /*  Prepare it. */
    waveOutPrepareHeader(hWave, &m_Hdr, sizeof(WAVEHDR));
    return TRUE;
}

CWaveBuffer::~CWaveBuffer() 
{
    if (m_Hdr.lpData) 
    {
        waveOutUnprepareHeader(m_hWave, &m_Hdr, sizeof(WAVEHDR));
        LocalFree(m_Hdr.lpData);
    }
}

void CWaveBuffer::Flush()
{
    // ASSERT(m_nBytes != 0);
    m_nBytes = 0;
    waveOutWrite(m_hWave, &m_Hdr, sizeof(WAVEHDR));
}

BOOL CWaveBuffer::Write(PBYTE pData, int nBytes, int& BytesWritten)
{
    // ASSERT((DWORD)m_nBytes != m_Hdr.dwBufferLength);
    BytesWritten = min((int)m_Hdr.dwBufferLength - m_nBytes, nBytes);
    CopyMemory((PVOID)(m_Hdr.lpData + m_nBytes), (PVOID)pData, BytesWritten);
    m_nBytes += BytesWritten;
    if (m_nBytes == (int)m_Hdr.dwBufferLength) 
    {
        /*  Write it! */
        m_nBytes = 0;
        waveOutWrite(m_hWave, &m_Hdr, sizeof(WAVEHDR));
        return TRUE;
    }
    return FALSE;
}

void CALLBACK WaveCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, 
                           DWORD dw1, DWORD dw2)
{
    if (uMsg == WOM_DONE) 
    {
      ReleaseSemaphore((HANDLE)dwUser, 1, NULL);
    }
}

CWaveOut::CWaveOut(LPCWAVEFORMATEX Format, int nBuffers, int BufferSize) :
    m_nBuffers(nBuffers),
    m_CurrentBuffer(0),
    m_NoBuffer(TRUE),
    m_hSem(CreateSemaphore(NULL, nBuffers, nBuffers, NULL)),
    m_Hdrs(new CWaveBuffer[nBuffers]),
    m_hWave(NULL)
{
    /*  Create wave device. */
    waveOutOpen(&m_hWave,
                WAVE_MAPPER,
                Format,
                (DWORD)WaveCallback,
                (DWORD)m_hSem,
                CALLBACK_FUNCTION);

    /*  Initialize the wave buffers. */
    for (int i = 0; i < nBuffers; i++) 
    {
        m_Hdrs[i].Init(m_hWave, BufferSize);
    }
}

CWaveOut::~CWaveOut()
{
    /*  First, get the buffers back. */
    waveOutReset(m_hWave);
    /*  Free the buffers. */
    delete [] m_Hdrs;
    /*  Close the wave device. */
    waveOutClose(m_hWave);
    /*  Free the semaphore. */
    CloseHandle(m_hSem);
}

void CWaveOut::Flush()
{
    if (!m_NoBuffer) 
    {
        m_Hdrs[m_CurrentBuffer].Flush();
        m_NoBuffer = TRUE;
        m_CurrentBuffer = (m_CurrentBuffer + 1) % m_nBuffers;
    }
}

void CWaveOut::Reset()
{
    waveOutReset(m_hWave);
}

void CWaveOut::Write(PBYTE pData, int nBytes)
{
    while (nBytes != 0) 
    {
        /*  Get a buffer if necessary. */
        if (m_NoBuffer) 
        {
            WaitForSingleObject(m_hSem, INFINITE);
            m_NoBuffer = FALSE;
        }

        /*  Write into a buffer. */
        int nWritten;
        if (m_Hdrs[m_CurrentBuffer].Write(pData, nBytes, nWritten)) 
        {
            m_NoBuffer = TRUE;
            m_CurrentBuffer = (m_CurrentBuffer + 1) % m_nBuffers;
            nBytes -= nWritten;
            pData += nWritten;
        } 
        else 
        {
            // ASSERT(nWritten == nBytes);
            break;
        }
    }
}

void CWaveOut::Wait()
{
    /*  Send any remaining buffers. */
    Flush();
    /*  Wait for the buffers back. */
    for (int i = 0; i < m_nBuffers; i++) 
    {
        WaitForSingleObject(m_hSem, INFINITE);
    }
    LONG lPrevCount;
    ReleaseSemaphore(m_hSem, m_nBuffers, &lPrevCount);
}

HRESULT RenderStreamToDevice(IMultiMediaStream *pMMStream)
{
    WAVEFORMATEX wfx;
    #define DATA_SIZE 5000

    IMediaStream        *pStream = NULL;
    IAudioStreamSample  *pSample = NULL;
    IAudioMediaStream   *pAudioStream = NULL;
    IAudioData          *pAudioData = NULL;

    HRESULT hr = pMMStream->GetMediaStream(MSPID_PrimaryAudio, &pStream);
    if (FAILED(hr))
    {
        return hr;
    }

    pStream->QueryInterface(IID_IAudioMediaStream, (void **)&pAudioStream);
    pStream->Release();

    hr = CoCreateInstance(CLSID_AMAudioData, NULL, 
        CLSCTX_INPROC_SERVER, IID_IAudioData, (void **)&pAudioData);
    if (FAILED(hr))
    {
        pAudioStream->Release();
        return hr;
    }

    PBYTE pBuffer = (PBYTE)LocalAlloc(LMEM_FIXED, DATA_SIZE);
    if (pBuffer == NULL)
    {
        pAudioStream->Release();
        pAudioData->Release();
        return E_OUTOFMEMORY;
    }

    pAudioStream->GetFormat(&wfx);
    pAudioData->SetBuffer(DATA_SIZE, pBuffer, 0);
    pAudioData->SetFormat(&wfx);
    hr = pAudioStream->CreateSample(pAudioData, 0, &pSample);
    pAudioStream->Release();
    if (FAILED(hr))
    {
        LocalFree((HLOCAL)pBuffer);
        pAudioData->Release();
        pSample->Release();
        return hr;
    }

/*
/////////////////////////////////////////////////////////
  Instrument::LFO lfo;
  lfo.create(Instrument::LFO::Type_Sinus,1000,0,44100);
  lfo.setFreq(400);
/////////////////////////////////////////////////////////
*/

    sid_init(MINMIXRATE,0,0,0,0,0);

    CWaveOut WaveOut(&wfx, 4, 2048);

                short b[512];

                double step=(M_PI * 2.0 / sizeof(b));

                for ( int i= 0; i < sizeof(b); i++ )
                {
                  double pos= i * step;
                  b[i]= (short)(10000 * sin(pos));
                  printf("%i\r\n",b[i]);
                }


    HANDLE hEvent = CreateEvent(FALSE, NULL, NULL, FALSE);

    if (hEvent != 0)
    {
        int iTimes;
        for (iTimes = 0; iTimes < 3; iTimes++) 
        {
           /*
            instr1.noteOff();
            instr1.noteOn(40 + iTimes);
            instr1.noteOff();
           */

           sidreg[0]= freqtbllo[22]; // Frequency
           sidreg[1]= freqtblhi[22];

           sidreg[5]= 0x11;   // ADSR
           sidreg[6]= 0xFF;

           sidreg[24] = 0x8F; // filter / master volume

           sidreg[0x04] = 0x41; // wave / note on

            DWORD dwStart = timeGetTime();
            for (; ; ) 
            {
                //hr = pSample->Update(0, hEvent, NULL, 0);
                //if (FAILED(hr) || hr == MS_S_ENDOFSTREAM) 
                //{
                //    break;
                //}
                //WaitForSingleObject(hEvent, INFINITE);
                
                 DWORD dwTimeDiff = timeGetTime() - dwStart;
                 // Limit to 10 seconds
                
                //if (dwTimeDiff > 5000 )
                //  sidreg[0x04] = 0x20; // note off

                if (dwTimeDiff > 10000) {
                    break;
                }
                //DWORD dwLength;
                //pAudioData->GetInfo(NULL, NULL, &dwLength);

                /*
                for ( unsigned int j= 0; j < res; j++)
                {
                  //t.sid.clock();
                  //((short*)pBuffer)[j]= t.sid.output();

                  //lfo.clock();
                  //((short*)pBuffer)[j]= lfo.getValue();
                  //printf("%i\r\n",((short*)pBuffer)[j]);
                }
                */


                // int  res = sid_fillbuffer(b,256);
                


                WaveOut.Write((PBYTE)b, sizeof(b));
                
            }
            pMMStream->Seek(0);
        }
    }

    pAudioData->Release();
    pSample->Release();
    LocalFree((HLOCAL)pBuffer);
    return S_OK;
}

HRESULT RenderFileToMMStream(
    const char * szFileName, 
    IMultiMediaStream **ppMMStream)
{
    if (strlen(szFileName) > MAX_PATH)
    {
        return E_INVALIDARG;
    }

    IAMMultiMediaStream *pAMStream;
    HRESULT hr = CoCreateInstance(CLSID_AMMultiMediaStream, NULL, 
        CLSCTX_INPROC_SERVER, IID_IAMMultiMediaStream, 
        (void **)&pAMStream);
    if (FAILED(hr))
    { 
        return hr;
    }

    WCHAR wszName[MAX_PATH + 1];
    MultiByteToWideChar(CP_ACP, 0, szFileName, -1, wszName,
        MAX_PATH + 1);
    
    pAMStream->Initialize(STREAMTYPE_READ, AMMSF_NOGRAPHTHREAD, NULL);
    pAMStream->AddMediaStream(NULL, &MSPID_PrimaryAudio, 0, NULL);
    hr = pAMStream->OpenFile(wszName, AMMSF_RUN);
    {
        if (SUCCEEDED(hr))
        {
            hr = pAMStream->QueryInterface(IID_IMultiMediaStream, 
                (void**)ppMMStream);
        }
    }
    pAMStream->Release();
    return hr;
}

#define BUF_SIZE 1024


void RenderBufferToDevice()
{
  WAVEFORMATEX format;

  memset(&format,0,sizeof(WAVEFORMATEX));

  format.wFormatTag     = WAVE_FORMAT_PCM;
  format.nChannels      = 1;
  format.wBitsPerSample = 16;
  format.nSamplesPerSec = MINMIXRATE;
  format.nBlockAlign    = format.nChannels * format.wBitsPerSample / 8;
  format.nAvgBytesPerSec= format.nBlockAlign * format.nSamplesPerSec ;   
  format.cbSize         = 0;

  CWaveOut WaveOut(&format, 4, 2048);

  sid_init(MINMIXRATE,0,0,0,0,0);

  sidreg[0]= freqtbllo[16]; // Frequency
  sidreg[1]= freqtblhi[16];

  sidreg[5]= 0x11;   // ADSR
  sidreg[6]= 0x55;

  sidreg[24] = 0x8F; // filter / master volume

  sidreg[0x04] = 0x21; // wave / note on

  short b[BUF_SIZE];

  //double step=(M_PI * 2.0 / BUF_SIZE);

  //for ( int i= 0; i < BUF_SIZE; i++ )
  //{
  //  double pos= i * step;
  //  b[i]= 10000 * sin(pos);
  //  printf("%i\r\n",b[i]);
  //}


  DWORD dwStart = timeGetTime();
  DWORD dwTime= 0;
  
  while (dwTime < 10000)
  {
    if ( dwTime > 3000 && ( sidreg[0x04] & 0x01 ) != 0 )
      sidreg[0x04]= sidreg[0x04] & 0xFE;

    short *pBuf= b;
    unsigned int res= sid_fillbuffer(b,BUF_SIZE);
    WaveOut.Write((PBYTE)b, BUF_SIZE*2);
    dwTime= timeGetTime() - dwStart;
  }
}


void testSID()
{
  TestSID t;

  sid_init(MINMIXRATE,0,0,0,0,0);

  t.init(MINMIXRATE,0,0,0,0,0);

  t.filter.setMode(SIDIF::Filter::Mode_Off);
  //t.filter.setCutoffFrequency(0xFFFF);
  //t.filter.setResonance(0);
  //t.filter.setActiveVoice(1,false);
  //t.filter.setActiveVoice(2,false);
  //t.filter.setActiveVoice(3,false);
  t.filter.setVolume(15);
  
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////
  t.instrument[0].getVoice()->setWaveform(SIDIF::Voice::Waveform_Pulse);
  t.instrument[0].getVoice()->setPulseWidth(1024);
  t.instrument[0].getVoice()->setADSR(1,1,5,15);

  Instrument::Control *ctrl;
  ctrl= t.instrument[0].getControl(Instrument::ControlType_PWM);

  ctrl->setAmplitude(800);
  ctrl->setSamplingRate(MINMIXRATE);
  ctrl->setOffset(1000);
  ctrl->setLFO(Instrument::LFO::Type::Type_Triangle);
  ctrl->setLFOFrequence(1000);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////
  t.instrument[1].getVoice()->setWaveform(SIDIF::Voice::Waveform_Sawtooth);
  t.instrument[1].getVoice()->setPulseWidth(1024);
  t.instrument[1].getVoice()->setADSR(1,1,5,15);

  ctrl= t.instrument[1].getControl(Instrument::ControlType_Frequency);

  ctrl->setAmplitude(1000);
  ctrl->setSamplingRate(MINMIXRATE);
  ctrl->setOffset(1000);
  ctrl->setLFO(Instrument::LFO::Type::Type_Triangle);
  ctrl->setLFOFrequence(500);

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

  sidreg[0] = freqtbllo[16]; // Frequency
  sidreg[1] = freqtblhi[16];
  sidreg[5] = 0x11;          // ADSR
  sidreg[6] = 0x55;           
  sidreg[24]= 0x8F;          // filter / master volume
  sidreg[4] = 0x21;          // wave / note on
  
  t.instrument[0].noteOn(16);
  t.instrument[1].noteOn(16);

  DWORD dwStart = timeGetTime();
  DWORD dwTime= 0;

  short b[BUF_SIZE];

  while (dwTime < 10000)
  {
    if ( dwTime > 5000 && ( sidreg[0x04] & 0x01 ) != 0 )
    {
      t.instrument[0].noteOff();
      t.instrument[1].noteOff();
    }

    short *pBuf= b;
    //unsigned int res= sid_fillbuffer(b,BUF_SIZE);
    unsigned int res= t.fillBuffer(b,BUF_SIZE);
    WaveOut.Write((PBYTE)b, BUF_SIZE*2);
    dwTime= timeGetTime() - dwStart;
  }
}