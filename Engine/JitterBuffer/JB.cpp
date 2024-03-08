/**
 * JitterBuffer.cpp - Contains the jitter buffer's impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022, 2024
 */

#include <JitterBuffer/JB.h>

#include <Common/CRC32.h>
#include <Common/Common.h>
#include <Common/ShortSleep.h>

namespace JB
{

const char* to_string(Mode mode)
{
    switch (mode)
    {
    case Mode::Sound: return "Sound";
    case Mode::Video: return "Video";
    }
    return "";
}

JB::JB(Common::TimeMeter &timeMeter_)
	: timeMeter(timeMeter_),
    slowRenderingCallback(),

    mode(Mode::Video),
    name(),
    runned(false),

    mutex(),
    buffer(),

	frameDuration(40),

    buffering(true),
    reserveCount(4), // 160 ms delay

    prevRxTS(0), rxInterval(frameDuration),
    rxIntervals(),
    stateRxTS(40.0), covarianceRxTS(0.1),
    checkTime(0),

    prevSeq(0),

    sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

JB::~JB()
{
	Stop();
}

void JB::Start(Mode mode_, std::string_view name_)
{
	if (!runned)
	{
        while (!buffer.empty())
        {
            buffer.pop_front();
        }

		mode = mode_;
        name = name_;
        frameDuration = 40;
        reserveCount = (mode == Mode::Sound ? 4 : 1); /// 160 ms delay for sound, 40 ms for video

        prevSeq = 0;

        prevRxTS = static_cast<uint32_t>(timeMeter.Measure() / 1000);
        rxInterval = frameDuration;
        stateRxTS = frameDuration;
        covarianceRxTS = 0.1;

        checkTime = 0;

		runned = true;
        buffering = true;

        sysLog->info("{0}_JB[{1}] :: Started (frameDuration: {2}, reserveCount: {3})", to_string(mode), name, frameDuration, reserveCount);
	}
}

void JB::Stop()
{
	if (runned)
	{
		runned = false;
        while (!buffer.empty())
        {
            buffer.pop_front();
        }

        sysLog->info("{0}_JB[{1}] :: Stopped", to_string(mode), name);
	}
}

bool JB::IsStarted()
{
	return runned;
}

void JB::SetSlowRenderingCallback(std::function<void(void)> callback)
{
    slowRenderingCallback = callback;
}

void JB::SetFrameDuration(uint32_t ms)
{
	Stop();
	frameDuration = ms;
    sysLog->info("{0}_JB[{1}] :: Change frame duration (frameDuration: {2})", to_string(mode), name, frameDuration);
	Start(mode, name);
}

void JB::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
    if (runned)
	{
		const auto &packet = *static_cast<const Transport::RTPPacket*>(&packet_);

        auto rtpPacket = std::make_shared<Transport::OwnedRTPPacket>(packet.rtpHeader,
            packet.payload,
            packet.payloadSize,
            mode == Mode::Sound ? Transport::RTPPayloadType::ptOpus : Transport::RTPPayloadType::ptVP8);

        std::lock_guard<std::mutex> lock(mutex);
        buffer.push_back(rtpPacket);

        if (packet.rtpHeader.seq - 1 == prevSeq) /// Don't calc jitter on loses
        {
            CalcJitter(packet.rtpHeader);
        }
        else
        {
            sysLog->trace("{0}_JB[{1}] :: Packet loss (prev_seq: {2}, current_seq: {3})", to_string(mode), name, prevSeq, packet.rtpHeader.seq);
            prevRxTS = static_cast<uint32_t>(timeMeter.Measure() / 1000);
        }

        prevSeq = packet.rtpHeader.seq;
	}
}

void JB::GetFrame(Transport::OwnedRTPPacket& output)
{
    if (!runned)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex);

    if (buffering)
    {
        if (buffer.size() < reserveCount ||
            buffer.size() < (floor((double)rxInterval / frameDuration)))
        {
            return;
        }
        else
        {
            buffering = false;
        }
    }
    
    if (checkTime == 4000) // 4 seconds check interval
    {
        auto bufferSize = buffer.size();

        while (buffer.size() > reserveCount && rxInterval < (buffer.size() * frameDuration)) /// Prevent big delay
        {
            buffer.pop_front();
        }

        sysLog->trace("{0}_JB[{1}] :: Check (rxInterval: {2}, buffer size: {3}, removed: {4})", to_string(mode), name, rxInterval, buffer.size(), bufferSize - buffer.size());

        checkTime = 0;
    }
    checkTime += frameDuration;

    if (!buffer.empty())
    {
        if (mode == Mode::Video ||
            rxInterval <= (buffer.size() + 1) * frameDuration)
        {
            output = std::move(*buffer.front());
            buffer.pop_front();
        }
        else
        {
            buffering = true;
            sysLog->trace("{0}_JB[{1}] :: Buffering (rxInterval: {2}, buffer size: {3})", to_string(mode), name, rxInterval, buffer.size());
        }
    }
    else
    {
        buffering = true;
        sysLog->warn("{0}_JB[{1}] :: Empty (rxInterval: {2})", to_string(mode), name, rxInterval);
    }  
}

void JB::ReadFrame(Transport::OwnedRTPPacket &out)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (!buffer.empty())
    {
        out = *buffer.front();
    }
}

void JB::CalcJitter(const Transport::RTPPacket::RTPHeader &header)
{
    auto currentTime = static_cast<uint32_t>(timeMeter.Measure() / 1000);
    auto interarrivalTime = currentTime - prevRxTS;

    prevRxTS = currentTime;

    auto actualrxInterval = KalmanCorrectRxTS(interarrivalTime);
    rxIntervals.push_front(actualrxInterval);

    if (rxIntervals.size() > 100) rxIntervals.pop_back();

    rxInterval = GetMaxRX();
}

uint32_t JB::KalmanCorrectRxTS(uint32_t data)
{
    double f = 1.0, h = 1.0, q = 2.0, r = 2.0;

    //time update - prediction
    auto x0 = f * stateRxTS;
    auto p0 = f * covarianceRxTS * f + q;

    //measurement update - correction
    auto k = h * p0 / (h * p0 * h + r);
    stateRxTS = x0 + k * (data - h * x0);
    covarianceRxTS = (1 - k * h) * p0;

    return static_cast<uint32_t>(stateRxTS);
}

uint32_t JB::GetMaxRX()
{
    uint32_t max_ = 0;

    for (auto v : rxIntervals)
    {
        if (v > max_)
        {
            max_ = v;
        }
    }

    return max_;
}

}
