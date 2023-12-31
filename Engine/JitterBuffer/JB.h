/**
 * JitterBuffer.h - Contains the jitter buffer's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <Transport/ISocket.h>
#include <Transport/RTP/RTPPacket.h>
#include <Transport/RTP/OwnedRTPPacket.h>

#include <Common/TimeMeter.h>

#include <atomic>
#include <thread>
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
    sound
};

class JB : public Transport::ISocket
{
public:
	JB(Transport::ISocket &receiver, Common::TimeMeter &timeMeter);
	~JB();

    void SetFrameRate(uint32_t rate);

	void Start(Mode mode);
	void Stop();
	bool IsStarted();

    void SetSlowRenderingCallback(std::function<void(void)> callback);
    
	virtual void Send(const Transport::IPacket &packet_, const Transport::Address *address = nullptr) final;

private:
    Transport::ISocket &receiver;
    Common::TimeMeter &timeMeter;

    std::function<void(void)> slowRenderingCallback;

    Mode mode;
    std::atomic<bool> runned;
    std::thread thread;

    std::mutex mutex;
    std::deque<std::shared_ptr<Transport::OwnedRTPPacket>> buffer;

	uint64_t packetDuration;

    uint64_t measureTime;
	uint64_t renderTime;
    uint32_t overTimeCount;

    uint32_t prevRxTS, maxRxInterval;
    double stateRxTS, covarianceRxTS;
    uint32_t checkTime;
 
    std::shared_ptr<spdlog::logger> sysLog, errLog;

	void run();

    uint32_t KalmanCorrectRxTS(uint32_t interarrivalTime);

    void CalcJitter(const Transport::RTPPacket::RTPHeader &header);
};

}
