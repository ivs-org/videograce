/**
 * IController.h - Contains controller's interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014 - 2020
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <Proto/DeviceType.h>
#include <Proto/Member.h>
#include <Proto/Conference.h>
#include <Proto/CmdCallRequest.h>
#include <Proto/CmdCallResponse.h>
#include <Proto/CmdMemberAction.h>
#include <Proto/CmdMicrophoneActive.h>
#include <Proto/CmdUserUpdateRequest.h>
#include <Proto/CmdConferenceUpdateResponse.h>
#include <Proto/CmdSendConnectToConference.h>
#include <Proto/Group.h>

#include <Video/Resolution.h>
#include <Video/ColorSpace.h>

#include <Storage/Storage.h>

namespace Controller
{
	enum class State
	{
		Undefined = 0,

		NetworkError,
		UpdateRequired,
		CredentialsError,
		ServerError,

		Initial,
		Ready,
		Conferencing,
		Ended
	};

	struct DeviceValues
	{
		uint32_t pos;

		uint32_t deviceId;
		uint64_t clientId;
		uint32_t receiverSSRC, authorSSRC;

		uint32_t rate;
		Video::Resolution resolution;
		Video::ColorSpace colorSpace;

		Proto::DeviceType type;
		std::string name;

		std::string metadata;

		std::string addr; /// Translator address
		uint16_t port;    /// Translator port

		bool mySource;
		bool disabled;

		std::string secureKey;

		DeviceValues()
			: pos(0),
			deviceId(0),
			clientId(0),
			receiverSSRC(0), authorSSRC(0),
			rate(256),
			resolution(Video::rVGA),
			colorSpace(Video::ColorSpace::I420),
			type(Proto::DeviceType::Undefined),
			name(),
			metadata(),
			addr(),
			port(0),
			mySource(false),
			disabled(false),
			secureKey()
		{
		}
	};

	struct CallValues
	{
		Proto::CALL_REQUEST::Type requestType;
		Proto::CALL_RESPONSE::Type responseType;

		int64_t subscriberId;
		uint32_t subscriberConnectionId;

		std::string name;

		time_t timeLimit;
		
		CallValues()
			: requestType(Proto::CALL_REQUEST::Type::Undefined), responseType(Proto::CALL_RESPONSE::Type::Undefined), subscriberId(0), subscriberConnectionId(0), name(), timeLimit(0)
		{
		}

		CallValues(Proto::CALL_RESPONSE::Type responseType_, int64_t requesterId_, uint32_t subscriberConnectionId_, const std::string &name_)
			: requestType(Proto::CALL_REQUEST::Type::Undefined), responseType(responseType_), subscriberId(requesterId_), subscriberConnectionId(subscriberConnectionId_), name(name_), timeLimit(0)
		{
		}
	};

	struct Event
	{
		enum class Type
		{
			Undefined = 0,
			LogonSuccess,
			AuthNeeded,
			UpdateRequired,
			ServerFull,
			MemoryError,
			OvertimeCoding,
			OvertimeRendering,
			DeviceResolutionChanged,
			Disconnected,
            NetworkError,
			Updated,
			ServerOutdated,
			ServerChanged,
			UserUpdateResponse,
			
			GrantsUpdated,
			
			CameraCreated,
			CameraStarted,
			MicrophoneCreated,
			MicrophoneStarted,
			MicrophoneSpeak,
			MicrophoneSilent,
		
			ResolutionChanged,

			MicrophoneActive,
			
			DisconnectedFromConference,
			ConferenceLicenseFull,
			ConferenceNotExists,
			ConferenceNotAllowed,
			ConferenceConnected,
			ServerInternalError,

			DeviceConnect,
			DeviceDisconnect,

			CameraError,
			MicrophoneError,
			DeviceEnded,
			
			CallRequest,
			CallResponse,

			ResponderNotRegistered,
			ResponderNotConnected,
			AutoCallDenied,

			ConferenceCreated,
			StartConnectToConference,
			DoDisconnectFromConference,

			ScheduleConnect,

			MemberAdded,
			MemberRemoved,

			DisallowedActionInThisConference,
			AcceptedAction,
			RejectedAction,
			BusyAction,

			ActionTurnCamera,
			ActionTurnMicrophone,
			ActionTurnDemonstration,
			ActionTurnSpeaker,
			ActionEnableRemoteControl,
			ActionDisableRemoteControl,
            ActionMuteMicrophone,
			ActionDisconnectFromConference,

			WantSpeak,

			AddRTPAddress,
			ReceiveTCPMediaAddress,
			ReadyToMakeMediaTest,

			UpdateMembers
		};

		Type type;

		DeviceValues deviceValues;
		CallValues callValues;
		Proto::Conference conference;
		
		int64_t iData;
		std::string data;

		Event()
			: type(Type::Undefined), deviceValues(), callValues(), conference(), iData(-1), data()
		{}

		explicit Event(Type type_)
			: type(type_), deviceValues(), callValues(), conference(), iData(-1), data()
		{}
	};

	class IController
	{
	public:
		virtual void Connect(const std::string &serverAddress, bool secureConnection) = 0;
		virtual void Disconnect() = 0;
		virtual bool Connected() = 0;

		virtual void UserUpdate(int64_t userId, const std::string &name = "", const std::string &avatar = "", const std::string &login = "", const std::string &password = "") = 0;

		/// Set login / password from user input
		virtual void SetCredentials(const std::string &login, const std::string &password) = 0;

		/// Do update
		virtual void Update(const std::string &fileName, const std::string &appFolder = "") = 0;

		/// Set maximum bitrates
		virtual void SetMaxBitrate(uint32_t value) = 0;

		/// Devices manipulation
		virtual void CreateCapturer(const DeviceValues &device) = 0;
		virtual void ConnectCapturer(const DeviceValues &device) = 0;
		virtual void DeleteCapturer(uint32_t deviceId) = 0;
		
		/// Get the conference values to connecting
		virtual void ConnectToConference(const Proto::Conference &conference, bool hasCamera, bool hasMicrophone, bool hasDemonstration) = 0;

		/// Disconnecting from conference
		virtual void DisconnectFromConference(std::vector<DeviceValues> capturers, std::vector<DeviceValues> renderers) = 0;

		/// Connect renderer to translator
		virtual void ConnectRenderer(uint32_t deviceId, uint32_t ssrc) = 0;

		/// Disconnect the renderer
		virtual void DisconnectRenderer(uint32_t deviceId, uint32_t ssrc) = 0;
		
		/// Change resolution notifys
		virtual void ChangeResolution(uint32_t deviceId, Video::Resolution resolution) = 0;

		/// Microphone active notify
		virtual void MicrophoneActive(uint32_t deviceId, Proto::MICROPHONE_ACTIVE::ActiveType activeType) = 0;

		/// Send the call request
		virtual void CallRequest(CallValues values) = 0;

		/// Send the call response
		virtual void CallResponse(CallValues values) = 0;

		/// Add contact to private group (local contact list)
		virtual void AddContact(int64_t clientId) = 0;

        /// Remove contact to private group (local contact list)
		virtual void DeleteContact(int64_t clientId) = 0;

		/// Send create conference action
		virtual Proto::CONFERENCE_UPDATE_RESPONSE::Command CreateConference(const Proto::Conference &conference) = 0;

		/// Send edit conference action
		virtual Proto::CONFERENCE_UPDATE_RESPONSE::Command EditConference(const Proto::Conference &conference) = 0;

		/// Send delete conference action
		virtual Proto::CONFERENCE_UPDATE_RESPONSE::Command DeleteConference(int64_t conferenceId) = 0;

        /// Add me to confrence's members
        virtual Proto::CONFERENCE_UPDATE_RESPONSE::Command AddMeToConference(const std::string &tag) = 0;

        /// Remove me from confrence's members
        virtual Proto::CONFERENCE_UPDATE_RESPONSE::Command DeleteMeFromConference(int64_t conferenceId) = 0;

		/// Send create temp conference action
		virtual void CreateTempConference() = 0;

		/// Send connect to conference message to subscriber
		virtual void SendConnectToConference(const std::string &tag, int64_t connecter_id, uint32_t connecter_connection_id, uint32_t flags) = 0;

		/// Update group list
		virtual void UpdateGroupList() = 0;

		/// Update contact list statuses
		virtual void UpdateContactList() = 0;

		/// Search the contact
		virtual void SearchContact(const std::string &name) = 0;

        /// Return the finded contacts after call SearchContact()
        virtual const Storage::Contacts &GetFindedContects() = 0;

		/// Update the conferences list
		virtual void UpdateConferencesList() = 0;

		/// Send the action to member
		virtual void SendMemberAction(const std::vector<int64_t> &members, Proto::MEMBER_ACTION::Action action, uint32_t grants = 0) = 0;
		virtual void SendMemberActionResult(int64_t member, Proto::MEMBER_ACTION::Result result) = 0;

		/// Send turning the speaker
		virtual void SendTurnSpeaker() = 0;

		/// Send want to speaking
		virtual void SendWantSpeak() = 0;

		/// Load the IM messages
		virtual void LoadMessages(uint64_t fromDT) = 0;

		/// Delivery the IM messages
		virtual void DeliveryMessages(const Storage::Messages &messages) = 0;

		/// Send request to the server to getting addreses of the media sockets
		virtual void RequestMediaAddresses() = 0;
		
		/// Return the currect mashine state
		virtual State GetState() const = 0;

		/// Get is the current connection is secure
		virtual bool IsSecureConnection() const = 0;

		/// Get the current server's address
		virtual std::string GetServerAddress() const = 0;

		/// Get the current login
		virtual std::string GetLogin() const = 0;

		/// Get the current password
		virtual std::string GetPassword() const = 0;

		/// Return the currect user's grants
		virtual uint32_t GetGrants() const = 0;

		/// Return the client id
		virtual int64_t GetMyClientId() const = 0;

		/// Return the client name
		virtual std::string GetMyClientName() const = 0;

		/// Return the server name
		virtual std::string GetServerName() const = 0;

		/// Return the current conference values
		virtual Proto::Conference &GetCurrentConference() = 0;

		/// Return reduced frame rate when user is listening
		virtual uint16_t GetReducedFrameRate() = 0;

	protected:
		~IController() {}
	};
}
