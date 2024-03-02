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
    case Mode::local: return "Local";
    case Mode::sound: return "Sound";
    case Mode::video: return "Video";
    }
    return "";
}

JB::JB(Common::TimeMeter &timeMeter_)
	: timeMeter(timeMeter_),
    slowRenderingCallback(),

    mode(Mode::video),
    name(),
    runned(false),

    mutex(),
    buffer(),

	frameDuration(10),
	measureTime(0),
	renderTime(0),
    overTimeCount(0),

    prevRxTS(0), rxInterval(frameDuration),
    stateRxTS(10.0), covarianceRxTS(0.1),
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
		mode = mode_;
        name = name_;
        frameDuration = (mode == Mode::sound ? 10 : (frameDuration < 40 ? 40 : frameDuration));

        renderTime = 0;
        overTimeCount = 0;
        prevSeq = 0;

        prevRxTS = static_cast<uint32_t>(timeMeter.Measure() / 1000);
        rxInterval = frameDuration;
        stateRxTS = frameDuration;
        covarianceRxTS = 0.1;

        checkTime = 0;

		measureTime = timeMeter.Measure();

		runned = true;
	}
}

void JB::Stop()
{
	if (runned)
	{
		runned = false;
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
            mode == Mode::sound ? Transport::RTPPayloadType::ptOpus : Transport::RTPPayloadType::ptVP8);

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
    if (checkTime == frameDuration * (mode == Mode::sound ? 300 : 150))
    {
        sysLog->trace("{0}_JB[{1}] :: Check (rxInterval: {2}, buffer size: {3})", to_string(mode), name, rxInterval, buffer.size());

        /*while (buffer.size() > 1 && rxInterval > buffer.size() * frameDuration * 3) /// Prevent big delay
        {
            buffer.pop_front();
        }*/

        checkTime = 0;
    }
    checkTime += frameDuration;

    if (!buffer.empty())
    {
        if (mode == Mode::local ||
            rxInterval < buffer.size() * frameDuration)
        {
            output = std::move(*buffer.front());
            buffer.pop_front();
        }
        else
        {
            sysLog->trace("{0}_JB[{1}] :: Buffering (rxInterval: {2}, buffer size: {3})", to_string(mode), name, rxInterval, buffer.size());
        }
    }
    else
    {
        sysLog->warn("{0}_JB[{1}] :: Empty (rxInterval: {2})", to_string(mode), name, rxInterval);
    }  
}

void JB::CalcJitter(const Transport::RTPPacket::RTPHeader &header)
{
    auto currentTime = static_cast<uint32_t>(timeMeter.Measure() / 1000);
    auto interarrivalTime = currentTime - prevRxTS;

    prevRxTS = currentTime;

    rxInterval = KalmanCorrectRxTS(interarrivalTime);
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

}
