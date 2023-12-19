/**
 * Controller.cpp - Contains controller's impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014 - 2020
 */

#include <algorithm>
#include <fstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <Version.h>

#include <Common/BitHelpers.h>
#include <Common/FSHelpers.h>
#include <Common/md5.h>
#include <Common/MACAddresses.h>
#include <Common/Process.h>

#include <Proto/CommandType.h>
#include <Proto/CmdUserUpdateRequest.h>
#include <Proto/CmdUserUpdateResponse.h>
#include <Proto/CmdConnectRequest.h>
#include <Proto/CmdConnectResponse.h>
#include <Proto/CmdDisconnect.h>
#include <Proto/CmdChangeServer.h>
#include <Proto/CmdPing.h>
#include <Proto/CmdUpdateGrants.h>
#include <Proto/CmdSetMaxBitrate.h>
#include <Proto/CmdContactList.h>
#include <Proto/CmdGroupList.h>
#include <Proto/CmdSearchContact.h>
#include <Proto/CmdConferencesList.h>
#include <Proto/CmdDeviceParams.h>
#include <Proto/CmdDeviceConnect.h>
#include <Proto/CmdDeviceDisconnect.h>
#include <Proto/CmdRendererConnect.h>
#include <Proto/CmdRendererDisconnect.h>
#include <Proto/CmdResolutionChange.h>
#include <Proto/CmdCallRequest.h>
#include <Proto/CmdCallResponse.h>
#include <Proto/CmdContactsUpdate.h>
#include <Proto/CmdConferenceUpdateRequest.h>
#include <Proto/CmdConferenceUpdateResponse.h>
#include <Proto/CmdCreateTempConference.h>
#include <Proto/CmdSendConnectToConference.h>
#include <Proto/CmdConnectToConferenceRequest.h>
#include <Proto/CmdConnectToConferenceResponse.h>
#include <Proto/CmdDisconnectFromConference.h>
#include <Proto/CmdChangeContactState.h>
#include <Proto/CmdChangeMemberState.h>
#include <Proto/CmdMemberAction.h>
#include <Proto/CmdTurnSpeaker.h>
#include <Proto/CmdWantSpeak.h>
#include <Proto/CmdScheduleConnect.h>
#include <Proto/CmdLoadMessages.h>
#include <Proto/CmdDeliveryMessages.h>
#include <Proto/CmdRequestMediaAddresses.h>
#include <Proto/CmdMediaAddressesList.h>
#include <Proto/MemberGrants.h>

#include <Transport/HTTP/HttpClient.h>
#include <Transport/URI/URI.h>

#include "Controller.h"

#ifdef WIN32
#include <ShlObj.h>
#include <shellapi.h>
#endif

#include <boost/nowide/convert.hpp>
#include <codecvt>

namespace Controller
{

const char* Controller::toString(State state)
{
    switch (state)
    {
        case State::Undefined: return "Undefined";
        case State::NetworkError: return "NetworkError";
        case State::UpdateRequired: return "UpdateRequired";
        case State::CredentialsError: return "CredentialsError";
        case State::ServerError: return "ServerError";
        case State::Initial: return "Initial";
        case State::Ready: return "Ready";
        case State::Ended: return "Ended";
        case State::Conferencing: return "Conferencing";
    };
    return "";
}

const char* Controller::toString(Controller::DisconnectReason reason)
{
    switch (reason)
    {
        case Controller::DisconnectReason::NetworkError: return "NetworkError";
        case Controller::DisconnectReason::Redirected: return "Redirected";
        case Controller::DisconnectReason::AuthNeeded: return "AuthNeeded";
        case Controller::DisconnectReason::InvalidCredentionals: return "InvalidCredentionals";
        case Controller::DisconnectReason::UpdateRequired: return "UpdateRequired";
        case Controller::DisconnectReason::ServerOutdated: return "ServerOutdated";
        case Controller::DisconnectReason::ServerFull: return "ServerFull";
        case Controller::DisconnectReason::InternalServerError: return "InternalServerError";
    };
    return "";
}

/// Class impl

Controller::Controller(Storage::Storage &storage_, IMemberList &memberList_)
    : eventHandler(),
    conferenceUpdateHandler(),
    contactListHandler(),
    storage(storage_),
    memberList(memberList_),
    state(State::Initial),
    grants(0),
    maxOutputBitrate(0),
    reducedFrameRate(5),
    callStart(0),
    currentConference(),
    subscriberId(0),
    baseURL(),
    webSocket(std::bind(&Controller::OnWebSocket, this, std::placeholders::_1, std::placeholders::_2)),
    serverAddress(), secureConnection(false),
    login(), password(),
    clientId(0), connectionId(0),
    clientName(),
    secureKey(),
    serverName(),
    disconnectReason(DisconnectReason::NetworkError),
    pinging(false),
    pinger(),
    lastPingTime(0),
    pingTimeMeter(),
    updater(),
    contactListReceving(ContactListReceiving::Update),
    sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
}

Controller::~Controller()
{
    Disconnect();

    if (pinger.joinable()) pinger.join();
    if (updater.joinable()) updater.join();
}

void Controller::SetEventHandler(std::function<void(const Event &ev)> handler_)
{
    eventHandler = handler_;
}

void Controller::SetConferenceUpdateHandler(std::function<void(const Proto::CONFERENCE_UPDATE_RESPONSE::Command&)> handler)
{
    conferenceUpdateHandler = handler;
}

void Controller::SetContactListHandler(std::function<void(const Storage::Contacts&)> handler)
{
    contactListHandler = handler;
}

void Controller::Connect(std::string_view serverAddress_, bool secureConnection_)
{
    sysLog->info("Controller :: Connect");
    
    serverAddress = serverAddress_;
    secureConnection = secureConnection_;

    ChangeState(State::Initial);

    if (serverAddress.empty())
    {
        Event event_;
        event_.type = Event::Type::AuthNeeded;
        event_.iData = 0;
        eventHandler(event_);
        return;
    }

    baseURL = (secureConnection ? std::string("https://") : std::string("http://")) + serverAddress;

    webSocket.Connect(baseURL);
}

void Controller::Disconnect(DisconnectReason disconnectReason_)
{
    sysLog->info("Controller :: Disconnect, reason {0}", toString(disconnectReason_));

    disconnectReason = disconnectReason_;

    std::thread([this]() {
        if (webSocket.IsConnected()) {
            webSocket.Send(Proto::DISCONNECT::Command().Serialize());
        }
        PingerStop();
        webSocket.Disconnect();
    }).detach();
}

bool Controller::Connected()
{
    return webSocket.IsConnected();
}

