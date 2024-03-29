/**
 * JitterBuffer.cpp - Contains the jitter buffer's impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2022
 */

#include <JitterBuffer/JB.h>

#include <Common/CRC32.h>
#include <Common/Common.h>
#include <Common/ShortSleep.h>

namespace JB
{

JB::JB(Transport::ISocket &receiver_, Common::TimeMeter &timeMeter_)
	: receiver(receiver_),
    timeMeter(timeMeter_),
    slowRenderingCallback(),

    mode(Mode::video),
    runned(false),
    thread(),

    mutex(),
    buffer(),

	packetDuration(20000),
	measureTime(0),
	renderTime(0),
    overTimeCount(0),

    prevRxTS(0), maxRxInterval(static_cast<uint32_t>(packetDuration / 1000)),
    stateRxTS(20.0), covarianceRxTS(0.1),
    checkTime(0),

    sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

JB::~JB()
{
	Stop();
}

void JB::Start(Mode mode_)
{
	if (!runned)
	{
		mode = mode_;
        packetDuration = (mode == Mode::sound ? 20000 : (packetDuration < 40000 ? 40000 : packetDuration));

        renderTime = 0;
        overTimeCount = 0;

        prevRxTS = static_cast<uint32_t>(timeMeter.Measure() / 1000);
        maxRxInterval = static_cast<uint32_t>(packetDuration / 1000);
        stateRxTS = 20.0;
        covarianceRxTS = 0.1;

        checkTime = 0;

		measureTime = timeMeter.Measure();

		runned = true;
		thread = std::thread(&JB::run, this);
	}
}

void JB::Stop()
{
	if (runned)
	{
		runned = false;
		if (thread.joinable()) thread.join();
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

void JB::SetFrameRate(uint32_t rate)
{
	Stop();
	packetDuration = (1000 / rate) * 1000;
	Start(mode);
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

        CalcJitter(packet.rtpHeader);
	}
}

void JB::run()
{
	const uint64_t APPROACH = 500;

	while (runned)
	{
		auto startTime = timeMeter.Measure();
		while (runned && renderTime + APPROACH < packetDuration && timeMeter.Measure() - startTime < packetDuration - renderTime - APPROACH)
		{
			Common::ShortSleep();
		}
        
        std::shared_ptr<Transport::OwnedRTPPacket> packet;
        {
            std::lock_guard<std::mutex> lock(mutex);

            if (checkTime == (packetDuration / 1000) * (mode == Mode::sound ? 300 : 150))
            {
                while (!buffer.empty() && maxRxInterval < buffer.size() * (packetDuration / 1000))
                {
                    if (mode == Mode::video)
                    {
                        packet = buffer.front();

                        Transport::RTPPacket outputPacket;
                        outputPacket.rtpHeader = packet->header;
                        outputPacket.payload = packet->data;
                        outputPacket.payloadSize = packet->size;
                        receiver.Send(outputPacket);
                    }
                    
                    buffer.pop_front();
                }

                checkTime = 0;
                maxRxInterval = static_cast<uint32_t>(packetDuration / 1000);
            }
            checkTime += static_cast<uint32_t>(packetDuration / 1000);

            if (!buffer.empty() && maxRxInterval < buffer.size() * (packetDuration / 1000))
            {
                packet = buffer.front();
                buffer.pop_front();
            }
            else
            {
                if (mode == Mode::sound)
                {
                    packet = std::shared_ptr<Transport::OwnedRTPPacket>(new Transport::OwnedRTPPacket(
                        {},
                        (uint8_t*)"����s��FO�OO��/�",
                        16,
                        Transport::RTPPayloadType::ptOpus)); // Send the silent packet to prevent audio clicks
                }
                else
                {
                    continue;
                }
            }
        }

		auto sendTime = timeMeter.Measure();
        
        Transport::RTPPacket outputPacket;
		outputPacket.rtpHeader = packet->header;
		outputPacket.payload = packet->data;
		outputPacket.payloadSize = packet->size;
		receiver.Send(outputPacket);

        packet.reset();
		
		renderTime = timeMeter.Measure() - sendTime;

		if (renderTime > packetDuration + 5000)
		{
			++overTimeCount;
		}
		else if (overTimeCount > 0)
		{
			--overTimeCount;
		}
		if (overTimeCount == 100)
		{
			overTimeCount = 10000; // prevent duplicates

            if (slowRenderingCallback)
            {
                slowRenderingCallback();
            }
		}
	}
}

void JB::CalcJitter(const Transport::RTPPacket::RTPHeader &header)
{
    auto currentTime = static_cast<uint32_t>(timeMeter.Measure() / 1000);
    auto interarrivalTime = currentTime - prevRxTS;

    prevRxTS = currentTime;

    auto correctedInterval = KalmanCorrectRxTS(interarrivalTime);

    if (maxRxInterval < correctedInterval)
    {
        maxRxInterval = correctedInterval;
    }
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
