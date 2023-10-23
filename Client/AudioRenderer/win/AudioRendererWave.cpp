/**
 * AudioRenderer.cpp - Contains audio renderer's impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014, 2016
 */

#include "AudioRendererWave.h"
#include <Transport/RTP/RTPPacket.h>
#include <DSound.h>

#include <wui/config/config.hpp>

#include <boost/nowide/convert.hpp>

#include <atltrace.h>

namespace AudioRenderer
{

BOOL WaveBuffer::Init(AudioRendererWave *audioRenderer_, HWAVEOUT hWave, int Size)
{
	audioRenderer = audioRenderer_;
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

WaveBuffer::~WaveBuffer() 
{
	if (m_Hdr.lpData) 
	{
		waveOutUnprepareHeader(m_hWave, &m_Hdr, sizeof(WAVEHDR));
		LocalFree(m_Hdr.lpData);
	}
}

void WaveBuffer::Flush()
{
	// ASSERT(m_nBytes != 0);
	m_nBytes = 0;
	auto r = waveOutWrite(m_hWave, &m_Hdr, sizeof(WAVEHDR));
	if (r != MMSYSERR_NOERROR)
	{
		audioRenderer->runned = false;
		audioRenderer->error = true;
	}
}

BOOL WaveBuffer::Write(PBYTE pData, int nBytes, int& BytesWritten)
{
	// ASSERT((DWORD)m_nBytes != m_Hdr.dwBufferLength);
	BytesWritten = min((int)m_Hdr.dwBufferLength - m_nBytes, nBytes);
	CopyMemory((PVOID)(m_Hdr.lpData + m_nBytes), (PVOID)pData, BytesWritten);
	m_nBytes += BytesWritten;
	if (m_nBytes == (int)m_Hdr.dwBufferLength) 
	{
		/*  Write it! */
		m_nBytes = 0;
		auto r = waveOutWrite(m_hWave, &m_Hdr, sizeof(WAVEHDR));
		if (r != MMSYSERR_NOERROR)
		{
			audioRenderer->runned = false;
			audioRenderer->error = true;
		}
		return TRUE;
	}
	return FALSE;
}

///
AudioRendererWave::AudioRendererWave()
	: runned(false),
	readySem(),
	thread(),
	deviceName(),
	devices(),
    sampleFreq(wui::config::get_int("AudioRenderer", "SampleFreq", 48000)),
	volume(static_cast<uint16_t>((static_cast<float>(wui::config::get_int("AudioRenderer", "Volume", 100)) / 100) * 65535)),
	mute(wui::config::get_int("AudioRenderer", "Enabled", 1) == 0),
	m_hSem(NULL),
	m_nBuffers(4),
	m_CurrentBuffer(0),
	m_NoBuffer(TRUE),
	m_Hdrs(NULL),
	m_hWave(NULL),
	aecReceiver(nullptr),
    errorHandler()
{
}

AudioRendererWave::~AudioRendererWave()
{
	Stop();
}

bool AudioRendererWave::SetDeviceName(const std::string &name)
{
	if (deviceName == name)
	{
		return false;
	}

	deviceName = name;
	if (runned)
	{
		Stop();
		Start(sampleFreq);
	}
	return true;
}

void AudioRendererWave::Start(int32_t sampleFreq_)
{
    if (!runned)
	{
        sampleFreq = sampleFreq_;

		if (thread.joinable()) thread.join();
	
		thread = std::thread(&AudioRendererWave::run, this);
		readySem.wait_for(1000);
	}
}

void AudioRendererWave::Stop()
{
	runned = false;
	if (thread.joinable()) thread.join();
}

bool AudioRendererWave::SetMute(bool yes)
{
    wui::config::set_int("AudioRenderer", "Enabled", yes ? 0 : 1);
	mute = yes;
	return true;
}

bool AudioRendererWave::GetMute()
{
	return mute;
}

void AudioRendererWave::SetVolume(uint16_t value)
{
    wui::config::set_int("AudioRenderer", "Volume", value);

	volume = static_cast<uint16_t>((static_cast<double>(value) / 100) * 65535);

	SetVolume();
}

uint16_t AudioRendererWave::GetVolume()
{
    return static_cast<uint16_t>((static_cast<double>(volume) / 65535) * 100);
}

void AudioRendererWave::SetVolume()
{
	if (runned)
	{
		uint32_t vol = 0;
		*reinterpret_cast<uint16_t*>(&vol) = volume;
		*(reinterpret_cast<uint16_t*>(&vol) + 1) = volume;
		waveOutSetVolume(m_hWave, vol);
	}
}

void AudioRendererWave::Send(const Transport::IPacket &packet, const Transport::Address *)
{
	if (runned && !mute)
	{
		const Transport::RTPPacket &rtpPacket = *static_cast<const Transport::RTPPacket*>(&packet);

		PBYTE pData = const_cast<PBYTE>(rtpPacket.payload);
		int nBytes = rtpPacket.payloadSize;
		while (nBytes != 0)
		{
			/*  Get a buffer if necessary. */
			if (m_NoBuffer)
			{
				WaitForSingleObject(m_hSem, INFINITE);
				m_NoBuffer = FALSE;
			}

			/*  Write into a buffer. */
			int nWritten = 0;
			if (m_Hdrs[m_CurrentBuffer].Write(pData, nBytes, nWritten))
			{
				m_NoBuffer = TRUE;
				m_CurrentBuffer = (m_CurrentBuffer + 1) % m_nBuffers;
				nBytes -= nWritten;
				pData += nWritten;

				if (aecReceiver)
				{
					aecReceiver->Send(packet);
				}
			}
			else
			{
				// ASSERT(nWritten == nBytes);

				break;
			}
		}
	}
}

void CALLBACK WaveCallback(HWAVEOUT hWave, UINT uMsg, DWORD_PTR dwUser,
	DWORD_PTR dw1, DWORD_PTR dw2)
{
	if (uMsg == WOM_DONE)
	{
		ReleaseSemaphore((HANDLE)dwUser, 1, NULL);
	}
}

void AudioRendererWave::run()
{
	m_hSem = CreateSemaphore(NULL, m_nBuffers, m_nBuffers, NULL);
	
	WAVEFORMATEX wfx;
	wfx.cbSize = sizeof(WAVEFORMATEX);
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 1;
	wfx.nSamplesPerSec = sampleFreq;
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = (wfx.wBitsPerSample / 8) * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	MMRESULT ret = waveOutOpen(&m_hWave,
		GetDeviceID(),
		&wfx,
		(DWORD_PTR)WaveCallback,
		(DWORD_PTR)m_hSem,
		CALLBACK_FUNCTION);

	if (ret != 0)
	{
		ATLTRACE("Error in waveOutOpen\n");
        if (errorHandler)
        {
            errorHandler(0, "AudioRendererWave::waveOutOpen error");
        }
		return;
	}

	m_Hdrs = std::unique_ptr<WaveBuffer[]>(new WaveBuffer[m_nBuffers]);
	for (int i = 0; i < m_nBuffers; i++)
	{
		m_Hdrs[i].Init(this, m_hWave, sampleFreq / 25); // 20 ms delay
	}
	
	runned = true;
	error = false;
	
	SetVolume();

	readySem.notify();

	while (runned)
	{
		Sleep(500);
	}

	Wait();

	waveOutReset(m_hWave);
	m_Hdrs.reset(nullptr);
	waveOutClose(m_hWave);
	CloseHandle(m_hSem);

	if (error && errorHandler)
	{
        errorHandler(0, "AudioRendererWave::run()");
	}
}

void AudioRendererWave::Flush()
{
	if (!m_NoBuffer)
	{
		m_Hdrs[m_CurrentBuffer].Flush();
		m_NoBuffer = TRUE;
		m_CurrentBuffer = (m_CurrentBuffer + 1) % m_nBuffers;
	}
}

void AudioRendererWave::Wait()
{
	/*  Send any remaining buffers. */
	Flush();
	/*  Wait for the buffers back. */

	if (!error)
	{
		for (int i = 0; i < m_nBuffers; i++)
		{
			WaitForSingleObject(m_hSem, INFINITE);
		}
	}
	LONG lPrevCount;
	ReleaseSemaphore(m_hSem, m_nBuffers, &lPrevCount);
}

int AudioRendererWave::GetDeviceID()
{
	int id = 0;
	for (const auto &d : devices)
	{
		if (d.find(deviceName) != std::string::npos)
		{
			break;
		}
		++id;
	}
	return id;
}

BOOL CALLBACK AudioRendererWave::DSEnumNameDevices(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR, LPVOID lpContext)
{
	if (lpGUID != NULL)
	{
		((AudioRendererWave*)lpContext)->devices.emplace_back(boost::nowide::narrow(lpszDesc));
	}
	return TRUE;
}


std::vector<std::string> AudioRendererWave::GetSoundRenderers()
{
	devices.clear();
	
	HRESULT ret = DirectSoundEnumerate((LPDSENUMCALLBACK)AudioRendererWave::DSEnumNameDevices, (VOID*)this);
	if (!SUCCEEDED(ret))
	{
		ATLTRACE("DirectSoundEnumerate error %d \n", ret);
        if (errorHandler)
        {
            errorHandler(ret, " AudioRendererWave::GetSoundRenderers() DirectSoundEnumerate error");
        }
	}

	return devices;
}

uint32_t AudioRendererWave::GetLatency() const
{
	return wui::config::get_int("AudioRenderer", "Latency", 100);
}

void AudioRendererWave::SetAECReceiver(Transport::ISocket *socket)
{
	aecReceiver = socket;
}

void AudioRendererWave::SetErrorHandler(std::function<void(uint32_t code, const std::string &msg)> errorHandler_)
{
    errorHandler = errorHandler_;
}

}
