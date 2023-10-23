/**
 * Storage.h - Contains storage header
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <functional>

#include <Common/Common.h>

#include <Proto/Message.h>
#include <Proto/Member.h>
#include <Proto/Group.h>
#include <Proto/Conference.h>
#include <Proto/CmdContactList.h>

namespace DB { class Connection; }

namespace Storage
{

/// Messages
typedef std::vector<Proto::Message> Messages;

/// Contacts
typedef std::vector<Proto::Member> Contacts;

/// Groups
typedef std::vector<Proto::Group> Groups;

enum class MessageAction
{
    Added = 0,
    Updated
};

/// Conferences
typedef std::vector<Proto::Conference> Conferences;

class Storage
{
public:
	Storage();
	~Storage();

	void Connect(const std::string &dbPath);
	void SetMyClientId(int64_t id);
    
	/// Messages and calls API
	void AddMessage(const Proto::Message &message);
	void UpdateMessages(const Messages &messages);

    std::vector<int64_t> GetAbsentContacts(const Messages &messages);

	size_t LoadMessages(int64_t start, int64_t subscriber, const std::string &conference, uint32_t limit);
	size_t LoadNextMessages(int32_t count);

	Messages GetUndeliveredMessages();

	uint64_t GetLastMessageDT();

	std::recursive_mutex &MessagesMutex();
	const Messages &GetMessages() const;

    std::string SubscribeMessagesReceiver(std::function<void(MessageAction, const Messages &messages)> receiver);
    void UnsubscribeMessagesReceiver(const std::string &subscriberId);

	/// ContactList API
	void UpdateContacts(Proto::CONTACT_LIST::SortType sortType, bool showNumbers, const Contacts &contacts);
    void DeleteContact(int64_t clientId);
	void ClearContacts();

	void ChangeContactState(int64_t clientId, Proto::MemberState state);
	
	void LoadContacts();

    std::recursive_mutex &GetContactsMutex();
	const Contacts &GetContacts() const;

    bool GetShowNumbers();

    std::string SubscribeContactsReceiver(std::function<void(const Contacts &contacts)> receiver);
    void UnsubscribeContactsReceiver(const std::string &subscriberId);

	/// GroupList API
	void UpdateGroups(const Groups &groups);
	void ClearGroups();

	void LoadGroups();

	std::recursive_mutex &GetGroupsMutex();
	Groups &GetGroups();

	bool IsGroupRolled(int64_t groupId);
	void ChangeGroupRolled(int64_t groupId);

    std::string SubscribeGroupsReceiver(std::function<void(const Groups &groups)> receiver);
    void UnsubscribeGroupsReceiver(const std::string &subscriberId);

	/// Conferences API
	void UpdateConferences(const Conferences &conferences);

	void LoadConferences();

	std::recursive_mutex &GetConferencesMutex();
	const Conferences &GetConferences() const;

	bool IsConferenceRolled(int64_t conferenceId);
	void ChangeConferenceRolled(int64_t conferenceId);

    std::string SubscribeConferencesReceiver(std::function<void(const Conferences &conferences)> receiver);
    void UnsubscribeConferencesReceiver(const std::string &subscriberId);

private:
	std::string dbPath;

	int64_t myClientId;
	
	int64_t messagesStart, messagesEnd;
	int64_t messageSubscriber;
	int32_t messagesLimit;
	std::string messagesConference;

	std::recursive_mutex messagesMutex;
	Messages messages;
    std::map<std::string, std::function<void(MessageAction, const Messages &messages)>> messagesReceivers;

	std::recursive_mutex contactsMutex;
	Contacts contacts;
    std::map<std::string, std::function<void(const Contacts &contacts)>> contactsReceivers;

	std::recursive_mutex groupsMutex;
	Groups groups;
    std::map<std::string, std::function<void(const Groups &groups)>> groupsReceivers;

	std::recursive_mutex conferencesMutex;
	Conferences conferences;
    std::map<std::string, std::function<void(const Conferences &conferences)>> conferencesReceivers;

    Proto::CONTACT_LIST::SortType contactSortType;
    int32_t showNumbers;

	void UpdateDB();

	size_t LoadMessages(int64_t start);

    void UpdateUnreadedContacts();
	void UpdateUnreadedConferences();

    int32_t CalcUnreadedContact(DB::Connection &conn, int64_t clientId);
    int32_t CalcUnreadedConference(DB::Connection &conn, const std::string &tag);

    Proto::CONTACT_LIST::SortType GetContactSortType();

    int64_t GetMyClientId();

    void SortContacts();
    void SortConferences();
};

std::string GUID();

}
