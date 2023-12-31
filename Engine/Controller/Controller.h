/**
 * Controller.h - Contains controller's header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014 - 2020
 */

#pragma once

#include <atomic>
#include <thread>

#include "IController.h"
#include "IMemberList.h"

#include <Transport/WS/WebSocket.h>

#include <Common/TimeMeter.h>

#include <spdlog/spdlog.h>

namespace Controller
{

class Controller : public IController
{
public:
    Controller(Storage::Storage &storage, IMemberList &memberList);
    ~Controller();

    virtual void SetEventHandler(std::function<void(const Event&)> handler);
    virtual void SetConferenceUpdateHandler(std::function<void(const Proto::CONFERENCE_UPDATE_RESPONSE::Command&)> handler);
    virtual void SetContactListHandler(std::function<void(const Storage::Contacts&)> handler);
    
    virtual void Connect(std::string_view serverAddress, bool secureConnection);
    virtual void Disconnect();
    virtual bool Connected();

    virtual void UserUpdate(int64_t userId, std::string_view name = "", std::string_view avatar = "", std::string_view login = "", std::string_view password = "");

    virtual void SetCredentials(std::string_view login, std::string_view password);
    virtual void Update(std::string_view fileName, std::string_view appFolder = "");
    virtual void SetMaxBitrate(uint32_t value);

    virtual void CreateCapturer(const DeviceValues &device);
    virtual void ConnectCapturer(const DeviceValues &device);
    virtual void DeleteCapturer(uint32_t deviceId);

    virtual void ConnectToConference(const Proto::Conference &conference, bool hasCamera, bool hasMicrophone, bool hasDemonstration);
    virtual void DisconnectFromConference(std::vector<DeviceValues> capturers, std::vector<DeviceValues> renderers);

    virtual void ConnectRenderer(uint32_t deviceId, uint32_t ssrc);
    virtual void DisconnectRenderer(uint32_t deviceId, uint32_t ssrc);

    virtual void ChangeResolution(uint32_t deviceId, Video::Resolution resolution);
    virtual void MicrophoneActive(uint32_t deviceId, Proto::MICROPHONE_ACTIVE::ActiveType activeType);

    virtual void CallRequest(CallValues values);
    virtual void CallResponse(CallValues values);

    virtual void AddContact(int64_t clientId);
    virtual void DeleteContact(int64_t clientId);

    virtual void CreateConference(const Proto::Conference &conference);
    virtual void EditConference(const Proto::Conference &conference);
    virtual void DeleteConference(int64_t conferenceId);
    virtual void AddMeToConference(std::string_view tag);
    virtual void DeleteMeFromConference(int64_t conferenceId);
    virtual void CreateTempConference();
    virtual void SendConnectToConference(std::string_view tag, int64_t connecter_id, uint32_t connecter_connection_id, uint32_t flags);

    virtual void UpdateGroupList();

    virtual void UpdateContactList();

    virtual void SearchContact(std::string_view name);

    virtual void UpdateConferencesList();

    virtual void SendMemberAction(const std::vector<int64_t> &members, Proto::MEMBER_ACTION::Action action, uint32_t grants = 0);
    virtual void SendMemberActionResult(int64_t member, Proto::MEMBER_ACTION::Result result);

    virtual void SendTurnSpeaker();
    virtual void SendWantSpeak();

    virtual void LoadMessages(uint64_t fromDT);
    virtual void DeliveryMessages(const Storage::Messages &messages);

    virtual void RequestMediaAddresses();

    virtual State GetState() const;

    virtual bool IsSecureConnection() const;
    virtual std::string GetServerAddress() const;
    virtual std::string GetLogin() const;
    virtual std::string GetPassword() const;

    virtual std::string GetAccessToken() const;

    virtual uint32_t GetGrants() const;
    virtual uint32_t GetMaxOutputBitrate() const;

    virtual int64_t GetMyClientId() const;
    virtual std::string GetMyClientName() const;
    virtual std::string GetServerName() const;

    virtual Proto::Conference &GetCurrentConference();

private:
    std::function<void(const Event&)> eventHandler;
    std::function<void(const Proto::CONFERENCE_UPDATE_RESPONSE::Command&)> conferenceUpdateHandler;
    std::function<void(const Storage::Contacts&)> contactListHandler;

    Storage::Storage &storage;
    IMemberList &memberList;

    std::atomic<State> state;

    uint32_t grants;
    uint32_t maxOutputBitrate;

    time_t callStart;

    Proto::Conference currentConference;
    int64_t subscriberId;

    std::string baseURL;
    Transport::WebSocket webSocket;

    std::string serverAddress;
    bool secureConnection;
    std::string login, password;

    int64_t clientId, connectionId;
    std::string clientName;
    std::string secureKey;
    std::string accessToken;

    std::string serverName;

    Storage::Contacts findedContacts;

    enum class DisconnectReason
    {
        NetworkError = 0,

        Redirected,

        AuthNeeded,
        InvalidCredentionals,
        
        UpdateRequired,
        ServerOutdated,
        
        ServerFull,
        InternalServerError
    } disconnectReason;

    const char* toString(DisconnectReason reason);
    const char* toString(State state);

    std::thread updater;

    enum class ContactListReceiving
    {
        Undefined = 0,

        Update,
        Search
    };
    ContactListReceiving contactListReceving;

    std::shared_ptr<spdlog::logger> sysLog, errLog;

    void OnWebSocket(Transport::WSMethod method, std::string_view message);

    void Logon();
    void Logout();

    void Disconnect(DisconnectReason disconnectReason);
    
    void ChangeState(State newState);

    void SendCommand(std::string_view command);
};

}