void Controller::UserUpdate(int64_t userId, std::string_view name_, std::string_view avatar_, std::string_view login_, std::string_view password_)
{
    SendCommand(Proto::USER_UPDATE_REQUEST::Command(Proto::USER_UPDATE_REQUEST::Action::ChangeMeta,
        userId,
        name_,
        avatar_,
        login_,
        password_).Serialize());
}

void Controller::Disconnect()
{
    Logout();
}

void Controller::SetCredentials(std::string_view login_, std::string_view password_)
{
    login = login_;
    password = password_;
}

void Controller::PingerStart()
{
    sysLog->trace("Controller::PingerStart :: Perform starting");

    if (!pinging)
    {
        pinging = true;

        pinger = std::thread([this]() {
            sysLog->trace("Controller::PingerStart :: Started");

            uint16_t cnt = 0;
            
            pingTimeMeter.Reset();
            lastPingTime = pingTimeMeter.Measure();

            while (pinging)
            {
                if (++cnt > 8)
                {
                    SendCommand(Proto::PING::Command().Serialize());
                    cnt = 0;
                }

                /*if (lastPingTime + 5000000 < pingTimeMeter.Measure())
                {
                    errLog->error("Controller :: Connection to server ping timeouted last ping: {0}, now {1}", lastPingTime, pingTimeMeter.Measure());
                    return Disconnect(DisconnectReason::NetworkError);
                }*/

                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }
        });
    }
}

void Controller::PingerStop()
{
    sysLog->trace("Controller::PingerStop :: Perform stoping");

    pinging = false;
    if (pinger.joinable()) pinger.join();

    sysLog->trace("Controller::PingerStop :: Stoped");
}

std::string GetDefaultAddress(std::string_view serverAddress)
{
    std::string serverAddress_ = "proto://" + std::string(serverAddress);

    std::string proto, host, port;

    Transport::ParseURI(serverAddress_, proto, host, port);
    
    return host;
}

void Controller::CreateCapturer(const DeviceValues &device)
{
    SendCommand(Proto::DEVICE_PARAMS::Command(0,
        0,
        device.type,
        device.pos,
        device.name,
        device.metadata,
        device.resolution,
        device.colorSpace).Serialize());
}

void Controller::ConnectCapturer(const DeviceValues &device)
{
    SendCommand(Proto::DEVICE_CONNECT::Command(Proto::DEVICE_CONNECT::ConnectType::CreatedDevice,
        device.type,
        device.deviceId,
        clientId,
        device.metadata,
        0,
        device.authorSSRC,
        "", 0,
        device.name,
        device.resolution,
        device.colorSpace,
        false, "").Serialize());
}

void Controller::DeleteCapturer(uint32_t deviceId)
{
    SendCommand(Proto::DEVICE_DISCONNECT::Command(Proto::DeviceType::Undefined, deviceId, 0).Serialize());
}

void Controller::ConnectToConference(const Proto::Conference &conference, bool hasCamera, bool hasMicrophone, bool hasDemonstration)
{
    if (GetState() < State::Ready)
    {
        return errLog->error("Controller::ConnectToConference can't work in state {0}", toString(GetState()));
    }

    currentConference = conference;

    SendCommand(Proto::CONNECT_TO_CONFERENCE_REQUEST::Command(conference.tag, conference.connect_members, hasCamera, hasMicrophone, hasDemonstration).Serialize());
}

void Controller::DisconnectFromConference(std::vector<DeviceValues> capturers, std::vector<DeviceValues> renderers)
{
    /// Disconnecting all renderers
    for (const auto &renderer : renderers)
    {
        SendCommand(Proto::RENDERER_DISCONNECT::Command(renderer.deviceId, renderer.receiverSSRC).Serialize());
    }

    /// Disconnecting all capturers
    for (const auto &capturer : capturers)
    {
        SendCommand(Proto::DEVICE_DISCONNECT::Command(Proto::DeviceType::Undefined, capturer.deviceId, 0).Serialize());
    }
        
    /// Disconnecting from conference
    SendCommand(Proto::DISCONNECT_FROM_CONFERENCE::Command().Serialize());

    /// Writing the call to history
    time_t duration = time(0) - callStart;

    storage.AddMessage(Proto::Message(Storage::GUID(),
        time(0),
        Proto::MessageType::Call,
        subscriberId, "",
        subscriberId, "",
        subscriberId, "",
        currentConference.tag, currentConference.name,
        Proto::MessageStatus::Created,
        "{\"type\":\"system_notice\",\"event\":\"call_ended\",\"duration\":" + std::to_string(duration) + "}",
        (int32_t)duration, Proto::CallResult::Answered,
        "", "", ""));

    subscriberId = 0;

    auto conferenceId = currentConference.id;

    currentConference.Clear();

    if (state == State::Conferencing)
    {
        ChangeState(State::Ready);
    }

    Event event_;
    event_.type = Event::Type::DisconnectedFromConference;
    event_.iData = conferenceId;
    eventHandler(event_);
}

void Controller::ConnectRenderer(uint32_t deviceId, uint32_t ssrc)
{
    SendCommand(Proto::RENDERER_CONNECT::Command(deviceId, ssrc).Serialize());
}

void Controller::DisconnectRenderer(uint32_t deviceId, uint32_t ssrc)
{
    SendCommand(Proto::RENDERER_DISCONNECT::Command(deviceId, ssrc).Serialize());
}

void Controller::ChangeResolution(uint32_t deviceId, Video::Resolution resolution)
{
    SendCommand(Proto::RESOLUTION_CHANGE::Command(deviceId, resolution).Serialize());
}

void Controller::MicrophoneActive(uint32_t deviceId, Proto::MICROPHONE_ACTIVE::ActiveType activeType)
{
    SendCommand(Proto::MICROPHONE_ACTIVE::Command(activeType, deviceId, clientId).Serialize());
}

void Controller::CallRequest(CallValues values)
{
    SendCommand(Proto::CALL_REQUEST::Command(values.name, 0, 0, values.requestType, 0).Serialize());
}

