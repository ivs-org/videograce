/**
 * JitterBuffer.h - Contains the jitter buffer's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018, 2024
 */

#pragma once

#include <Transport/ISocket.h>
#include <Transport/RTP/OwnedRTPPacket.h>

#include <Common/TimeMeter.h>
#include <Common/StatMeter.h>

#include <atomic>
#include <mutex>
#include <deque>
#include <vector>
#include <algorithm>

#include <spdlog/spdlog.h>

namespace JB
{

enum class Mode
{
    Video,
    Sound
};

class JB : public Transport::ISocket
{
public:
	JB(Common::TimeMeter &timeMeter);
	~JB();

    void SetFrameDuration(uint32_t ms);

	void Start(Mode mode, std::string_view name);
	void Stop();
    bool IsStarted();
    
    /// Receive PCM audio or RGB video frame
	virtual void Send(const Transport::IPacket &packet_, const Transport::Address *address = nullptr) final;

    /// Get next frame to play in renderer (The data will be moved to the buffer provided by the argument)
    void GetFrame(Transport::OwnedRTPPacket&);

    /// Get last frame without delete from buffer
    void ReadFrame(Transport::OwnedRTPPacket&);
private:
    Common::TimeMeter &timeMeter;

    Mode mode;
    std::string name;
    std::atomic_bool runned;

    std::mutex mutex;
    std::deque<std::shared_ptr<Transport::OwnedRTPPacket>> buffer;

    Common::StatMeter statMeter;

	uint32_t frameDuration;

    bool buffering;
    uint32_t reserveCount;

    uint32_t prevRxTS, rxInterval;
    double stateRxTS, covarianceRxTS;
    uint32_t checkTime;

    uint16_t prevSeq;
 
    std::shared_ptr<spdlog::logger> sysLog, errLog;

    uint32_t KalmanCorrectRxTS(uint32_t interarrivalTime);

    void CalcJitter(const Transport::RTPPacket::RTPHeader &header);
};

}
