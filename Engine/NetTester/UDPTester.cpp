/**
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
	runned(false),
	thread(),
	connections()
{
}

UDPTester::~UDPTester()
{
	runned = false;
	if (thread.joinable()) thread.join();
}

void UDPTester::AddAddress(const char* address, uint16_t port)
{
	if (std::find(connections.begin(), connections.end(), Connection(Transport::Address(address, port))) == connections.end())
	{
		connections.emplace_back(Connection(Transport::Address(address, port)));
	}
}

void UDPTester::ClearAddresses()
{
	connections.clear();
}

void UDPTester::DoTheTest()
{
	if (!runned && !connections.empty())
	{
		if (thread.joinable()) thread.join();

		thread = std::thread([this]() {
			runned = true;

			Transport::RTCPPacket packet;
			packet.rtcp_common.pt = Transport::RTCPPacket::RTCP_APP;
			packet.rtcp_common.length = 1;
			packet.rtcps[0].r.app.messageType = Transport::RTCPPacket::amtUDPTest;
			
			for (auto &connection : connections)
			{
				connection.available = false;

				if (connection.address.type != Transport::Address::Type::Undefined)
				{
					Transport::RTPSocket rtpSocket;

					rtpSocket.SetReceiver(nullptr, this);

					rtpSocket.Start(connection.address.type);

					rtpSocket.Send(packet, &connection.address);

					std::this_thread::sleep_for(std::chrono::milliseconds(2000));
					rtpSocket.Send(packet, &connection.address);

					rtpSocket.Stop();
				}
			}

			if (runned)
			{
                readyCallback();
			}

			runned = false;
		});
	}
}

bool UDPTester::TestPassed() const
{
	for (auto &connection : connections)
	{
		if (!connection.available)
		{
			return false;
		}
	}
	return true;
}

std::string UDPTester::GetErrorMessage() const
{
    std::string errorMessage = locale->get("net_test", "udp_sockets_unavailable") + "\n";
	for (auto &connection : connections)
	{
		if (!connection.available)
		{
			errorMessage += connection.address.toString() + "\n";
		}
	}

	return errorMessage;
}

void UDPTester::Send(const Transport::IPacket &packet, const Transport::Address *address)
{
	auto it = std::find(connections.begin(), connections.end(), Connection(*address));
	if (it != connections.end())
	{
		it->available = true;
	}
}

}