void Controller::CallResponse(CallValues values)
{
    SendCommand(Proto::CALL_RESPONSE::Command(values.subscriberId, values.subscriberConnectionId, "", values.responseType, 0).Serialize());

    /// Write to the history
    Proto::CallResult callResult = Proto::CallResult::Undefined;
    std::string eventType;
    switch (values.responseType)
    {
        case Proto::CALL_RESPONSE::Type::Accept:
            eventType = "call_start";
            callResult = Proto::CallResult::Answered;
        break;
        case Proto::CALL_RESPONSE::Type::NotConnected: callResult = Proto::CallResult::Offline;
            eventType = "subscriber_offline";
            callResult = Proto::CallResult::Offline;
            subscriberId = 0;
        break;
        case Proto::CALL_RESPONSE::Type::Refuse:
            eventType = "you_reject_call";
            callResult = Proto::CallResult::Rejected;
            subscriberId = 0;
        break;
        case Proto::CALL_RESPONSE::Type::Busy:
            eventType = "subscriber_busy";
            callResult = Proto::CallResult::Busy;
            subscriberId = 0;
        break;
        case Proto::CALL_RESPONSE::Type::Timeout:
            eventType = "missed_call";
            callResult = Proto::CallResult::Missed;
            subscriberId = 0;
        break;
        default: break;
    }

    auto msg = Proto::Message(Storage::GUID(),
        time(0),
        Proto::MessageType::Call,
        values.subscriberId, "",
        values.subscriberId, "",
        values.subscriberId, "",
        currentConference.tag, currentConference.name,
        Proto::MessageStatus::Created,
        "{\"type\":\"system_notice\",\"event\":\"" + eventType + "\"}",
        0, callResult,
        "", "", "");

    storage.AddMessage(msg);

    auto absentContacts = storage.GetAbsentContacts({ msg });
    if (!absentContacts.empty())
    {
        AddContact(absentContacts[0]);
    }
}

void Controller::AddContact(int64_t clientId)
{
    if (clientId == 0)
    {
        return errLog->critical("Controller::AddContact() clientId == 0");
    }
    SendCommand(Proto::CONTACTS_UPDATE::Command(
        Proto::CONTACTS_UPDATE::Action::Add,
        clientId
    ).Serialize());
}

void Controller::DeleteContact(int64_t clientId)
{
    SendCommand(Proto::CONTACTS_UPDATE::Command(
        Proto::CONTACTS_UPDATE::Action::Delete,
        clientId
    ).Serialize());
}

void Controller::CreateConference(const Proto::Conference &conference)
{
    SendCommand(Proto::CONFERENCE_UPDATE_REQUEST::Command(
        Proto::CONFERENCE_UPDATE_REQUEST::Action::Create,
        conference
    ).Serialize());
}

void Controller::EditConference(const Proto::Conference &conference)
{
    SendCommand(Proto::CONFERENCE_UPDATE_REQUEST::Command(
        Proto::CONFERENCE_UPDATE_REQUEST::Action::Edit,
        conference
    ).Serialize());
}

void Controller::DeleteConference(int64_t conferenceId)
{
    SendCommand(Proto::CONFERENCE_UPDATE_REQUEST::Command(
        Proto::CONFERENCE_UPDATE_REQUEST::Action::Delete,
        Proto::Conference(conferenceId)
    ).Serialize());
}

void Controller::AddMeToConference(std::string_view tag)
{
    SendCommand(Proto::CONFERENCE_UPDATE_REQUEST::Command(
        Proto::CONFERENCE_UPDATE_REQUEST::Action::AddMe,
        Proto::Conference(tag)
    ).Serialize());
}

void Controller::DeleteMeFromConference(int64_t conferenceId)
{
    SendCommand(Proto::CONFERENCE_UPDATE_REQUEST::Command(
        Proto::CONFERENCE_UPDATE_REQUEST::Action::DeleteMe,
        Proto::Conference(conferenceId)
    ).Serialize());
}

void Controller::CreateTempConference()
{
    std::vector<std::string> macs;
    Common::GetMacAddresses(macs);

    std::string confTag;

    if (!macs.empty())
    {
        confTag = md5(macs[0].c_str());
    }
    else
    {
        confTag = md5(boost::uuids::to_string(boost::uuids::random_generator()()).c_str());
    }

    SendCommand(Proto::CREATE_TEMP_CONFERENCE::Command(confTag).Serialize());
}

void Controller::SendConnectToConference(std::string_view tag, int64_t connecter_id, uint32_t connecter_connection_id, uint32_t flags)
{
    SendCommand(Proto::SEND_CONNECT_TO_CONFERENCE::Command(tag, connecter_id, connecter_connection_id, flags).Serialize());
}

void Controller::UpdateGroupList()
{
    SendCommand(Proto::GROUP_LIST::Command().Serialize());
}

void Controller::UpdateContactList()
{
    contactListReceving = ContactListReceiving::Update;
    SendCommand(Proto::SEARCH_CONTACT::Command("==UPDATE==").Serialize());
}

void Controller::SearchContact(std::string_view name)
{
    if (Connected())
    {
        contactListReceving = ContactListReceiving::Search;
        SendCommand(Proto::SEARCH_CONTACT::Command(name).Serialize());
    }
}

void Controller::UpdateConferencesList()
{
    SendCommand(Proto::CONFERENCES_LIST::Command().Serialize());
}

void Controller::SetMaxBitrate(uint32_t value)
{
    SendCommand(Proto::SET_MAX_BITRATE::Command(value).Serialize());
}

void Controller::SendMemberAction(const std::vector<int64_t> &members, Proto::MEMBER_ACTION::Action action, uint32_t grants)
{
    SendCommand(Proto::MEMBER_ACTION::Command(members, action, grants).Serialize());
}

void Controller::SendMemberActionResult(int64_t member, Proto::MEMBER_ACTION::Result result)
{
    SendCommand(Proto::MEMBER_ACTION::Command(member, result).Serialize());
}

void Controller::SendTurnSpeaker()
{
    SendCommand(Proto::TURN_SPEAKER::Command().Serialize());
}

void Controller::SendWantSpeak()
{
    SendCommand(Proto::WANT_SPEAK::Command().Serialize());
}

void Controller::LoadMessages(uint64_t fromDT)
{
    SendCommand(Proto::LOAD_MESSAGES::Command(fromDT).Serialize());
}

void Controller::DeliveryMessages(const Storage::Messages &messages)
{
    Proto::DELIVERY_MESSAGES::Command cmd;
    cmd.messages = messages;
    SendCommand(cmd.Serialize());
}

void Controller::RequestMediaAddresses()
{
    SendCommand(Proto::REQUEST_MEDIA_ADDRESSES::Command().Serialize());
}

State Controller::GetState() const
{
    return state;
}

bool Controller::IsSecureConnection() const
{
    return secureConnection;
}

std::string Controller::GetServerAddress() const
{
    return serverAddress;
}

std::string Controller::GetLogin() const
{
    return login;
}

std::string Controller::GetPassword() const
{
    return password;
}

uint32_t Controller::GetGrants() const
{
    return grants;
}

uint32_t Controller::GetMaxOutputBitrate() const
{
    return maxOutputBitrate;
}

int64_t Controller::GetMyClientId() const
{
    return clientId;
}

std::string Controller::GetMyClientName() const
{
    return clientName;
}

std::string Controller::GetServerName() const
{
    return serverName;
}

Proto::Conference &Controller::GetCurrentConference()
{
    return currentConference;
}

uint16_t Controller::GetReducedFrameRate()
{
    return reducedFrameRate;
}

