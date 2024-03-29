﻿/**
 * UDPTester.cpp - Contains udp availability tester impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <NetTester/UDPTester.h>

#include <Transport/RTPSocket.h>
#include <Transport/RTP/RTCPPacket.h>

#include <Common/Common.h>

#include <algorithm>

namespace NetTester
{

UDPTester::UDPTester(std::shared_ptr<wui::i_locale> locale_, std::function<void()> readyCallback_)
	: locale(locale_),
    readyCallback(readyCallback_),
	mutex(),
	iteration(0),
	timer(std::bind(&UDPTester::onTimer, this)),
	inputAddresses(), availAddresses(),
	rtpSockets(),
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

UDPTester::~UDPTester()
{
	Stop();
}

void UDPTester::AddAddress(std::string_view address, uint16_t port)
{
	sysLog->trace("UDPTester :: AddAddress :: Perform adding {0}:{1}", address, port);

	if (std::find(inputAddresses.begin(), inputAddresses.end(), Transport::Address(address.data(), port)) == inputAddresses.end())
	{
		inputAddresses.emplace_back(Transport::Address(address.data(), port));

		auto rtpSocket = std::make_shared<Transport::RTPSocket>();
		rtpSocket->SetReceiver(nullptr, this);
		
		std::lock_guard<std::mutex> lock(mutex);
		rtpSockets.emplace_back(rtpSocket);
	}
}

void UDPTester::ClearAddresses()
{
	sysLog->trace("UDPTester :: ClearAddresses()");

	inputAddresses.clear();
	availAddresses.clear();

	rtpSockets.clear();
}

void UDPTester::DoTheTest()
{
	if (!inputAddresses.empty())
	{
		timer.stop();

		availAddresses.clear();
		iteration = 0;

		sysLog->trace("UDPTester :: DoTheTest :: Started");

		timer.start(250);
	}
}

void UDPTester::Stop()
{
	timer.stop();
	
	sysLog->trace("UDPTester :: Stopped");
}

bool UDPTester::TestPassed() const
{
	return !availAddresses.empty();
}

std::string UDPTester::GetErrorMessage() const
{
    std::string errorMessage = locale->get("net_test", "udp_sockets_unavailable") + "\n";
	for (auto &input : inputAddresses)
	{
		if (std::find(availAddresses.begin(), availAddresses.end(), input) == availAddresses.end())
		{
			errorMessage += input.toString() + "\n";
		}
	}

	return errorMessage;
}

void UDPTester::Send(const Transport::IPacket &packet, const Transport::Address *address)
{
	std::lock_guard<std::mutex> lock(mutex);

	auto it = std::find(inputAddresses.begin(), inputAddresses.end(), *address);
	if (it != inputAddresses.end())
	{
		availAddresses.emplace_back(*address);
		sysLog->trace("UDPTester :: Added avail {0}", address->toString());
	}
}

void UDPTester::onTimer()
{
	std::lock_guard<std::mutex> lock(mutex);

	if (iteration < inputAddresses.size())
	{
		auto& a = inputAddresses[iteration];
		if (a.type != Transport::Address::Type::Undefined)
		{
			Transport::RTCPPacket packet;
			packet.rtcp_common.pt = Transport::RTCPPacket::RTCP_APP;
			packet.rtcp_common.length = 1;
			packet.rtcps[0].r.app.messageType = Transport::RTCPPacket::amtUDPTest;

			auto rtpSocket = rtpSockets[iteration];

			rtpSocket->Start(a.type);
			rtpSocket->Send(packet, &a);

			sysLog->trace("UDPTester :: DoTheTest :: Send test to address: {0}", a.toString());
		}
	}
	else if (iteration == inputAddresses.size() * 2) /// Double time to wait responses
	{
		readyCallback();
	}
	++iteration;
}

}
