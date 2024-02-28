/**
 * AudioRendererWave.h - Contains audio renderer's header based on waveOut API (for WinXP)
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016, 2024
 */

#pragma once

#include <AudioRenderer/AudioRenderer.h>

#include <Transport/ISocket.h>

#include <memory>
#include <thread>
#include <string>
#include <atomic>
#include <queue>

#include <mt/semaphore.h>

#include <Audio/SoundBlock.h>

#include <Device/DS/DSCommon.h>

namespace AudioRenderer
{

class AudioRendererWave;

class WaveBuffer
{
public:
	~WaveBuffer();
	BOOL Init(AudioRendererWave *audioRenderer, HWAVEOUT hWave, int Size);
	BOOL Write(PBYTE pData, int nBytes, int& BytesWritten);
	void Flush();
private:
	AudioRendererWave *audioRenderer;
	WAVEHDR  m_Hdr;
	HWAVEOUT m_hWave;
	int      m_nBytes;
};

class AudioRendererWave
{
public:
	AudioRendererWave(std::function<void(Transport::OwnedRTPPacket&)> pcmSource);
	~AudioRendererWave();

	bool SetDeviceName(std::string_view name);
	void Start(int32_t sampleFreq);
	void Stop();
	std::vector<std::string> GetSoundRenderers();
	bool SetMute(bool yes);
	bool GetMute();
	void SetVolume(uint16_t val);
    uint16_t GetVolume();
	uint32_t GetLatency() const;

	void SetAECReceiver(Transport::ISocket *socket);
    void SetErrorHandler(std::function<void(uint32_t code, std::string_view msg)>);

private:
	std::function<void(Transport::OwnedRTPPacket&)> pcmSource;

	std::atomic<bool> runned, error;

	mt::semaphore readySem;

	std::thread thread;

	std::string deviceName;

	std::vector<std::string> devices;

    int32_t sampleFreq;
    		
	uint16_t volume;
	std::atomic<bool> mute;
			
	HANDLE                        m_hSem;
	const int                     m_nBuffers;
	int                           m_CurrentBuffer;
	BOOL                          m_NoBuffer;
	std::unique_ptr<WaveBuffer[]> m_Hdrs;
	HWAVEOUT                      m_hWave;

	Transport::ISocket* aecReceiver;

    std::function<void(uint32_t code, std::string_view msg)> errorHandler;

	void Flush();
	void Wait();

	int GetDeviceID();
	static BOOL CALLBACK DSEnumNameDevices(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName, LPVOID lpContext);
	
	void run();

	void SetVolume();

	friend WaveBuffer;
};

}