void Controller::OnWebSocket(Transport::WSMethod method, std::string_view message)
{
    switch (method)
    {
        case Transport::WSMethod::Open:
        {
            sysLog->info("Controller :: Connection to server established");

            Logon();
        }
        break;
        case Transport::WSMethod::Message:
        {
            sysLog->trace("Controller::WebSocket recv message: {0}", message);

            auto commandType = Proto::GetCommandType(message);
            switch (commandType)
            {
                case Proto::CommandType::ConnectResponse:
                {
                    Proto::CONNECT_RESPONSE::Command cmd;
                    cmd.Parse(message);

                    switch (cmd.result)
                    {
                        case Proto::CONNECT_RESPONSE::Result::OK:
                            if (cmd.server_version < SERVER_VERSION)
                            {
                                ChangeState(State::ServerError);
                                errLog->critical("Controller :: Server version is outdated\n");
                                return Disconnect(DisconnectReason::ServerOutdated);
                            }

                            if (!secureConnection && !cmd.secure_key.empty())
                            {
                                secureConnection = true;
                                
                                Event event_;
                                event_.type = Event::Type::ServerChanged;
                                event_.data = serverAddress;
                                event_.iData = secureConnection ? 1 : 0;
                                eventHandler(event_);

                                return Disconnect(DisconnectReason::Redirected);
                            }
                            else if (secureConnection == 1 && cmd.secure_key.empty())
                            {
                                secureConnection = false;
                                
                                Event event_;
                                event_.type = Event::Type::ServerChanged;
                                event_.data = serverAddress;
                                event_.iData = secureConnection ? 1 : 0;
                                eventHandler(event_);

                                return Disconnect(DisconnectReason::Redirected);
                            }

                            clientId = cmd.id;
                            connectionId = cmd.connection_id;
                            clientName = cmd.name;
                            secureKey = cmd.secure_key;
                            serverName = cmd.server_name;
                            grants = cmd.grants;
                            maxOutputBitrate = cmd.max_output_bitrate;
                            reducedFrameRate = cmd.reduced_frame_rate;
                            
                            ChangeState(State::Ready);

                            PingerStart();
                            
                            eventHandler(Event(Event::Type::LogonSuccess));
                        break;
                        case Proto::CONNECT_RESPONSE::Result::InvalidCredentials:
                        {
                            ChangeState(State::CredentialsError);
                            errLog->critical("Controller :: Credentials error in connecting to server\n");
                            Disconnect(DisconnectReason::InvalidCredentionals);
                        }
                        break;
                        case Proto::CONNECT_RESPONSE::Result::UpdateRequired:
                            ChangeState(State::UpdateRequired);
                            sysLog->critical("Controller :: Update required\n");
                            Disconnect(DisconnectReason::UpdateRequired);
                        break;
                        case Proto::CONNECT_RESPONSE::Result::Redirect:
                        {
                            std::string proto, host, port;
                            Transport::ParseURI(cmd.redirect_url, proto, host, port);
                            if (!proto.empty())
                            {
                                serverAddress = host + ":" + port;
                                secureConnection = proto == "https";
                                
                                Event event_;
                                event_.type = Event::Type::ServerChanged;
                                event_.data = serverAddress;
                                event_.iData = secureConnection ? 1 : 0;
                                eventHandler(event_);

                                return Disconnect(DisconnectReason::Redirected);;
                            }
                            else
                            {
                                ChangeState(State::ServerError);
                                errLog->critical("Controller :: Server sent incorrect redirect URL\n");
                                return Disconnect(DisconnectReason::InternalServerError);
                            }
                        }
                        break;
                        case Proto::CONNECT_RESPONSE::Result::ServerFull:
                            ChangeState(State::ServerError);
                            sysLog->critical("Controller :: Server full\n");
                            Disconnect(DisconnectReason::ServerFull);
                        break;
                        case Proto::CONNECT_RESPONSE::Result::InternalServerError:
                            ChangeState(State::ServerError);
                            sysLog->critical("Controller :: Internal server error\n");
                            Disconnect(DisconnectReason::InternalServerError);
                        break;
                    }
                }
                break;
                case Proto::CommandType::UserUpdateResponse:
                {
                    Proto::USER_UPDATE_RESPONSE::Command cmd;
                    cmd.Parse(message);

                    Event event_;
                    event_.type = Event::Type::UserUpdateResponse;
                    event_.deviceValues.pos = static_cast<int32_t>(cmd.action);
                    event_.deviceValues.clientId = cmd.user_id;
                    event_.iData = static_cast<int32_t>(cmd.result);
                    event_.data = cmd.message;

                    eventHandler(event_);
                }
                break;
                case Proto::CommandType::UpdateGrants:
                {
                    Proto::UPDATE_GRANTS::Command cmd;
                    cmd.Parse(message);

                    grants = cmd.grants;

                    Event event_;
                    event_.type = Event::Type::GrantsUpdated;
                    event_.iData = grants;

                    eventHandler(event_);
                }
                break;
                case Proto::CommandType::DeviceParams:
                {
                    Proto::DEVICE_PARAMS::Command cmd;
                    cmd.Parse(message);

                    sysLog->info("Controller :: Received device params device_type {0:d} id {1:d}", static_cast<int32_t>(cmd.device_type), cmd.id);

                    Event event_;
                    event_.type = cmd.device_type == Proto::DeviceType::Microphone ? Event::Type::MicrophoneCreated : Event::Type::CameraCreated;
                    event_.deviceValues.deviceId = cmd.id;
                    event_.deviceValues.authorSSRC = cmd.ssrc;
                    event_.deviceValues.type = cmd.device_type;
                    event_.deviceValues.pos = cmd.ord;
                    event_.deviceValues.name = cmd.name;
                    event_.deviceValues.metadata = cmd.metadata;
                    event_.deviceValues.resolution = static_cast<Video::Resolution>(cmd.resolution);
                    event_.deviceValues.colorSpace = static_cast<Video::ColorSpace>(cmd.color_space);
                    eventHandler(event_);
                }
                break;
                case Proto::CommandType::DeviceConnect:
                {
                    Proto::DEVICE_CONNECT::Command cmd;
                    cmd.Parse(message);

                    sysLog->info("Controller :: Received device connect (connect_type: {0:d} device_id {1:d}, client_id: {2:d})", static_cast<int32_t>(cmd.connect_type), cmd.device_id, cmd.client_id);

                    switch (cmd.connect_type)
                    {
                        case Proto::DEVICE_CONNECT::ConnectType::CreatedDevice:
                        {
                            if (cmd.address.empty() || cmd.address == "0.0.0.0" || cmd.address == "::/")
                            {
                                cmd.address = GetDefaultAddress(serverAddress);
                            }

                            Event event_;
                            event_.type = cmd.device_type == Proto::DeviceType::Microphone ? Event::Type::MicrophoneStarted : Event::Type::CameraStarted;

                            event_.deviceValues.type = cmd.device_type;
                            event_.deviceValues.deviceId = cmd.device_id;
                            event_.deviceValues.authorSSRC = cmd.author_ssrc;
                            event_.deviceValues.colorSpace = static_cast<Video::ColorSpace>(cmd.color_space);
                            event_.deviceValues.addr = cmd.address;
                            event_.deviceValues.port = cmd.port;
                            event_.deviceValues.secureKey = secureKey;
                            eventHandler(event_);
                        }
                        break;
                        case Proto::DEVICE_CONNECT::ConnectType::ConnectRenderer:
                        {
                            Event event_;
                            event_.type = Event::Type::DeviceConnect;
                            event_.deviceValues.type = cmd.device_type;
                            event_.deviceValues.deviceId = cmd.device_id;
                            event_.deviceValues.clientId = cmd.client_id;
                            event_.deviceValues.metadata = cmd.metadata;
                            event_.deviceValues.receiverSSRC = cmd.receiver_ssrc;
                            event_.deviceValues.authorSSRC = cmd.author_ssrc;
                            event_.deviceValues.port = cmd.port;
                            event_.deviceValues.name = cmd.name;
                            event_.deviceValues.resolution = (Video::Resolution)cmd.resolution;
                            event_.deviceValues.mySource = cmd.my;
                            event_.deviceValues.addr = cmd.address;
                            event_.deviceValues.secureKey = cmd.secure_key;

                            if (event_.deviceValues.addr.empty() || event_.deviceValues.addr == "0.0.0.0" || event_.deviceValues.addr == "::/")
                            {
                                event_.deviceValues.addr = GetDefaultAddress(serverAddress);
                            }

                            eventHandler(event_);
                        }
                        break;
                        default: break;
                    }
                }
                break;
                case Proto::CommandType::DeviceDisconnect:
                {
                    Proto::DEVICE_DISCONNECT::Command cmd;
                    cmd.Parse(message);
                    
                    sysLog->info("Controller :: Received disconnect device (device_id: {0:d}), client_id: {1:d})", cmd.device_id, cmd.client_id);

                    Event event_;
                    event_.type = Event::Type::DeviceDisconnect;
                    event_.deviceValues.type = cmd.device_type;
                    event_.deviceValues.deviceId = cmd.device_id;
                    event_.deviceValues.clientId = cmd.client_id;
                    eventHandler(event_);
                }
                break;
                case Proto::CommandType::ResolutionChange:
                {
                    Proto::RESOLUTION_CHANGE::Command cmd;
                    cmd.Parse(message);

                    sysLog->info("Controller :: Received resolution changed (device_id: {0:d}, resolution: {1:d})", cmd.id, static_cast<uint32_t>(cmd.resolution));
                    Event event_;
                    event_.type = Event::Type::ResolutionChanged;
                    event_.deviceValues.deviceId = cmd.id;
                    event_.deviceValues.resolution = cmd.resolution;
                    eventHandler(event_);
                }
                break;
                case Proto::CommandType::MicrophoneActive:
                {
                    Proto::MICROPHONE_ACTIVE::Command cmd;
                    cmd.Parse(message);

                    sysLog->info("Controller :: Received microphone active (client_id: {0:d}, device_id: {1:d}, value: {2:d})", cmd.client_id, cmd.device_id, static_cast<uint32_t>(cmd.active_type));
                    Event event_;
                    event_.type = Event::Type::MicrophoneActive;
                    event_.deviceValues.clientId = cmd.client_id;
                    event_.deviceValues.deviceId = cmd.device_id;
                    event_.iData = static_cast<int32_t>(cmd.active_type);
                    eventHandler(event_);
                }
                break;
                case Proto::CommandType::CallRequest:
                {
                    Proto::CALL_REQUEST::Command cmd;
                    cmd.Parse(message);

                    sysLog->info("Controller :: Received call request (subscriber_id: {0:d}, type: {1:d})", cmd.id, static_cast<int32_t>(cmd.type));
                    
                    subscriberId = cmd.id;

                    Event event_;
                    event_.type = Event::Type::CallRequest;
                    event_.callValues.name = cmd.name;
                    event_.callValues.subscriberId = cmd.id;
                    event_.callValues.subscriberConnectionId = cmd.connection_id;
                    event_.callValues.requestType = cmd.type;
                    event_.callValues.timeLimit = cmd.time_limit;
                    eventHandler(event_);
                }
                break;
                case Proto::CommandType::CallResponse:
                {
                    Proto::CALL_RESPONSE::Command cmd;
                    cmd.Parse(message);
                    
                    sysLog->info("Controller :: Received call response (subscriber_id: {0:d}, type: {1:d})", cmd.id, static_cast<int32_t>(cmd.type));

                    switch (cmd.type)
                    {
                        case Proto::CALL_RESPONSE::Type::AutoCall:
                            eventHandler(Event(Event::Type::AutoCallDenied));
                        break;
                        case Proto::CALL_RESPONSE::Type::NotConnected:
                            eventHandler(Event(Event::Type::ResponderNotConnected));
                        break;
                        case Proto::CALL_RESPONSE::Type::Accept: case Proto::CALL_RESPONSE::Type::Refuse:
                        case Proto::CALL_RESPONSE::Type::Busy: case Proto::CALL_RESPONSE::Type::Timeout:
                        {
                            /// Writing call history
                            Proto::CallResult callResult = Proto::CallResult::Undefined;
                            std::string eventType;
                            switch (cmd.type)
                            {
                                case Proto::CALL_RESPONSE::Type::Accept:
                                    eventType = "call_start";
                                    callResult = Proto::CallResult::Answered;
                                    subscriberId = cmd.id;
                                break;
                                case Proto::CALL_RESPONSE::Type::Refuse:
                                    eventType = "subscriber_reject_call";
                                    callResult = Proto::CallResult::Rejected;
                                break;
                                case Proto::CALL_RESPONSE::Type::Busy:
                                    eventType = "subscriber_busy";
                                    callResult = Proto::CallResult::Busy;
                                break;
                                case Proto::CALL_RESPONSE::Type::Timeout:
                                    eventType = "subscriber_answer_timeout";
                                    callResult = Proto::CallResult::Missed;
                                break;
                            }

                            auto msg = Proto::Message(Storage::GUID(),
                                time(0),
                                Proto::MessageType::Call,
                                cmd.id, "",
                                cmd.id, "",
                                cmd.id, "",
                                "", "",
                                Proto::MessageStatus::Created,
                                "{\"type\":\"system_notice\",\"event\":\"" + eventType + "\"}",
                                0, callResult,
                                "", "", "");

                            storage.AddMessage(msg);

                            auto absentContacts = storage.GetAbsentContacts({ msg });
                            if (!absentContacts.empty())
                            {
                                AddContact(absentContacts[0]);
                            }

                            Event event_;
                            event_.type = Event::Type::CallResponse;
                            event_.callValues.subscriberId = cmd.id;
                            event_.callValues.subscriberConnectionId = cmd.connection_id;
                            event_.callValues.name = cmd.name;
                            event_.callValues.responseType = cmd.type;
                            event_.callValues.timeLimit = cmd.time_limit;
                            eventHandler(event_);
                        }
                        break;
                        default: break;
                    }
                }
                break;
                case Proto::CommandType::ConferenceUpdateResponse:
                {
                    Proto::CONFERENCE_UPDATE_RESPONSE::Command cmd;
                    cmd.Parse(message);

                    if (conferenceUpdateHandler) conferenceUpdateHandler(cmd);
                }
                break;
                case Proto::CommandType::CreateTempConference:
                {
                    Proto::CREATE_TEMP_CONFERENCE::Command cmd;
                    cmd.Parse(message);

                    Event event_;
                    event_.type = Event::Type::ConferenceCreated;
                    event_.conference.tag = cmd.tag;
                    eventHandler(event_);
                }
                break;
                case Proto::CommandType::SendConnectToConference:
                {
                    Proto::SEND_CONNECT_TO_CONFERENCE::Command cmd;
                    cmd.Parse(message);
                    
                    sysLog->info("Controller :: Received send connect to conference (tag: {0})", cmd.tag);
                    
                    Event event_;
                    event_.type = Event::Type::StartConnectToConference;
                    event_.conference.tag = cmd.tag;
                    event_.conference.grants = cmd.flags;
                    eventHandler(event_);
                }
                break;
                case Proto::CommandType::ConnectToConferenceResponse:
                {
                    Proto::CONNECT_TO_CONFERENCE_RESPONSE::Command cmd;
                    cmd.Parse(message);

                    sysLog->info("Controller :: Received connect to conference response (result: {0:d})", static_cast<int32_t>(cmd.result));

                    switch (cmd.result)
                    {
                        case Proto::CONNECT_TO_CONFERENCE_RESPONSE::Result::OK:
                            ChangeState(State::Conferencing);

                            currentConference.id = cmd.id;
                            currentConference.tag = cmd.tag;
                            currentConference.founder_id = cmd.founder_id;
                            currentConference.name = cmd.name;
                            currentConference.grants = cmd.grants;
                            currentConference.temp = cmd.temp;

                            sysLog->info("Controller :: Successfully connected to conference (tag: {0})", currentConference.tag);
                            
                            callStart = time(0);

                            eventHandler(Event(Event::Type::ConferenceConnected));

                            if (!currentConference.temp)
                            {
                                storage.AddMessage(Proto::Message(Storage::GUID(),
                                    time(0),
                                    Proto::MessageType::Call,
                                    0, "",
                                    0, "",
                                    0, "",
                                    currentConference.tag, currentConference.name,
                                    Proto::MessageStatus::Created,
                                    "{\"type\":\"system_notice\",\"event\":\"call_start\"}",
                                    0, Proto::CallResult::Answered,
                                    "", "", ""));
                            }
                        break;
                        case Proto::CONNECT_TO_CONFERENCE_RESPONSE::Result::NotAllowed:
                            ChangeState(State::Ready);
                            currentConference.Clear();
                            eventHandler(Event(Event::Type::ConferenceNotAllowed));
                        break;
                        case Proto::CONNECT_TO_CONFERENCE_RESPONSE::Result::NotExists:
                            ChangeState(State::Ready);
                            currentConference.Clear();
                            eventHandler(Event(Event::Type::ConferenceNotExists));
                        break;
                        case Proto::CONNECT_TO_CONFERENCE_RESPONSE::Result::LicenseFull:
                            ChangeState(State::Ready);
                            currentConference.Clear();
                            eventHandler(Event(Event::Type::ConferenceLicenseFull));
                        break;
                        default: break;
                    }
                }
                break;
                case Proto::CommandType::DisconnectFromConference:
                {
                    sysLog->info("Controller :: Received disconnect from conference");

                    if (GetState() == State::Conferencing)
                    {
                        eventHandler(Event(Event::Type::DoDisconnectFromConference));
                    }
                }
                break;
                case Proto::CommandType::ContactList:
                {
                    Proto::CONTACT_LIST::Command cmd;
                    cmd.Parse(message);

                    switch (contactListReceving)
                    {
                        case ContactListReceiving::Update:
                            storage.UpdateContacts(cmd.sort_type, cmd.show_numbers, cmd.members);

                            for (auto &member : cmd.members)
                            {
                                if (member.id == clientId)
                                {
                                    clientName = member.name;
                                    login = member.login;
                                    break;
                                }
                            }
                        break;
                        case ContactListReceiving::Search:
                            if (contactListHandler) contactListHandler(cmd.members);
                        break;
                        default: break;
                    }

                    contactListReceving = ContactListReceiving::Update;
                }
                break;
                case Proto::CommandType::ChangeContactState:
                {
                    Proto::CHANGE_CONTACT_STATE::Command cmd;
                    cmd.Parse(message);

                    storage.ChangeContactState(cmd.id, cmd.state);
                }
                break;
                case Proto::CommandType::GroupList:
                {
                    Proto::GROUP_LIST::Command cmd;
                    cmd.Parse(message);

                    storage.ClearGroups();
                    storage.UpdateGroups(cmd.groups);
                }
                break;
                case Proto::CommandType::ConferencesList:
                {
                    Proto::CONFERENCES_LIST::Command cmd;
                    cmd.Parse(message);

                    storage.UpdateConferences(cmd.conferences);

                    for (auto &c : cmd.conferences)
                    {
                        if (c.tag == currentConference.tag)
                        {
                            currentConference = c;
                            break;
                        }
                    }
                }
                break;
                case Proto::CommandType::ChangeMemberState:
                {
                    Proto::CHANGE_MEMBER_STATE::Command cmd;
                    cmd.Parse(message);

                    std::lock_guard<std::recursive_mutex> lock(memberList.GetItemsMutex());

                    for (auto &member : cmd.members)
                    {
                        sysLog->info("Controller :: Received changed member state (member_id: {0}, state: {1:d})", member.id, static_cast<int>(member.state));
                        if (member.state == Proto::MemberState::Conferencing)
                        {
                            if (!memberList.ExistsMember(member.id))
                            {									
                                Event event_;
                                event_.type = Event::Type::MemberAdded;
                                event_.iData = member.id;
                                event_.data = member.name;
                                event_.deviceValues.type = Proto::DeviceType::Camera;
                                event_.deviceValues.disabled = !member.has_camera;
                                eventHandler(event_);
                            }
                            memberList.UpdateItem(member);
                        }
                        else
                        {
                            memberList.DeleteItem(member.id);

                            Event event_;
                            event_.type = Event::Type::MemberRemoved;
                            event_.iData = member.id;
                            eventHandler(event_);
                        }
                    }

                    eventHandler(Event(Event::Type::UpdateMembers));
                }
                break;
                case Proto::CommandType::ScheduleConnect:
                {
                    Proto::SCHEDULE_CONNECT::Command cmd;
                    cmd.Parse(message);

                    Event event_;
                    event_.type = Event::Type::ScheduleConnect;
                    event_.conference.tag = cmd.tag;
                    event_.conference.name = cmd.name;
                    event_.conference.duration = cmd.time_limit;
                    eventHandler(event_);
                }
                break;
                case Proto::CommandType::MemberAction:
                {
                    Proto::MEMBER_ACTION::Command cmd;
                    cmd.Parse(message);

                    if (cmd.result == Proto::MEMBER_ACTION::Result::NotAllowed)
                    {
                        return eventHandler(Event(Event::Type::DisallowedActionInThisConference));
                    }
                    else if (cmd.result == Proto::MEMBER_ACTION::Result::Accepted)
                    {
                        Event event_;
                        event_.type = Event::Type::AcceptedAction;
                        event_.iData = cmd.actor_id;
                        event_.data = cmd.actor_name;
                        return eventHandler(event_);
                    }
                    else if (cmd.result == Proto::MEMBER_ACTION::Result::Rejected)
                    {
                        Event event_;
                        event_.type = Event::Type::RejectedAction;
                        event_.iData = cmd.actor_id;
                        event_.data = cmd.actor_name;
                        return eventHandler(event_);
                    }
                    else if (cmd.result == Proto::MEMBER_ACTION::Result::Busy)
                    {
                        Event event_;
                        event_.type = Event::Type::BusyAction;
                        event_.iData = cmd.actor_id;
                        event_.data = cmd.actor_name;
                        return eventHandler(event_);
                    }
                    
                    Event event_;
                    switch (cmd.action)
                    {
                        case Proto::MEMBER_ACTION::Action::TurnCamera:
                            event_.type = Event::Type::ActionTurnCamera;
                        break;
                        case Proto::MEMBER_ACTION::Action::TurnMicrophone:
                            event_.type = Event::Type::ActionTurnMicrophone;
                        break;
                        case Proto::MEMBER_ACTION::Action::TurnDemonstration:
                            event_.type = Event::Type::ActionTurnDemonstration;
                        break;
                        case Proto::MEMBER_ACTION::Action::EnableRemoteControl:
                            event_.type = Event::Type::ActionEnableRemoteControl;
                        break;
                        case Proto::MEMBER_ACTION::Action::DisableRemoteControl:
                            event_.type = Event::Type::ActionDisableRemoteControl;
                        break;
                        case Proto::MEMBER_ACTION::Action::TurnSpeaker:
                            event_.type = Event::Type::ActionTurnSpeaker;
                        break;
                        case Proto::MEMBER_ACTION::Action::MuteMicrophone:
                            event_.type = Event::Type::ActionMuteMicrophone;
                        break;
                        case Proto::MEMBER_ACTION::Action::DisconnectFromConference:
                            event_.type = Event::Type::ActionDisconnectFromConference;
                        break;
                    }

                    event_.iData = cmd.actor_id;
                    event_.data = cmd.actor_name;

                    eventHandler(event_);
                }
                break;
                case Proto::CommandType::WantSpeak:
                {
                    Proto::WANT_SPEAK::Command cmd;
                    cmd.Parse(message);

                    Event event_;
                    event_.type = Event::Type::WantSpeak;
                    event_.iData = cmd.user_id;
                    event_.data = cmd.user_name;
                    if (cmd.is_speak) SetBit(event_.conference.grants, static_cast<int32_t>(Proto::MemberGrants::Speaker)); else ClearBit(event_.conference.grants, static_cast<int32_t>(Proto::MemberGrants::Speaker));
                    eventHandler(event_);
                }
                break;
                case Proto::CommandType::DeliveryMessages:
                {
                    Proto::DELIVERY_MESSAGES::Command cmd;
                    cmd.Parse(message);

                    storage.UpdateMessages(cmd.messages);

                    auto absentContacts = storage.GetAbsentContacts(cmd.messages);
                    for (auto &c : absentContacts)
                    {
                        AddContact(c);
                    }
                }
                break;
                case Proto::CommandType::MediaAddressesList:
                {
                    Proto::MEDIA_ADDRESSES_LIST::Command cmd;
                    cmd.Parse(message);

                    for (const auto &a : cmd.addresses)
                    {
                        std::string proto, host, port;
                        Transport::ParseURI(a, proto, host, port);

                        if (host == "0.0.0.0" || host == "::/")
                        {
                            host = GetDefaultAddress(serverAddress);
                        }

                        Event event_;
                        event_.type = proto == "rtp" ? Event::Type::AddRTPAddress : Event::Type::ReceiveTCPMediaAddress;
                        event_.iData = atoi(port.c_str());
                        event_.data = host;
                        eventHandler(event_);
                    }

                    eventHandler(Event(Event::Type::ReadyToMakeMediaTest));
                }
                break;
                case Proto::CommandType::Disconnect:
                {
                    sysLog->info("Controller :: Received disconnect from server");

                    std::thread([this]() { Logout(); }).detach();
                }
                break;
                case Proto::CommandType::ChangeServer:
                {
                    Proto::CHANGE_SERVER::Command cmd;
                    cmd.Parse(message);

                    sysLog->info("Controller :: Received change server to url: {0}", cmd.url);

                    std::string proto, host, port;
                    Transport::ParseURI(cmd.url, proto, host, port);
                    if (!proto.empty())
                    {
                        serverAddress = host + ":" + port;
                        secureConnection = proto == "https";

                        Event event_;
                        event_.type = Event::Type::ServerChanged;
                        event_.data = serverAddress;
                        event_.iData = secureConnection ? 1 : 0;
                        eventHandler(event_);
                    }
                    else
                    {
                        ChangeState(State::ServerError);
                        errLog->critical("Controller :: Server sent incorrect redirect URL\n");
                    }

                    std::thread([this]() { Logout(); }).detach();
                }
                break;
                case Proto::CommandType::Undefined:
                break;
                case Proto::CommandType::Ping:
                    lastPingTime = pingTimeMeter.Measure();
                break;
                default: break;
            }
        }
        break;
        case Transport::WSMethod::Close:
            sysLog->info("Controller :: WebSocket closed (message: \"{0}\", reason: {1})", message, toString(disconnectReason));
            switch (disconnectReason)
            {
                case DisconnectReason::NetworkError: case DisconnectReason::Redirected:
                    eventHandler(Event(Event::Type::Disconnected));
                break;
                case DisconnectReason::AuthNeeded:
                {
                    Event event_;
                    event_.type = Event::Type::AuthNeeded;
                    event_.iData = 2;
                    eventHandler(event_);
                }
                break;
                case DisconnectReason::InvalidCredentionals:
                {
                    Event event_;
                    event_.type = Event::Type::AuthNeeded;
                    event_.iData = 1;
                    eventHandler(event_);
                }
                break;
                case DisconnectReason::UpdateRequired:
                    eventHandler(Event(Event::Type::UpdateRequired));
                break;
                case DisconnectReason::ServerOutdated:
                    eventHandler(Event(Event::Type::ServerOutdated));
                break;
                case DisconnectReason::ServerFull:
                    eventHandler(Event(Event::Type::ServerFull));
                break;
                case DisconnectReason::InternalServerError:
                    eventHandler(Event(Event::Type::ServerInternalError));
                break;
                default:
                break;
            }
            disconnectReason = DisconnectReason::NetworkError;
        break;
        case Transport::WSMethod::Error:
            errLog->critical("Controller :: WebSocket error (message: \"{0}\")", message);
            
            eventHandler(Event(Event::Type::NetworkError));

            std::this_thread::yield();
        break;
    }
}

