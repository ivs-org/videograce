/**
 * CommandType.cpp - Contains protocol command type impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#include <Proto/CommandType.h>

#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <vector>

#include <Common/Quoter.h>

#include <Proto/CmdUserUpdateRequest.h>
#include <Proto/CmdUserUpdateResponse.h>
#include <Proto/CmdCredentialsRequest.h>
#include <Proto/CmdCredentialsResponse.h>
#include <Proto/CmdConnectRequest.h>
#include <Proto/CmdConnectResponse.h>
#include <Proto/CmdDisconnect.h>
#include <Proto/CmdChangeServer.h>
#include <Proto/CmdPing.h>
#include <Proto/CmdSetMaxBitrate.h>
#include <Proto/CmdUpdateGrants.h>
#include <Proto/CmdContactList.h>
#include <Proto/CmdSearchContact.h>
#include <Proto/CmdContactsUpdate.h>
#include <Proto/CmdGroupList.h>
#include <Proto/CmdConferencesList.h>
#include <Proto/CmdDeviceParams.h>
#include <Proto/CmdDeviceConnect.h>
#include <Proto/CmdDeviceDisconnect.h>
#include <Proto/CmdRendererConnect.h>
#include <Proto/CmdRendererDisconnect.h>
#include <Proto/CmdResolutionChange.h>
#include <Proto/CmdMicrophoneActive.h>
#include <Proto/CmdCallRequest.h>
#include <Proto/CmdCallResponse.h>
#include <Proto/CmdConferenceUpdateRequest.h>
#include <Proto/CmdConferenceUpdateResponse.h>
#include <Proto/CmdCreateTempConference.h>
#include <Proto/CmdSendConnectToConference.h>
#include <Proto/CmdConnectToConferenceRequest.h>
#include <Proto/CmdConnectToConferenceResponse.h>
#include <Proto/CmdDisconnectFromConference.h>
#include <Proto/CmdChangeContactState.h>
#include <Proto/CmdChangeMemberState.h>
#include <Proto/CmdTurnSpeaker.h>
#include <Proto/CmdMemberAction.h>
#include <Proto/CmdWantSpeak.h>
#include <Proto/CmdScheduleConnect.h>
#include <Proto/CmdDeliveryMessages.h>
#include <Proto/CmdLoadMessages.h>
#include <Proto/CmdRequestMediaAddresses.h>
#include <Proto/CmdMediaAddressesList.h>

namespace Proto
{

CommandType GetCommandType(const std::string &message)
{
	auto firstQuotePos = message.find_first_of('"');
	if (firstQuotePos == std::string::npos)
	{
		return CommandType::Undefined;
	}
	auto secondQuotePos = message.find_first_of('"', firstQuotePos + 1);
	if (secondQuotePos == std::string::npos)
	{
		return CommandType::Undefined;
	}

	std::string command = message.substr(firstQuotePos, secondQuotePos);
	std::transform(command.begin(), command.end(), command.begin(), ::tolower);

	if (command == quot(Proto::USER_UPDATE_REQUEST::NAME))
	{
		return CommandType::UserUpdateRequest;
	}
	else if (command == quot(Proto::USER_UPDATE_RESPONSE::NAME))
	{
		return CommandType::UserUpdateResponse;
	}
	else if (command == quot(Proto::CREDENTIALS_REQUEST::NAME))
	{
		return CommandType::CredentialsRequest;
	}
	else if (command == quot(Proto::CREDENTIALS_RESPONSE::NAME))
	{
		return CommandType::CredentialsResponse;
	}
	else if (command == quot(Proto::CONNECT_REQUEST::NAME))
	{
		return CommandType::ConnectRequest;
	}
	else if (command == quot(Proto::CONNECT_RESPONSE::NAME))
	{
		return CommandType::ConnectResponse;
	}
	else if (command == quot(Proto::DISCONNECT::NAME))
	{
		return CommandType::Disconnect;
	}
	else if (command == quot(Proto::CHANGE_SERVER::NAME))
	{
		return CommandType::ChangeServer;
	}
	else if (command == quot(Proto::PING::NAME))
	{
		return CommandType::Ping;
	}
	else if (command == quot(Proto::SET_MAX_BITRATE::NAME))
	{
		return CommandType::SetMaxBitrate;
	}
	else if (command == quot(Proto::UPDATE_GRANTS::NAME))
	{
		return CommandType::UpdateGrants;
	}
	else if (command == quot(Proto::CONTACT_LIST::NAME))
	{
		return CommandType::ContactList;
	}
	else if (command == quot(Proto::SEARCH_CONTACT::NAME))
	{
		return CommandType::SearchContact;
	}
	else if (command == quot(Proto::CONTACTS_UPDATE::NAME))
	{
		return CommandType::ContactsUpdate;
	}
	else if (command == quot(Proto::GROUP_LIST::NAME))
	{
		return CommandType::GroupList;
	}
	else if (command == quot(Proto::CONFERENCES_LIST::NAME))
	{
		return CommandType::ConferencesList;
	}
	else if (command == quot(Proto::DEVICE_PARAMS::NAME))
	{
		return CommandType::DeviceParams;
	}
	else if (command == quot(Proto::DEVICE_CONNECT::NAME))
	{
		return CommandType::DeviceConnect;
	}
	else if (command == quot(Proto::DEVICE_DISCONNECT::NAME))
	{
		return CommandType::DeviceDisconnect;
	}
	else if (command == quot(Proto::RENDERER_CONNECT::NAME))
	{
		return CommandType::RendererConnect;
	}
	else if (command == quot(Proto::RENDERER_DISCONNECT::NAME))
	{
		return CommandType::RendererDisconnect;
	}
	else if (command == quot(Proto::RESOLUTION_CHANGE::NAME))
	{
		return CommandType::ResolutionChange;
	}
	else if (command == quot(Proto::MICROPHONE_ACTIVE::NAME))
	{
		return CommandType::MicrophoneActive;
	}
	else if (command == quot(Proto::CALL_REQUEST::NAME))
	{
		return CommandType::CallRequest;
	}
	else if (command == quot(Proto::CALL_RESPONSE::NAME))
	{
		return CommandType::CallResponse;
	}
	else if (command == quot(Proto::CONFERENCE_UPDATE_REQUEST::NAME))
	{
		return CommandType::ConferenceUpdateRequest;
	}
	else if (command == quot(Proto::CONFERENCE_UPDATE_RESPONSE::NAME))
	{
		return CommandType::ConferenceUpdateResponse;
	}
	else if (command == quot(Proto::CREATE_TEMP_CONFERENCE::NAME))
	{
		return CommandType::CreateTempConference;
	}
	else if (command == quot(Proto::SEND_CONNECT_TO_CONFERENCE::NAME))
	{
		return CommandType::SendConnectToConference;
	}
	else if (command == quot(Proto::CONNECT_TO_CONFERENCE_REQUEST::NAME))
	{
		return CommandType::ConnectToConferenceRequest;
	}
	else if (command == quot(Proto::CONNECT_TO_CONFERENCE_RESPONSE::NAME))
	{
		return CommandType::ConnectToConferenceResponse;
	}
	else if (command == quot(Proto::DISCONNECT_FROM_CONFERENCE::NAME))
	{
		return CommandType::DisconnectFromConference;
	}
	else if (command == quot(Proto::CHANGE_CONTACT_STATE::NAME))
	{
		return CommandType::ChangeContactState;
	}
	else if (command == quot(Proto::CHANGE_MEMBER_STATE::NAME))
	{
		return CommandType::ChangeMemberState;
	}
	else if (command == quot(Proto::TURN_SPEAKER::NAME))
	{
		return CommandType::TurnSpeaker;
	}
	else if (command == quot(Proto::MEMBER_ACTION::NAME))
	{
		return CommandType::MemberAction;
	}
	else if (command == quot(Proto::WANT_SPEAK::NAME))
	{
		return CommandType::WantSpeak;
	}
	else if (command == quot(Proto::SCHEDULE_CONNECT::NAME))
	{
		return CommandType::ScheduleConnect;
	}
	else if (command == quot(Proto::DELIVERY_MESSAGES::NAME))
	{
		return CommandType::DeliveryMessages;
	}
	else if (command == quot(Proto::LOAD_MESSAGES::NAME))
	{
		return CommandType::LoadMessages;
	}
	else if (command == quot(Proto::REQUEST_MEDIA_ADDRESSES::NAME))
	{
		return CommandType::RequestMediaAddresses;
	}
	else if (command == quot(Proto::MEDIA_ADDRESSES_LIST::NAME))
	{
		return CommandType::MediaAddressesList;
	}

	return CommandType::Undefined;
}

}
