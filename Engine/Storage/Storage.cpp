/**
 * Storage.cpp - Contains storage impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <ctime>
#include <algorithm>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <Storage/Storage.h>

#include <db/connection.h>
#include <db/transaction.h>
#include <db/query.h>

namespace Storage
{

std::string GUID()
{
	return boost::uuids::to_string(boost::uuids::random_generator()());
}

Storage::Storage()
	: dbPath(),
	myClientId(0),
	messagesStart(0), messagesEnd(0), messageSubscriber(0), messagesLimit(0),
	messagesConference(),
	messagesMutex(), messages(), messagesReceivers(),
	contactsMutex(), contacts(), contactsReceivers(),
	groupsMutex(), groups(), groupsReceivers(),
	conferencesMutex(), conferences(), conferencesReceivers(),
    contactSortType(Proto::CONTACT_LIST::SortType::Undefined),
    showNumbers(-1)
{
}

Storage::~Storage()
{
}

void Storage::Connect(std::string_view dbPath_)
{
	dbPath = dbPath_;
	UpdateDB();
	LoadContacts();
	LoadGroups();
	LoadConferences();
}

void Storage::SetMyClientId(int64_t id)
{
	myClientId = id;

    db::connection conn(db::dbms::SQLite, dbPath);

    db::transaction writeTr(conn);
    writeTr.start();

    db::query upd_sort_query(conn);
    upd_sort_query.prepare("update settings set value = ? where key='my_client_id'");
    upd_sort_query.set(0, id);
    upd_sort_query.step();

    writeTr.commit();
}

/// Messages

void Storage::AddMessage(const Proto::Message &message)
{
	db::connection conn(db::dbms::SQLite, dbPath);

	db::transaction writeTr(conn);
	writeTr.start();

	db::query message_query(conn);
	message_query.prepare("insert into messages (guid, dt, type, author_id, sender_id, subscriber_id, conference_tag, text_value, call_duration, call_result, data_preview, data_url, status) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
	message_query.set(0, message.guid);
	message_query.set(1, static_cast<int64_t>(message.dt));
	message_query.set(2, static_cast<int32_t>(message.type));
	message_query.set(3, message.author_id);
	message_query.set(4, message.sender_id);
	message_query.set(5, message.subscriber_id);
	message_query.set(6, message.conference_tag);
	message_query.set(7, message.text);
	message_query.set(8, message.call_duration);
	message_query.set(9, static_cast<int32_t>(message.call_result));
	message_query.set(10, message.preview);
	message_query.set(11, message.url);
	message_query.set(12, static_cast<int32_t>(message.status));
	message_query.step();
	message_query.reset();
	
	if (messageSubscriber == message.subscriber_id || (!messagesConference.empty() && messagesConference == message.conference_tag))
	{
		std::lock_guard<std::recursive_mutex> lock(messagesMutex);
		messages.insert(messages.begin(), message);
	}
	
	writeTr.commit();

    for (auto receiver : messagesReceivers)
    {
        receiver.second(MessageAction::Added, { message });
    }
}

template <typename T>
inline void AddSQLValue(std::string_view fieldName, T value, std::string &fields, std::string &values, bool exists, bool allowNull = true)
{
	std::string value_ = std::to_string(value);
	if (value == 0 && allowNull)
	{
		value_ = "null";
	}

	if (!exists)
	{
		fields += std::string(fieldName) + ",";
		values += value_ + ",";
	}
	else
	{
		values += std::string(fieldName) + "=" + value_ + ",";
	}
}

inline void AddSQLValue(std::string_view fieldName, const std::string& value, std::string& fields, std::string& values, bool exists)
{
	std::string value_ = "'" + value + "'";
	if (value.empty())
	{
		value_ = "null";
	}

	if (!exists)
	{
		fields += std::string(fieldName) + ",";
		values += value_ + ",";
	}
	else
	{
		values += std::string(fieldName) + "=" + value_ + ",";
	}
}

template <typename T>
inline void AddOnlyDataSQLValue(std::string_view fieldName, T value, std::string &fields, std::string &values, bool exists)
{
	if (value != 0)
	{
		if (!exists)
		{
			fields += std::string(fieldName) + ",";
			values += std::to_string(value) + ",";
		}
		else
		{
			values += std::string(fieldName) + "=" + std::to_string(value) + ",";
		}
	}
}

inline void AddOnlyDataSQLValue(std::string_view fieldName, const std::string &value, std::string &fields, std::string &values, bool exists)
{
	if (!value.empty())
	{
		if (!exists)
		{
			fields += std::string(fieldName) + ",";
			values += "'" + value + "',";
		}
		else
		{
			values += std::string(fieldName) + "='" + value + "',";
		}
	}
}

void Storage::UpdateMessages(const Messages &inputMessages)
{
	db::connection conn(db::dbms::SQLite, dbPath);

	db::transaction writeTr(conn);
	writeTr.start();
	
	db::query exists_query(conn);
	exists_query.prepare("select id from messages where guid = ?");

	db::query message_query(conn);
	
    bool updatedMessages = false;

	std::lock_guard<std::recursive_mutex> lock(messagesMutex);
	for (auto &message : inputMessages)
	{
		exists_query.set(0, message.guid);
		bool exists = exists_query.step();
		exists_query.reset();

		std::string sqlFields, sqlValues;

		AddOnlyDataSQLValue("guid", message.guid, sqlFields, sqlValues, exists);
		AddOnlyDataSQLValue("dt", message.dt, sqlFields, sqlValues, exists);
		AddOnlyDataSQLValue("type", static_cast<int32_t>(message.type), sqlFields, sqlValues, exists);
		AddOnlyDataSQLValue("author_id", message.author_id, sqlFields, sqlValues, exists);
		AddOnlyDataSQLValue("sender_id", message.sender_id, sqlFields, sqlValues, exists);
		AddOnlyDataSQLValue("subscriber_id", message.subscriber_id, sqlFields, sqlValues, exists);
		AddOnlyDataSQLValue("conference_tag", message.conference_tag, sqlFields, sqlValues, exists);
		AddOnlyDataSQLValue("text_value", message.text, sqlFields, sqlValues, exists);
		AddOnlyDataSQLValue("call_duration", message.call_duration, sqlFields, sqlValues, exists);
		AddOnlyDataSQLValue("call_result", static_cast<int32_t>(message.call_result), sqlFields, sqlValues, exists);
		AddOnlyDataSQLValue("data_preview", message.preview, sqlFields, sqlValues, exists);
		AddOnlyDataSQLValue("data_url", message.url, sqlFields, sqlValues, exists);
		AddOnlyDataSQLValue("status", static_cast<int32_t>(message.status), sqlFields, sqlValues, exists);

		if (!sqlFields.empty()) sqlFields.pop_back(); // drop last ","
		if (!sqlValues.empty()) sqlValues.pop_back(); // drop last ","

		message_query.prepare(!exists ? "insert into messages (" + sqlFields + ") values (" + sqlValues + ")" : "update messages set " + sqlValues + " where guid='" + message.guid + "'");
		message_query.step();
		message_query.reset();

		if (messageSubscriber == message.subscriber_id ||
			(!messagesConference.empty() && messagesConference == message.conference_tag) ||
			message.status == Proto::MessageStatus::Readed)
		{
			auto storedMessage = std::find(messages.begin(), messages.end(), message.guid);
			if (storedMessage != messages.end())
			{
				if (message.dt != 0)                                     storedMessage->dt = message.dt;
				if (message.type != Proto::MessageType::Undefined)       storedMessage->type = message.type;
				if (message.author_id != 0)                              storedMessage->author_id = message.author_id;
				if (message.sender_id != 0)                              storedMessage->sender_id = message.sender_id;
				if (message.subscriber_id != 0)                          storedMessage->subscriber_id = message.subscriber_id;
				if (!message.conference_tag.empty())                     storedMessage->conference_tag = message.conference_tag;
				if (!message.text.empty())                               storedMessage->text = message.text;
				if (message.call_duration != 0)                          storedMessage->call_duration = message.call_duration;
				if (message.call_result != Proto::CallResult::Undefined) storedMessage->call_result = message.call_result;
				if (!message.preview.empty())                            storedMessage->preview = message.preview;
				if (!message.url.empty())                                storedMessage->url = message.url;
				if (message.status != Proto::MessageStatus::Undefined)   storedMessage->status = message.status;
			}
			else
			{
				messages.emplace_back(message);
			}

			updatedMessages = true;
		}
	}

	if (updatedMessages && !messages.empty())
	{
		std::sort(messages.begin(), messages.end(),
			[](const Proto::Message &a, const Proto::Message &b) -> bool
		{
			return a.dt > b.dt;
		});

		messagesEnd = (messages.end() - 1)->dt;
	}
	
	writeTr.commit();

	UpdateUnreadedContacts();
	UpdateUnreadedConferences();

    for (auto receiver : messagesReceivers)
    {
        receiver.second(MessageAction::Updated, inputMessages);
    }
}

std::vector<int64_t> Storage::GetAbsentContacts(const Messages &messages_)
{
    std::vector<int64_t> out;

    std::lock_guard<std::recursive_mutex> lock(contactsMutex);

    for (auto &msg : messages_)
    {
		if (msg.author_id == 0)
		{
			continue;
		}

        auto contact = std::find(contacts.begin(), contacts.end(), msg.author_id);
        if (contact == contacts.end())
        {
            out.emplace_back(msg.author_id);
        }
        else
        {
            std::lock_guard<std::recursive_mutex> lock(groupsMutex);
            for (const auto &g : contact->groups)
            {
                auto group = std::find(groups.begin(), groups.end(), g.id);
                if (group != groups.end())
                {
                    group->rolled = false;

                    auto parentId = group->parent_id;
                    while (parentId != 0)
                    {
                        auto parent = std::find(groups.begin(), groups.end(), parentId);
                        if (parent != groups.end())
                        {
                            parent->rolled = false;
                            parentId = parent->parent_id;
                        }
                    }
                }
            }
        }
    }

    return out;
}

void Storage::UpdateUnreadedContacts()
{
	if (GetMyClientId() == 0)
	{
		return;
	}

	std::lock_guard<std::recursive_mutex> lock(contactsMutex);
	for (auto &c : contacts)
	{
		c.unreaded_count = 0;
	}

	db::connection conn(db::dbms::SQLite, dbPath);
	db::query count_query(conn);
	count_query.prepare("select subscriber_id, count(id) from messages where status < " + std::to_string(static_cast<int32_t>(Proto::MessageStatus::Readed)) + " and ((type = " + std::to_string(static_cast<int32_t>(Proto::MessageType::TextMessage)) + " and author_id <> ?) or type = " + std::to_string(static_cast<int32_t>(Proto::MessageType::ServiceMessage)) + ") and conference_tag is null group by subscriber_id");
	count_query.set(0, GetMyClientId());
	while (count_query.step())
	{
		auto it = std::find(contacts.begin(), contacts.end(), count_query.get_int64(0));
		if (it != contacts.end())
		{
			it->unreaded_count = count_query.get_int32(1);
		}
	}

    SortContacts();
}

void Storage::UpdateUnreadedConferences()
{
	if (GetMyClientId() == 0)
	{
		return;
	}

	std::lock_guard<std::recursive_mutex> lock_(conferencesMutex);
	for (auto &c : conferences)
	{
		c.unreaded_count = 0;
	}

	db::connection conn(db::dbms::SQLite, dbPath);
	db::query count_query(conn);
	count_query.prepare("select conference_tag, count(id) from messages where status < " + std::to_string(static_cast<int32_t>(Proto::MessageStatus::Readed)) + " and subscriber_id is null and (author_id is null or author_id <> ?) group by conference_tag");
	count_query.set(0, GetMyClientId());
	while (count_query.step())
	{
		auto it = std::find(conferences.begin(), conferences.end(), count_query.get_string(0));
		if (it != conferences.end())
		{
			it->unreaded_count = count_query.get_int32(1);
		}
	}

    SortConferences();
}

int32_t Storage::CalcUnreadedContact(db::connection &conn, int64_t clientId)
{
    db::query count_query(conn);
    count_query.prepare("select subscriber_id, count(id) from messages where status < " + std::to_string(static_cast<int32_t>(Proto::MessageStatus::Readed)) + " and author_id <> ? and conference_tag is null group by subscriber_id");
    count_query.set(0, clientId);
    while (count_query.step())
    {
        if (count_query.get_int64(0) == clientId)
        {
            return count_query.get_int32(1);
        }
    }

    return 0;
}

int32_t Storage::CalcUnreadedConference(db::connection &conn, std::string_view tag)
{
    if (GetMyClientId() == 0)
    {
        return 0;
    }

    db::query count_query(conn);
    count_query.prepare("select conference_tag, count(id) from messages where status < " + std::to_string(static_cast<int32_t>(Proto::MessageStatus::Readed)) + " and subscriber_id is null and (author_id is null or author_id <> ?) group by conference_tag");
    count_query.set(0, GetMyClientId());
    while (count_query.step())
    {
        if (count_query.get_string(0) == tag)
        {
            return count_query.get_int32(1);
        }
    }

    return 0;
}

size_t Storage::LoadMessages(int64_t start, int64_t subscriber, std::string_view conference, uint32_t limit)
{
	messages.clear();

	messagesStart = start;
	messagesEnd = 0;
	messageSubscriber = subscriber;
	messagesConference = conference;
	messagesLimit = limit;

	return LoadMessages(start);
}

size_t Storage::LoadNextMessages(int32_t count)
{
	messagesLimit = count;

	return LoadMessages(messagesEnd);
}

size_t Storage::LoadMessages(int64_t start)
{
    std::lock_guard<std::recursive_mutex> lock(messagesMutex);

	db::connection conn(db::dbms::SQLite, dbPath);

	std::string predicat;

	if (start != 0)
	{
		predicat = " where dt < " + std::to_string(start);
	}

	if (messageSubscriber != 0)
	{
		if (predicat.empty()) predicat += " where"; else predicat += " and";

		predicat += " subscriber_id = " + std::to_string(messageSubscriber);

	}
	else if (!messagesConference.empty())
	{
		if (predicat.empty()) predicat += " where"; else predicat += " and";

		predicat += " conference_tag = '" + messagesConference + "'";
	}
	
	predicat += " order by dt desc";

	if (messagesLimit != 0)
	{
		predicat += " limit 0," + std::to_string(messagesLimit);
	}

	db::query user_name_query(conn);
	user_name_query.prepare("select name from users where id = ?");

	db::query conferences_query(conn);
	conferences_query.prepare("select name from conferences where tag = ?");

	size_t count = 0;

	db::query messages_query(conn);
	messages_query.prepare("select guid, dt, type, author_id, sender_id, subscriber_id, conference_tag, text_value, call_duration, call_result, data_preview, data_url, status from messages" + predicat);
	while (messages_query.step())
	{
		std::string authorName, senderName, subscriberName;
		user_name_query.set(0, messages_query.get_int64(3));
		if (user_name_query.step())
		{
			authorName = user_name_query.get_string(0);
		}
		user_name_query.reset();
		user_name_query.set(0, messages_query.get_int64(4));
		if (user_name_query.step())
		{
			senderName = user_name_query.get_string(0);
		}
		user_name_query.reset();
		user_name_query.set(0, messages_query.get_int64(5));
		if (user_name_query.step())
		{
			subscriberName = user_name_query.get_string(0);
		}
		user_name_query.reset();

		std::string conferenceName;
		conferences_query.set(0, messages_query.get_string(6));
		if (conferences_query.step())
		{
			conferenceName = conferences_query.get_string(0);
		}
		conferences_query.reset();

		messages.emplace_back(Proto::Message(messages_query.get_string(0),
			messages_query.get_int32(1),
			static_cast<Proto::MessageType>(messages_query.get_int32(2)),
			messages_query.get_int64(3),	authorName,
			messages_query.get_int64(4),	senderName,
			messages_query.get_int64(5),	subscriberName,
			messages_query.get_string(6), conferenceName,
			static_cast<Proto::MessageStatus>(messages_query.get_int32(12)),
			messages_query.get_string(7),
			messages_query.get_int32(8), static_cast<Proto::CallResult>(messages_query.get_int32(9)),
			messages_query.get_string(10), messages_query.get_string(11),
			"" ));

		messagesEnd = messages_query.get_int32(1);

		++count;
	}

	return count;
}

Messages Storage::GetUndeliveredMessages()
{
	Messages outMessages;

	db::connection conn(db::dbms::SQLite, dbPath);

	db::query user_name_query(conn);
	user_name_query.prepare("select name from users where id = ?");

	db::query conferences_query(conn);
	conferences_query.prepare("select name from conferences where tag = ?");

	db::query messages_query(conn);
	messages_query.prepare("select guid, dt, type, author_id, sender_id, subscriber_id, conference_tag, text_value, call_duration, call_result, data_preview, data_url, status from messages where sender_id = ? and type = 1 and status < 2");
	messages_query.set(0, GetMyClientId());
	while (messages_query.step())
	{
		std::string authorName, senderName, subscriberName;
		user_name_query.set(0, messages_query.get_int32(3));
		if (user_name_query.step())
		{
			authorName = user_name_query.get_string(0);
		}
		user_name_query.reset();
		user_name_query.set(0, messages_query.get_int32(4));
		if (user_name_query.step())
		{
			senderName = user_name_query.get_string(0);
		}
		user_name_query.reset();
		user_name_query.set(0, messages_query.get_int32(5));
		if (user_name_query.step())
		{
			subscriberName = user_name_query.get_string(0);
		}
		user_name_query.reset();

		std::string conferenceName;
		conferences_query.set(0, messages_query.get_string(6));
		if (conferences_query.step())
		{
			conferenceName = conferences_query.get_string(0);
		}
		conferences_query.reset();

		outMessages.emplace_back(Proto::Message(messages_query.get_string(0),
			messages_query.get_int32(1),
			static_cast<Proto::MessageType>(messages_query.get_int32(2)),
			messages_query.get_int32(3), authorName,
			messages_query.get_int32(4), senderName,
			messages_query.get_int32(5), subscriberName,
			messages_query.get_string(6), conferenceName,
			static_cast<Proto::MessageStatus>(messages_query.get_int32(12)),
			messages_query.get_string(7),
			messages_query.get_int32(8), static_cast<Proto::CallResult>(messages_query.get_int32(9)),
			messages_query.get_string(10), messages_query.get_string(11),
			""));
	}

	return outMessages;
}

uint64_t Storage::GetLastMessageDT()
{
	uint64_t out = 0;

	db::connection conn(db::dbms::SQLite, dbPath);

	db::query query(conn);
	query.prepare("select max(dt) from messages where type = 1 and status > 1");
	if (query.step())
	{
		out = query.get_int64(0);
	}
    return out;
}

std::recursive_mutex &Storage::MessagesMutex()
{
	return messagesMutex;
}

const Messages &Storage::GetMessages() const
{
	return messages;
}

std::string Storage::SubscribeMessagesReceiver(std::function<void(MessageAction, const Messages &messages)> receiver)
{
    std::lock_guard<std::recursive_mutex> lock(messagesMutex);

    auto id = GUID();

    messagesReceivers[id] = receiver;

    return id;
}

void Storage::UnsubscribeMessagesReceiver(std::string_view subscriberId)
{
    std::lock_guard<std::recursive_mutex> lock(messagesMutex);

    auto it = messagesReceivers.find(subscriberId.data());
    if (it != messagesReceivers.end())
    {
        messagesReceivers.erase(it);
    }
}

/// Contacts

void Storage::UpdateContacts(Proto::CONTACT_LIST::SortType sortType, bool showNumbers_, const Contacts &contacts_)
{
    contactSortType = sortType;
    showNumbers = showNumbers_ ? 1 : 0;

	db::connection conn(db::dbms::SQLite, dbPath);

	db::query exists_query(conn);
	exists_query.prepare("select id from users where id = ?");

	db::transaction writeTr(conn);
	writeTr.start();
	
    db::query upd_sort_query(conn);
    upd_sort_query.prepare("update settings set value = ? where key = ?");
    
    upd_sort_query.set(0, sortType == Proto::CONTACT_LIST::SortType::Name ? "0" : "1");
    upd_sort_query.set(1, "users_sort_type");
    upd_sort_query.step();
    upd_sort_query.reset();

    upd_sort_query.set(0, showNumbers_ ? "1" : "0");
    upd_sort_query.set(1, "show_number_on_contact_list");
    upd_sort_query.step();
    upd_sort_query.reset();

	std::lock_guard<std::recursive_mutex> lock(contactsMutex);
	for (auto &contact : contacts_)
	{
		bool exists = false;
		exists_query.set(0, contact.id);
		if (exists_query.step())
		{
			exists = true;
		}
		exists_query.reset();

		std::string sqlFields, sqlValues;

		if (!contact.deleted)
		{
			AddSQLValue("id", contact.id, sqlFields, sqlValues, exists);
			AddSQLValue("login", contact.login, sqlFields, sqlValues, exists);
			AddSQLValue("name", contact.name, sqlFields, sqlValues, exists);
			AddSQLValue("number", contact.number, sqlFields, sqlValues, exists);
			AddSQLValue("state", static_cast<int32_t>(contact.state), sqlFields, sqlValues, exists);
		}
		AddSQLValue("deleted", contact.deleted, sqlFields, sqlValues, exists);

		if (!sqlFields.empty()) sqlFields.pop_back(); // drop last ","
		if (!sqlValues.empty()) sqlValues.pop_back(); // drop last ","

		db::query contact_query(conn);
		contact_query.prepare(!exists ? "insert into users (" + sqlFields + ") values (" + sqlValues + ")" : "update users set " + sqlValues + " where id='" + std::to_string(contact.id) + "'");
		contact_query.step();
		contact_query.reset();

		std::string groupIds = "in ( ";
		for (auto &g : contact.groups)
		{
			groupIds += std::to_string(g.id) + ",";
		}
		groupIds.pop_back();
		groupIds += ")";

		db::query contact_groups_clear_query(conn);
		contact_groups_clear_query.prepare("delete from client_groups where client_id = ? and group_id not " + groupIds);
		contact_groups_clear_query.set(0, contact.id);
		contact_groups_clear_query.step();

		db::query exists_group_query(conn);
		exists_group_query.prepare("select id from client_groups where client_id = ? and group_id = ?");

		db::query contact_groups_query(conn);
		contact_groups_query.prepare("insert into client_groups (client_id, group_id) values (?, ?)");
		for (auto &g : contact.groups)
		{
			exists_group_query.set(0, contact.id);
			exists_group_query.set(1, g.id);
			if (!exists_group_query.step())
			{
				contact_groups_query.set(0, contact.id);
				contact_groups_query.set(1, g.id);
				contact_groups_query.step();
				contact_groups_query.reset();
			}
			exists_group_query.reset();				
		}

        auto it = std::find(contacts.begin(), contacts.end(), contact.id);
		if (it != contacts.end())
		{
			if (contact.deleted)
			{
				contacts.erase(it);
			}
			else
			{
                auto unreaded = it->unreaded_count;
               
                *it = contact;

                it->unreaded_count = unreaded;
			}
		}
		else
		{
			if (!contact.deleted)
			{
                contacts.emplace_back(contact);
			}
		}
	}

	writeTr.commit();

    UpdateUnreadedContacts();
    SortContacts();

    for (auto receiver : contactsReceivers)
    {
        receiver.second(contacts);
    }
}

void Storage::DeleteContact(int64_t clientId)
{
    db::connection conn(db::dbms::SQLite, dbPath);

    db::transaction writeTr(conn);
    writeTr.start();

    db::query contact_query(conn);
    contact_query.prepare("update users set deleted = 1 where id = ?");
    contact_query.set(0, clientId);
    contact_query.step();

    db::query groups_query(conn);
    groups_query.prepare("delete from client_groups where client_id = ?");
    groups_query.set(0, clientId);
    groups_query.step();

    writeTr.commit();

    {
        std::lock_guard<std::recursive_mutex> lock(contactsMutex);
        auto it = std::find(contacts.begin(), contacts.end(), clientId);
        if (it != contacts.end())
        {
            contacts.erase(it);
        }
    }

    for (auto receiver : contactsReceivers)
    {
        receiver.second({ Proto::Member(clientId) });
    }
}

void Storage::ClearContacts()
{
	db::connection conn(db::dbms::SQLite, dbPath);

	db::transaction writeTr(conn);
	writeTr.start();

	db::query del_query(conn);
	del_query.prepare("delete from users");
	del_query.step();

	writeTr.commit();

	contacts.clear();
}

void Storage::ChangeContactState(int64_t clientId, Proto::MemberState state)
{
	db::connection conn(db::dbms::SQLite, dbPath);

	db::transaction writeTr(conn);
	writeTr.start();

	db::query contact_query(conn);
	contact_query.prepare("update users set state = ? where id = ?");
	contact_query.set(0, static_cast<int32_t>(state));
	contact_query.set(1, static_cast<int32_t>(clientId));
	contact_query.step();

	writeTr.commit();

	Proto::Member changedContact;
	std::lock_guard<std::recursive_mutex> lock(contactsMutex);
	auto it = std::find(contacts.begin(), contacts.end(), clientId);
	if (it != contacts.end())
	{
		it->state = state;
		changedContact = *it;
	}

	if (changedContact.id != 0)
	{
        for (auto receiver : contactsReceivers)
        {
            receiver.second({ changedContact });
        }
	}
}

Proto::CONTACT_LIST::SortType Storage::GetContactSortType()
{
    if (contactSortType != Proto::CONTACT_LIST::SortType::Undefined)
    {
        return contactSortType;
    }

	db::connection conn(db::dbms::SQLite, dbPath);

	db::query get_sort_query(conn);
	get_sort_query.prepare("select value from settings where key='users_sort_type'");
	if (get_sort_query.step())
	{
        contactSortType = get_sort_query.get_string(0) != "0" ? Proto::CONTACT_LIST::SortType::Number : Proto::CONTACT_LIST::SortType::Name;
	}

	return contactSortType;
}

int64_t Storage::GetMyClientId()
{
    if (myClientId != 0)
    {
        return myClientId;
    }
    
    db::connection conn(db::dbms::SQLite, dbPath);

    db::query get_my_client_id_query(conn);
    get_my_client_id_query.prepare("select value from settings where key='my_client_id'");
    if (get_my_client_id_query.step())
    {
        return get_my_client_id_query.get_int64(0);
    }
    return 0;
}

void Storage::SortContacts()
{
    std::sort(contacts.begin(), contacts.end(),
        [this](const Proto::Member &a, const Proto::Member &b)
    {
        return GetContactSortType() == Proto::CONTACT_LIST::SortType::Name ? a.name < b.name : atoi(a.number.c_str()) < atoi(b.number.c_str());
    });

    std::sort(contacts.begin(), contacts.end(),
        [](const Proto::Member &a, const Proto::Member &b)
    {
        return a.unreaded_count > b.unreaded_count;
    });
}

void Storage::SortConferences()
{
    std::sort(conferences.begin(), conferences.end(),
        [](const Proto::Conference &a, const Proto::Conference &b)
    {
        return a.name < b.name;
    });

    std::sort(conferences.begin(), conferences.end(),
        [](const Proto::Conference &a, const Proto::Conference &b)
    {
        return a.unreaded_count > b.unreaded_count;
    });
}

void Storage::LoadContacts()
{
    std::lock_guard<std::recursive_mutex> lock(contactsMutex);

	contacts.clear();

	db::connection conn(db::dbms::SQLite, dbPath);

    std::vector<int64_t> availGroups;
    db::query avail_groups_query(conn);
    avail_groups_query.prepare("select id from groups");
    while (avail_groups_query.step())
    {
        availGroups.emplace_back(avail_groups_query.get_int64(0));
    }
    avail_groups_query.close();

	db::query contact_groups_query(conn);
	contact_groups_query.prepare("select group_id from client_groups where client_id = ?");

	db::query contacts_query(conn);
	contacts_query.prepare("select id, state, login, name, number from users where deleted is null order by " + std::string(GetContactSortType() == Proto::CONTACT_LIST::SortType::Name ? "name" : "cast(number as integer)"));
	while (contacts_query.step())
	{
        bool contactAvail = false;

        auto contactId = contacts_query.get_int64(0);

		std::vector<Proto::Group> groups_;

		contact_groups_query.set(0, contactId);
		while (contact_groups_query.step())
		{
            auto groupId = contact_groups_query.get_int64(0);
			groups_.emplace_back(Proto::Group(groupId));

            if (!contactAvail)
            {
                contactAvail = std::find(availGroups.begin(), availGroups.end(), groupId) != availGroups.end();
            }
		}
		contact_groups_query.reset();

        if (contactAvail)
        {
            auto member = Proto::Member(
                contactId,
                static_cast<Proto::MemberState>(contacts_query.get_int32(1)),
                contacts_query.get_string(2),
                contacts_query.get_string(3),
                contacts_query.get_string(4),
                groups_);

            member.unreaded_count = CalcUnreadedContact(conn, contacts_query.get_int64(0));

            contacts.emplace_back(member);
        }
	}

    SortContacts();
}

std::recursive_mutex &Storage::GetContactsMutex()
{
	return contactsMutex;
}

const Contacts &Storage::GetContacts() const
{
	return contacts;
}

bool Storage::GetShowNumbers()
{
    if (showNumbers != -1)
    {
        return showNumbers != 0;
    }

    db::connection conn(db::dbms::SQLite, dbPath);

    db::query get_query(conn);
    get_query.prepare("select value from settings where key='show_number_on_contact_list'");
    if (get_query.step())
    {
        showNumbers = get_query.get_string(0) != "0" ? 0 : 1;
    }

    return showNumbers != 0;
}

std::string Storage::SubscribeContactsReceiver(std::function<void(const Contacts &contacts)> receiver)
{
    std::lock_guard<std::recursive_mutex> lock(contactsMutex);

    auto id = GUID();

    contactsReceivers[id] = receiver;

    return id;
}

void Storage::UnsubscribeContactsReceiver(std::string_view subscriberId)
{
    std::lock_guard<std::recursive_mutex> lock(contactsMutex);

    auto it = contactsReceivers.find(subscriberId.data());
    if (it != contactsReceivers.end())
    {
        contactsReceivers.erase(it);
    }
}

/// Groups

void GetChildGroups(db::connection &conn, int64_t parentID, int32_t level, std::vector<Proto::Group> &out)
{
	db::query rolled_query(conn);
	rolled_query.prepare("select id from group_rolled where id = ?");

	db::query groups_query(conn);
	groups_query.prepare("select id, parent_id, tag, name, owner_id, password, grants from groups where deleted is null and parent_id = ? order by name");
	groups_query.set(0, parentID);

	++level;

	while (groups_query.step())
	{
		Proto::Group group(
			groups_query.get_int64(0),
			groups_query.get_int64(1),
			groups_query.get_string(2),
			groups_query.get_string(3),
			groups_query.get_int64(4),
			groups_query.get_string(5),
			groups_query.get_int32(6),
			level
		);

		rolled_query.set(0, group.id);
		if (rolled_query.step())
		{
			group.rolled = true;
		}
		rolled_query.reset();

		out.emplace_back(group);

		GetChildGroups(conn, group.id, level, out);
	}
}

std::string GetChildGroupsSQL(db::connection &conn, int64_t group_id)
{
	std::string groupsSQL = " in(" + std::to_string(group_id) + ",";

	std::vector<Proto::Group> groups;
	GetChildGroups(conn, group_id, -1, groups);
	for (auto &g : groups)
	{
		groupsSQL += std::to_string(g.id) + ",";
	}

	groupsSQL.pop_back(); // drop last ","
	groupsSQL += ") ";

	return groupsSQL;
}

void Storage::UpdateGroups(const Groups &groups_)
{
	db::connection conn(db::dbms::SQLite, dbPath);

	db::query exists_query(conn);
	exists_query.prepare("select id from groups where id = ?");

	db::transaction writeTr(conn);
	writeTr.start();

	std::lock_guard<std::recursive_mutex> lock(groupsMutex);
	for (auto &group : groups_)
	{
		exists_query.set(0, group.id);
		bool exists = exists_query.step();
		exists_query.reset();

		std::string sqlFields, sqlValues;

        if (!group.deleted)
		{
			AddSQLValue("id", group.id, sqlFields, sqlValues, exists);
			AddSQLValue("parent_id", group.parent_id, sqlFields, sqlValues, exists, false);
			AddSQLValue("tag", group.tag, sqlFields, sqlValues, exists);
			AddSQLValue("name", group.name, sqlFields, sqlValues, exists);
			AddSQLValue("owner_id", group.owner_id, sqlFields, sqlValues, exists);
			AddSQLValue("password", group.password, sqlFields, sqlValues, exists);
			AddSQLValue("grants", group.grants, sqlFields, sqlValues, exists);
		}
		AddSQLValue("deleted", group.deleted, sqlFields, sqlValues, exists);
			
		if (!sqlFields.empty()) sqlFields.pop_back(); // drop last ","
		if (!sqlValues.empty()) sqlValues.pop_back(); // drop last ","

		db::query group_query(conn);
		group_query.prepare(!exists ? "insert into groups (" + sqlFields + ") values (" + sqlValues + ")" : "update groups set " + sqlValues + " where id='" + std::to_string(group.id) + "'");
		group_query.step();
		group_query.reset();
	}

    writeTr.commit();
    LoadGroups();
	
    for (auto receiver : groupsReceivers)
    {
        receiver.second(groups_);
    }
}

void Storage::ClearGroups()
{
	db::connection conn(db::dbms::SQLite, dbPath);

	db::transaction writeTr(conn);
	writeTr.start();

	db::query del_query(conn);
	del_query.prepare("delete from groups");
	del_query.step();

	writeTr.commit();

	groups.clear();
}

void Storage::LoadGroups()
{
    std::lock_guard<std::recursive_mutex> lock(groupsMutex);

	groups.clear();

	db::connection conn(db::dbms::SQLite, dbPath);

	GetChildGroups(conn, 0, -1, groups);
}

std::recursive_mutex &Storage::GetGroupsMutex()
{
	return groupsMutex;
}

Groups &Storage::GetGroups()
{
	return groups;
}

bool Storage::IsGroupRolled(int64_t groupId)
{
	std::lock_guard<std::recursive_mutex> lock(groupsMutex);
	for (auto &group : groups)
	{
		if (group.id == groupId)
		{
			if (group.rolled)
			{
				return true;
			}
			else if (group.parent_id != 0)
			{
				return IsGroupRolled(group.parent_id);
			}
		}
	}

	return false;
}

void Storage::ChangeGroupRolled(int64_t groupId)
{
	std::lock_guard<std::recursive_mutex> lock(groupsMutex);

	for (auto &group : groups)
	{
		if (group.id == groupId)
		{
			group.rolled = !group.rolled;

			db::connection conn(db::dbms::SQLite, dbPath);
			db::transaction writeTr(conn);
			writeTr.start();

			db::query rolled_query(conn);
			rolled_query.prepare(group.rolled ? "insert into group_rolled (id) values (?)" : "delete from group_rolled where id = ?");
			rolled_query.set(0, group.id);
			rolled_query.step();
			
			writeTr.commit();

			break;
		}
	}
}

std::string Storage::SubscribeGroupsReceiver(std::function<void(const Groups &groups)> receiver)
{
    std::lock_guard<std::recursive_mutex> lock(groupsMutex);

    auto id = GUID();

    groupsReceivers[id] = receiver;

    return id;
}

void Storage::UnsubscribeGroupsReceiver(std::string_view subscriberId)
{
    std::lock_guard<std::recursive_mutex> lock(groupsMutex);

    auto it = groupsReceivers.find(subscriberId.data());
    if (it != groupsReceivers.end())
    {
        groupsReceivers.erase(it);
    }
}

/// Conferences
void Storage::UpdateConferences(const Conferences &conferences_)
{
	db::connection conn(db::dbms::SQLite, dbPath);

	db::query exists_query(conn);
	exists_query.prepare("select id from conferences where id = ?");

	db::query clear_members_query(conn);
	clear_members_query.prepare("delete from conference_members where conference_id = ?");

	db::query insert_members_query(conn);
	insert_members_query.prepare("insert into conference_members (conference_id, user_id, grants) values (?, ?, ?)");

	db::transaction writeTr(conn);
	writeTr.start();

    std::vector<std::string> newConferences;

	std::lock_guard<std::recursive_mutex> lock(conferencesMutex);
	for (auto &conference : conferences_)
	{
		exists_query.set(0, conference.id);
		bool exists = exists_query.step();
		exists_query.reset();

        std::string sqlFields, sqlValues;

        if (!conference.deleted)
		{
			AddSQLValue("id", conference.id, sqlFields, sqlValues, exists);
			AddSQLValue("tag", conference.tag, sqlFields, sqlValues, exists);
			AddSQLValue("name", conference.name, sqlFields, sqlValues, exists);
			AddSQLValue("descr", conference.descr, sqlFields, sqlValues, exists);
			AddSQLValue("founder_id", conference.founder_id, sqlFields, sqlValues, exists);
			AddSQLValue("type", static_cast<int32_t>(conference.type), sqlFields, sqlValues, exists);
			AddSQLValue("grants", conference.grants, sqlFields, sqlValues, exists);
			AddSQLValue("duration", conference.duration, sqlFields, sqlValues, exists);
			AddSQLValue("connect_members", conference.connect_members, sqlFields, sqlValues, exists);

            auto c = std::find(conferences.begin(), conferences.end(), conference.tag);
            if (c != conferences.end())
            {
                auto rolled = c->rolled;
                auto unreaded = c->unreaded_count;
                
                *c = conference;
                
                c->rolled = rolled;
                c->unreaded_count = unreaded;
            }
            else
            {
                conferences.emplace_back(conference);
                newConferences.emplace_back(conference.tag);
            }
		}
        else
        {
            auto c = std::find_if(conferences.begin(), conferences.end(), [conference](const Proto::Conference &c) { return c.id == conference.id; });
            if (c != conferences.end())
            {
                conferences.erase(c);
            }
        }
		AddSQLValue("deleted", conference.deleted, sqlFields, sqlValues, exists);

		if (!sqlFields.empty()) sqlFields.pop_back(); // drop last ","
		if (!sqlValues.empty()) sqlValues.pop_back(); // drop last ","

		db::query conference_query(conn);
		conference_query.prepare(!exists ? "insert into conferences (" + sqlFields + ") values (" + sqlValues + ")" : "update conferences set " + sqlValues + " where id='" + std::to_string(conference.id) + "'");
		conference_query.step();
		conference_query.reset();

		clear_members_query.set(0, static_cast<int32_t>(conference.id));
		clear_members_query.step();
		clear_members_query.reset();

		for (auto &m : conference.members)
		{
			insert_members_query.set(0, static_cast<int32_t>(conference.id));
			insert_members_query.set(1, m.id);
			insert_members_query.set(2, static_cast<int32_t>(m.grants));
			insert_members_query.step();
			insert_members_query.reset();
		}
	}

	writeTr.commit();

    for (auto receiver : conferencesReceivers)
    {
        receiver.second(conferences_);
    }

    if (!newConferences.empty())
    {
        Messages serviceNotifies;
        for (auto &c : newConferences)
        {
            auto message = Proto::Message(GUID(),
                time(0),
                Proto::MessageType::Join,
                0, "",
                0, "",
                0, "",
                c, "",
                Proto::MessageStatus::Created,
                "{\"type\":\"system_notice\",\"event\":\"added_to_conference\"}",
                0, Proto::CallResult::Undefined,
                "", "", "");

            serviceNotifies.emplace_back(message);
        }
        UpdateMessages(serviceNotifies);
    }
}

void Storage::LoadConferences()
{
    std::lock_guard<std::recursive_mutex> lock(conferencesMutex);

	conferences.clear();

	db::connection conn(db::dbms::SQLite, dbPath);

	db::query members_query(conn);
	members_query.prepare("select user_id, grants from conference_members where conference_id = ?");

	db::query user_query(conn);
	user_query.prepare("select state, login, name, number from users where id = ?");

	db::query contact_groups_query(conn);
	contact_groups_query.prepare("select group_id from client_groups where client_id = ?");

	db::query rolled_query(conn);
	rolled_query.prepare("select id from conference_rolled where id = ?");

	db::query conferences_query(conn);
	conferences_query.prepare("select id, tag, name, descr, founder_id, type, grants, duration, connect_members from conferences where deleted is null order by name");
	while (conferences_query.step())
	{
		auto conferenceId = conferences_query.get_int32(0);

		std::vector<Proto::Member> members;

		members_query.set(0, conferenceId);
		while (members_query.step())
		{
			auto userId = members_query.get_int64(0);
			auto grants = members_query.get_int32(1);
			Proto::MemberState state = Proto::MemberState::Undefined;
			std::string login, name, number;

			user_query.set(0, userId);
			if (user_query.step())
			{
				state = static_cast<Proto::MemberState>(user_query.get_int32(0));
				login = user_query.get_string(1);
				name = user_query.get_string(2);
				number = user_query.get_string(3);
			}
			user_query.reset();

			std::vector<Proto::Group> groups_;

			contact_groups_query.set(0, userId);
			while (contact_groups_query.step())
			{
				groups_.emplace_back(Proto::Group(contact_groups_query.get_int64(0)));
			}
			contact_groups_query.reset();

			members.emplace_back(Proto::Member(userId, state, login, name, number, groups_, grants));
		}
		members_query.reset();

		rolled_query.set(0, conferenceId);
		bool rolled = rolled_query.step();
		rolled_query.reset();

		auto conference = Proto::Conference(
			conferenceId,
			conferences_query.get_string(1),
			conferences_query.get_string(2),
			conferences_query.get_string(3), "", 
			conferences_query.get_int32(4),
			static_cast<Proto::ConferenceType>(conferences_query.get_int32(5)),
			conferences_query.get_int32(6),
			conferences_query.get_int32(7),
			members,
			conferences_query.get_int32(8) != 0,
			false,
			false,
			rolled);

        conference.unreaded_count = CalcUnreadedConference(conn, conferences_query.get_string(1));

        conferences.emplace_back(conference);
	}

    SortConferences();
}

std::recursive_mutex &Storage::GetConferencesMutex()
{
	return conferencesMutex;
}

const Conferences &Storage::GetConferences() const
{
	return conferences;
}

Proto::Conference Storage::GetConference(std::string_view tag)
{
	std::lock_guard<std::recursive_mutex> lock(conferencesMutex);
	for (auto& conference : conferences)
	{
		if (conference.tag == tag)
		{
			return conference;
		}
	}

	return {};
}

bool Storage::IsConferenceRolled(int64_t conferenceId)
{
	std::lock_guard<std::recursive_mutex> lock(conferencesMutex);
	for (auto &conference : conferences)
	{
		if (conference.id == conferenceId)
		{
			return conference.rolled;
		}
	}

	return false;
}

void Storage::ChangeConferenceRolled(int64_t conferenceId)
{
	std::lock_guard<std::recursive_mutex> lock(conferencesMutex);

	for (auto &conference : conferences)
	{
		if (conference.id == conferenceId)
		{
			conference.rolled = !conference.rolled;

			db::connection conn(db::dbms::SQLite, dbPath);
			db::transaction writeTr(conn);
			writeTr.start();

			db::query rolled_query(conn);
			rolled_query.prepare(conference.rolled ? "insert into conference_rolled (id) values (?)" : "delete from conference_rolled where id = ?");
			rolled_query.set(0, conferenceId);
			rolled_query.step();

			writeTr.commit();

			break;
		}
	}
}

std::string Storage::SubscribeConferencesReceiver(std::function<void(const Conferences &conferences)> receiver)
{
    std::lock_guard<std::recursive_mutex> lock(conferencesMutex);

    auto id = GUID();

    conferencesReceivers[id] = receiver;

    return id;
}

void Storage::UnsubscribeConferencesReceiver(std::string_view subscriberId)
{
    std::lock_guard<std::recursive_mutex> lock(conferencesMutex);

    auto it = conferencesReceivers.find(subscriberId.data());
    if (it != conferencesReceivers.end())
    {
        conferencesReceivers.erase(it);
    }
}

void Storage::UpdateDB()
{
	std::string currentDBVersion = "";

	db::connection conn(db::dbms::SQLite, dbPath);

	db::query get_ver_query(conn);
	get_ver_query.prepare("select db_version from db_version");
	if (get_ver_query.step())
	{
		currentDBVersion = get_ver_query.get_string(0);
	}
	get_ver_query.close();

	if (currentDBVersion == "") // no database, creating
	{
		currentDBVersion = "2.0.230304";

		db::transaction writeTr(conn);
		writeTr.start();

		db::query users_query(conn);
		users_query.prepare("CREATE TABLE `users` (`id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, `login` TEXT, `name` TEXT, `number` TEXT, `email` TEXT, `bio` TEXT, `icon` TEXT, `avatar` TEXT, `state` INTEGER, `deleted` INTEGER)");
		users_query.step();

		db::query groups_query(conn);
		groups_query.prepare("CREATE TABLE \"groups\" (\"id\" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, \"parent_id\" INTEGER NOT NULL, \"tag\" TEXT NOT NULL, \"name\" TEXT NOT NULL, \"password\" TEXT, \"grants\" INTEGER, \"owner_id\" integer, \"deleted\" INTEGER)");
		groups_query.step();

		db::query client_groups_query(conn);
		client_groups_query.prepare("CREATE TABLE \"client_groups\" (\"id\" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, \"client_id\" INTEGER NOT NULL, \"group_id\" INTEGER NOT NULL, \"deleted\" INTEGER);");
		client_groups_query.step();

		db::query group_rolled_query(conn);
		group_rolled_query.prepare("CREATE TABLE \"group_rolled\" (\"id\" INTEGER NOT NULL PRIMARY KEY);");
		group_rolled_query.step();

		db::query conference_rolled_query(conn);
		conference_rolled_query.prepare("CREATE TABLE \"conference_rolled\" (\"id\" INTEGER NOT NULL PRIMARY KEY);");
		conference_rolled_query.step();

		db::query conferences_query(conn);
		conferences_query.prepare("CREATE TABLE `conferences` (`id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, `tag` TEXT NOT NULL, `name` TEXT NOT NULL, `descr` TEXT, `founder_id` INTEGER, `type` INTEGER, `grants` INTEGER, `duration` INTEGER, `connect_members` INTEGER, `deleted` INTEGER);");
		conferences_query.step();

		db::query conference_members_query(conn);
		conference_members_query.prepare("CREATE TABLE `conference_members` (`conference_id` INTEGER NOT NULL, `user_id` INTEGER NOT NULL, `grants` INTEGER, PRIMARY KEY(`conference_id`, `user_id`));");
		conference_members_query.step();

		db::query messages_query(conn);
		messages_query.prepare("CREATE TABLE \"messages\" ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, `guid`	TEXT NOT NULL UNIQUE, `dt` INTEGER, `type` INTEGER, `author_id` INTEGER, `sender_id` INTEGER, `subscriber_id` INTEGER, `conference_tag` TEXT, `text_value` TEXT, `call_duration` INTEGER, `call_result` INTEGER, `data_preview` TEXT, `data_url` TEXT, `data_local_path` TEXT, status INTEGER )");
		messages_query.step();

		db::query messages_guid_index_query(conn);
		messages_guid_index_query.prepare("CREATE INDEX `messages_guid_index` ON `messages` ( `guid` )");
		messages_guid_index_query.step();

		db::query messages_author_id_index_query(conn);
		messages_author_id_index_query.prepare("CREATE INDEX `messages_author_id_index` ON `messages` ( `author_id` )");
		messages_author_id_index_query.step();

		db::query messages_sender_id_index_query(conn);
		messages_sender_id_index_query.prepare("CREATE INDEX `messages_sender_id_index` ON `messages` ( `sender_id` )");
		messages_sender_id_index_query.step();

		db::query messages_subscriber_id_index_query(conn);
		messages_subscriber_id_index_query.prepare("CREATE INDEX `messages_subscriber_id_index` ON `messages` ( `subscriber_id` )");
		messages_subscriber_id_index_query.step();

		db::query messages_conference_tag_index_query(conn);
		messages_conference_tag_index_query.prepare("CREATE INDEX `messages_conference_tag_index` ON `messages` ( `conference_tag` )");
		messages_conference_tag_index_query.step();

		db::query messages_dt_index_query(conn);
		messages_dt_index_query.prepare("CREATE INDEX `messages_dt_index` ON `messages` ( `dt` DESC )");
		messages_dt_index_query.step();

		db::query conferences_tag_index_query(conn);
		conferences_tag_index_query.prepare("CREATE UNIQUE INDEX `conferences_tag_index` ON `conferences` ( `tag` )");
		conferences_tag_index_query.step();

		db::query create_settings_query(conn);
		create_settings_query.prepare("CREATE TABLE \"settings\" (\"id\" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, \"key\" TEXT, \"value\" TEXT);");
		create_settings_query.step();

		db::query insert_settings_query(conn);
		insert_settings_query.prepare("insert into settings (key, value) values ('users_sort_type', '0')");
		insert_settings_query.step();
        insert_settings_query.prepare("insert into settings (key, value) values ('show_number_on_contact_list', '0')");
        insert_settings_query.step();
        insert_settings_query.prepare("insert into settings (key, value) values ('my_client_id', '0')");
        insert_settings_query.step();

		db::query db_version_query(conn);
		db_version_query.prepare("CREATE TABLE `db_version` ( `db_version` TEXT )");
		db_version_query.step();
		
		db::query upd_db_version_query(conn);
		upd_db_version_query.prepare("insert into db_version (db_version) values ('" + currentDBVersion + "')");
		upd_db_version_query.step();

		writeTr.commit();
	}
	if (currentDBVersion == "1.4.190606")
	{
		currentDBVersion = "1.4.191016";

		db::transaction writeTr(conn);
		writeTr.start();

		db::query create_settings_query(conn);
		create_settings_query.prepare("CREATE TABLE \"settings\" (\"id\" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, \"key\" TEXT, \"value\" TEXT);");
		create_settings_query.step();

		db::query insert_settings_query(conn);
		insert_settings_query.prepare("insert into settings (key, value) values ('users_sort_type', '0')");
		insert_settings_query.step();

		db::query upd_db_version_query(conn);
		upd_db_version_query.prepare("update db_version set db_version = '" + currentDBVersion + "'");
		upd_db_version_query.step();

		writeTr.commit();
	}
	if (currentDBVersion == "1.4.191016")
	{
		currentDBVersion = "1.4.200506";

		db::transaction writeTr(conn);
		writeTr.start();

		db::query alter_users_query(conn);
		alter_users_query.prepare("alter table `users` add `login` text;");
		alter_users_query.step();

		db::query upd_db_version_query(conn);
		upd_db_version_query.prepare("update db_version set db_version = '" + currentDBVersion + "'");
		upd_db_version_query.step();

		writeTr.commit();
	}
	if (currentDBVersion == "1.4.200506")
	{
		currentDBVersion = "1.4.210325";

		db::transaction writeTr(conn);
		writeTr.start();

		db::query alter_users_query(conn);
		alter_users_query.prepare("alter table users add group_id integer not null default 1;");
		alter_users_query.step();

		db::query groups_query(conn);
		groups_query.prepare("CREATE TABLE \"groups\" (\"id\" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, \"parent_id\" INTEGER NOT NULL, \"tag\" TEXT NOT NULL, \"name\" TEXT NOT NULL, \"password\" TEXT, \"grants\" INTEGER, \"deleted\"	INTEGER)");
		groups_query.step();

		db::query upd_db_version_query(conn);
		upd_db_version_query.prepare("update db_version set db_version = '" + currentDBVersion + "'");
		upd_db_version_query.step();

		writeTr.commit();
	}
	if (currentDBVersion == "1.4.210325")
	{
		currentDBVersion = "1.5.210426";

		db::transaction writeTr(conn);
		writeTr.start();

		db::query client_groups_query(conn);
		client_groups_query.prepare("CREATE TABLE \"client_groups\" (\"id\" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, \"client_id\" INTEGER NOT NULL, \"group_id\" INTEGER NOT NULL, \"deleted\" INTEGER);");
		client_groups_query.step();

		db::query upd_db_version_query(conn);
		upd_db_version_query.prepare("update db_version set db_version = '" + currentDBVersion + "'");
		upd_db_version_query.step();

		writeTr.commit();
	}
	if (currentDBVersion == "1.5.210426")
	{
		currentDBVersion = "1.5.210615";

		db::transaction writeTr(conn);
		writeTr.start();

		db::query client_groups_query(conn);
		client_groups_query.prepare("alter table groups add owner_id integer;");
		client_groups_query.step();

		db::query upd_db_version_query(conn);
		upd_db_version_query.prepare("update db_version set db_version = '" + currentDBVersion + "'");
		upd_db_version_query.step();

		writeTr.commit();
	}
	if (currentDBVersion == "1.5.210615")
	{
		currentDBVersion = "1.5.210626";

		db::transaction writeTr(conn);
		writeTr.start();

		db::query group_rolled_query(conn);
		group_rolled_query.prepare("CREATE TABLE \"group_rolled\" (\"id\" INTEGER NOT NULL PRIMARY KEY);");
		group_rolled_query.step();

		db::query upd_db_version_query(conn);
		upd_db_version_query.prepare("update db_version set db_version = '" + currentDBVersion + "'");
		upd_db_version_query.step();

		writeTr.commit();
	}
	if (currentDBVersion == "1.5.210626")
	{
		currentDBVersion = "1.6.211107";

		db::transaction writeTr(conn);
		writeTr.start();

		db::query conference_rolled_query(conn);
		conference_rolled_query.prepare("CREATE TABLE \"conference_rolled\" (\"id\" INTEGER NOT NULL PRIMARY KEY);");
		conference_rolled_query.step();

		db::query upd_db_version_query(conn);
		upd_db_version_query.prepare("update db_version set db_version = '" + currentDBVersion + "'");
		upd_db_version_query.step();

		writeTr.commit();
	}
	if (currentDBVersion == "1.6.211107")
	{
		currentDBVersion = "1.6.211122";

		db::transaction writeTr(conn);
		writeTr.start();

		db::query conference_rolled_query(conn);
		conference_rolled_query.prepare("alter table conference_members add grants integer;");
		conference_rolled_query.step();

		db::query upd_db_version_query(conn);
		upd_db_version_query.prepare("update db_version set db_version = '" + currentDBVersion + "'");
		upd_db_version_query.step();

		writeTr.commit();
	}
    if (currentDBVersion == "1.6.211122")
    {
        currentDBVersion = "2.0.221122";

        db::transaction writeTr(conn);
        writeTr.start();

        db::query insert_settings_query(conn);
        insert_settings_query.prepare("insert into settings (key, value) values ('my_client_id', '0')");
        insert_settings_query.step();

        db::query upd_db_version_query(conn);
        upd_db_version_query.prepare("update db_version set db_version = '" + currentDBVersion + "'");
        upd_db_version_query.step();

        writeTr.commit();
    }
    if (currentDBVersion == "2.0.221122")
    {
        currentDBVersion = "2.0.221204";

        db::transaction writeTr(conn);
        writeTr.start();

        db::query alter_query(conn);
        alter_query.prepare("alter table users add bio TEXT null;");
        alter_query.step();

        alter_query.prepare("alter table users add email TEXT null;");
        alter_query.step();

        db::query upd_db_version_query(conn);
        upd_db_version_query.prepare("update db_version set db_version = '" + currentDBVersion + "'");
        upd_db_version_query.step();

        writeTr.commit();
    }
    if (currentDBVersion == "2.0.221204")
    {
        currentDBVersion = "2.0.230304";

        db::transaction writeTr(conn);
        writeTr.start();

        db::query insert_settings_query(conn);
        insert_settings_query.prepare("insert into settings (key, value) values ('show_number_on_contact_list', '0')");
        insert_settings_query.step();

        db::query upd_db_version_query(conn);
        upd_db_version_query.prepare("update db_version set db_version = '" + currentDBVersion + "'");
        upd_db_version_query.step();

        writeTr.commit();
    }
}

}