void Controller::Logon()
{
    if (login.empty() || password.empty())
    {
        return Disconnect(DisconnectReason::AuthNeeded);
    }

    sysLog->info("Controller :: Logon");

    Proto::CONNECT_REQUEST::Command cmd;
    cmd.client_version = CLIENT_VERSION;
#ifdef WIN32
    cmd.system = "Windows";
#else
#ifdef __APPLE__
    cmd.system = "MacOS";
#else
    cmd.system = "Linux";
#endif
#endif
    cmd.login = login;
    cmd.password = password;

    SendCommand(cmd.Serialize());

    sysLog->info("Controller :: Logon sended");
}

void Controller::Logout()
{
    if (state == State::Ended)
    {
        return;
    }

    sysLog->info("Controller :: Logout");

    ChangeState(State::Ended);
    PingerStop();

    if (webSocket.IsConnected())
    {
        webSocket.Send(Proto::DISCONNECT::Command().Serialize());
    }

    webSocket.Disconnect();

    sysLog->info("Controller :: Logout finisned");
}

#ifndef WIN32
std::string get_path()
{
    char arg1[20];
    char exepath[PATH_MAX + 1] = { 0 };

    sprintf(arg1, "/proc/%d/exe", getpid());
    int ret = readlink(arg1, exepath, 1024);
    if (ret != 0) {}
    return std::string(exepath);
}

