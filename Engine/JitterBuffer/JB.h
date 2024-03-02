/**
 * JitterBuffer.h - Contains the jitter buffer's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <Transport/ISocket.h>
#include <Transport/RTP/OwnedRTPPacket.h>

#include <Common/TimeMeter.h>

#include <atomic>
#include <mutex>
#include <deque>
#include <vector>
#include <algorithm>
#include <functional>

#include <memory.h>

#include <spdlog/spdlog.h>

namespace JB
{

enum class Mode
{
    video,
    sound,
    local
};

class JB : public Transport::ISocket
{
public:
	JB(Common::TimeMeter &timeMeter);
	~JB();

    void SetFrameRate(uint32_t rate);

	void Start(Mode mode, std::string_view name);
	void Stop();
	bool IsStarted();

    void SetSlowRenderingCallback(std::function<void(void)> callback);
    
    /// Receive PCM audio or RGB video frame
	virtual void Send(const Transport::IPacket &packet_, const Transport::Address *address = nullptr) final;

    /// Get next frame to play in renderer (The data will be moved to the buffer provided by the argument)
    void GetFrame(Transport::OwnedRTPPacket&);
private:
    Common::TimeMeter &timeMeter;

    std::function<void(void)> slowRenderingCallback;

    Mode mode;
    std::string name;
    std::atomic_bool runned;

    std::mutex mutex;
    std::deque<std::shared_ptr<Transport::OwnedRTPPacket>> buffer;

	uint64_t packetDuration;

    uint64_t measureTime;
	uint64_t renderTime;
    int32_t overTimeCount;

    uint32_t prevRxTS, maxRxInterval;
    double stateRxTS, covarianceRxTS;
    uint32_t checkTime;

    uint16_t prevSeq;
 
    std::shared_ptr<spdlog::logger> sysLog, errLog;

    uint32_t KalmanCorrectRxTS(uint32_t interarrivalTime);

    void CalcJitter(const Transport::RTPPacket::RTPHeader &header);
};

}
