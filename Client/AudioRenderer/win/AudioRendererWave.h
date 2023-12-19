/**
 * AudioRendererWave.h - Contains audio renderer's header based on waveOut API (for WinXP)
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <AudioRenderer/IAudioRenderer.h>

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

class AudioRendererWave : public IAudioRenderer, public Transport::ISocket
{
public:
	AudioRendererWave();
	virtual ~AudioRendererWave();

	/// Impl of IAudioRenderer
	virtual bool SetDeviceName(std::string_view name) final;
	virtual void Start(int32_t sampleFreq) final;
	virtual void Stop() final;
	virtual std::vector<std::string> GetSoundRenderers();
	virtual bool SetMute(bool yes) final;
	virtual bool GetMute() final;
	virtual void SetVolume(uint16_t val) final;
    virtual uint16_t GetVolume() final;
	virtual uint32_t GetLatency() const final;

	virtual void SetAECReceiver(Transport::ISocket *socket) final;
    virtual void SetErrorHandler(std::function<void(uint32_t code, std::string_view msg)>) final;

	/// Impl of Transport::ISocket
	virtual void Send(const Transport::IPacket &packet, const Transport::Address *) final;

private:
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