void Controller::Update(std::string_view fileName, std::string_view appFolder)
{
    std::thread thread([this, fileName]()
    {
        std::string updatedExe;
        while (updatedExe.empty())
        {
            Transport::HTTPClient httpClient;
            httpClient.Connect(baseURL);
            if (Common::Is64BitSystem())
            {
                updatedExe = httpClient.Request("/update/x64/" + std::string(fileName), "GET");
            }
            else
            {
                updatedExe = httpClient.Request("/update/" + std::string(fileName), "GET");
            }
            std::this_thread::yield;
        }

        const std::string path = get_path();

        const std::string tmpPath = "/tmp/";

        {
            std::ofstream exefile;
            exefile.open(tmpPath + std::string(fileName), std::ios::binary);
            exefile << updatedExe;
            exefile.close();
        }
        std::string cmd = "chmod 755 " + tmpPath + std::string(fileName);
        int ret = system(cmd.c_str());
        cmd = "mv " + tmpPath + std::string(fileName) + " " + path;
        ret = system(cmd.c_str());
        cmd = path + " /restart &";
        ret = system(cmd.c_str());
        if (ret != 0) {}

        eventHandler(Event(Event::Type::Updated));
    });

    thread.detach();
}
#else
void Controller::Update(std::string_view fileName, std::string_view appFolder)
{
    if (updater.joinable()) updater.join();
    updater = std::thread([this, fileName, appFolder]()
    {
        std::string updatedExe;
        while (updatedExe.empty())
        {
            Transport::HTTPClient httpClient;
            httpClient.Connect(baseURL);

            if (Common::Is64BitSystem())
            {
                updatedExe = httpClient.Request("/update/x64/" + std::string(fileName), "GET");
            }
            else
            {
                updatedExe = httpClient.Request("/update/" + std::string(fileName), "GET");
            }
            std::this_thread::yield;
        }

        wchar_t moduleFileName_[MAX_PATH] = { 0 };
        GetModuleFileName(NULL, moduleFileName_, MAX_PATH);
        std::string moduleFileName(boost::nowide::narrow(moduleFileName_));

        const std::wstring exeName = boost::nowide::widen(Common::FileNameOf(moduleFileName));
        const std::wstring path = boost::nowide::widen(Common::DirNameOf(moduleFileName)) + L"\\";

        wchar_t appDataPath_[MAX_PATH] = { 0 };
        SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath_);

        std::wstring tmpPath = std::wstring(appDataPath_) + boost::nowide::widen(appFolder);

        {
            std::ofstream exefile;
            exefile.open(tmpPath + boost::nowide::widen(fileName) + L"__", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
            exefile << updatedExe;
            exefile.close();
        }

        {
            std::wofstream cmdfile;
            cmdfile.open(tmpPath + L"update.cmd", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
            cmdfile.imbue(std::locale(cmdfile.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::little_endian>));
            cmdfile << L"@chcp 65001\r\n:loop\r\n" \
                L"if exist \"" << tmpPath << boost::nowide::widen(fileName) << L"__\" (\r\n"\
                L"move \"" << tmpPath << boost::nowide::widen(fileName) << L"__\" \"" << path << exeName << L"\" \r\n"\
                L"goto loop \r\n"\
                L")\r\n"\
                L"\"" << path << exeName << L"\"\r\n"\
                L"del \"" << tmpPath << L"update.cmd\"";
            cmdfile.close();
        }

        ShellExecute(NULL, !Common::CheckAllowFileWrite(boost::nowide::narrow(path)) ? L"runas" : L"open", std::wstring(tmpPath + L"update.cmd").c_str(), 0, 0, SW_HIDE);

        eventHandler(Event(Event::Type::Updated));
    });
}
#endif

void Controller::ChangeState(State newState)
{
    State prevState = state;
    state = newState;
    sysLog->info("Controller :: Changed state (from: {0}, to: {1})", toString(prevState), toString(newState));
}

void Controller::SendCommand(std::string_view command)
{
    if (webSocket.IsConnected())
    {
        webSocket.Send(command);
    }
}

}
