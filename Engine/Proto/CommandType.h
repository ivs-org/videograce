/**
 * CommandType.h - Contains protocol command types enumeration and function
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016 - 2021
 */

#pragma once

#include <string>

namespace Proto
{
	enum class CommandType
	{
		Undefined = 0,

		UserUpdateRequest,
		UserUpdateResponse,

		CredentialsRequest,
		CredentialsResponse,

		ConnectRequest,
		ConnectResponse,
		Disconnect,
		ChangeServer,

		Ping,

		SetMaxBitrate,

		UpdateGrants,

		ContactList,
		SearchContact,

		ContactsUpdate,

		GroupList,

		ConferencesList,

		DeviceParams,

		DeviceConnect,
		DeviceDisconnect,

		RendererConnect,
		RendererDisconnect,

		ResolutionChange,

		MicrophoneActive,

		CallRequest,
		CallResponse,

		ConferenceUpdateRequest,
		ConferenceUpdateResponse,

		CreateTempConference,

		SendConnectToConference,

		ConnectToConferenceRequest,
		ConnectToConferenceResponse,
		DisconnectFromConference,

		ChangeContactState,

		TurnSpeaker,
		ChangeMemberState,
		MemberAction,

		WantSpeak,

		ScheduleConnect,

		DeliveryMessages,
		LoadMessages,

		RequestMediaAddresses,
		MediaAddressesList
	};

	CommandType GetCommandType(const std::string &message);
}
