/**
 * Message.cpp - Contains member structure impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2019, 2023
 */

#include <Proto/Message.h>

#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

#include <spdlog/spdlog.h>

namespace Proto
{

static const std::string GUID = "guid";
static const std::string DT = "dt";
static const std::string TYPE = "type";
static const std::string AUTHOR_ID = "author_id";
static const std::string AUTHOR_NAME = "author_name";
static const std::string SENDER_ID = "sender_id";
static const std::string SENDER_NAME = "sender_name";
static const std::string SUBSCRIBER_ID = "subscriber_id";
static const std::string SUBSCRIBER_NAME = "subscriber_name";
static const std::string CONFERENCE_TAG = "conference_tag";
static const std::string CONFERENCE_NAME = "conference_name";
static const std::string STATUS = "status";
static const std::string TEXT = "text";
static const std::string CALL_DURATION = "call_duration";
static const std::string CALL_RESULT = "call_result";
static const std::string PREVIEW = "preview";
static const std::string DATA = "data";
static const std::string URL = "url";

Message::Message()
	: guid(), dt(0), type(MessageType::Undefined), author_id(0), author_name(), sender_id(0), sender_name(), subscriber_id(0), subscriber_name(), conference_tag(), conference_name(), status(MessageStatus::Undefined), text(), call_duration(0), call_result(CallResult::Undefined), preview(), data(), url()
{
}

Message::Message(std::string_view guid_, time_t dt_, MessageType type_, int64_t author_id_, std::string_view author_name_, int64_t sender_id_, std::string_view sender_name_, int64_t subscriber_id_, std::string_view subscriber_name_,
	std::string_view conference_tag_, std::string_view conference_name_, MessageStatus status_, std::string_view text_, int32_t call_duration_, CallResult call_result_, std::string_view preview_, std::string_view data_, std::string_view url_)
	: guid(guid_), dt(dt_), type(type_), author_id(author_id_), author_name(author_name_), sender_id(sender_id_), sender_name(sender_name_), subscriber_id(subscriber_id_), subscriber_name(subscriber_name_),
	conference_tag(conference_tag_), conference_name(conference_name_), status(status_), text(text_), call_duration(call_duration_), call_result(call_result_), preview(preview_), data(data_), url(url_)
{
}

Message::~Message()
{
}

bool Message::Parse(const nlohmann::json::object_t& obj)
{
	try
	{
		guid = obj.at(GUID).get<std::string>();

		if (obj.count(DT) != 0) dt = obj.at(DT).get<uint64_t>();
		if (obj.count(TYPE) != 0) type = static_cast<MessageType>(obj.at(TYPE).get<uint32_t>());
		if (obj.count(AUTHOR_ID) != 0) author_id = obj.at(AUTHOR_ID).get<int64_t>();
		if (obj.count(AUTHOR_NAME) != 0) author_name = obj.at(AUTHOR_NAME).get<std::string>();
		if (obj.count(SENDER_ID) != 0) sender_id = obj.at(SENDER_ID).get<int64_t>();
		if (obj.count(SENDER_NAME) != 0) sender_name = obj.at(SENDER_NAME).get<std::string>();
		if (obj.count(SUBSCRIBER_ID) != 0) subscriber_id = obj.at(SUBSCRIBER_ID).get<int64_t>();
		if (obj.count(SUBSCRIBER_NAME) != 0) subscriber_name = obj.at(SUBSCRIBER_NAME).get<std::string>();
		if (obj.count(CONFERENCE_TAG) != 0) conference_tag = obj.at(CONFERENCE_TAG).get<std::string>();
		if (obj.count(CONFERENCE_NAME) != 0) conference_name = obj.at(CONFERENCE_NAME).get<std::string>();
		if (obj.count(STATUS) != 0) status = static_cast<MessageStatus>(obj.at(STATUS).get<uint32_t>());
		if (obj.count(TEXT) != 0) text = obj.at(TEXT).get<std::string>();
		if (obj.count(CALL_RESULT) != 0) call_result = static_cast<CallResult>(obj.at(CALL_RESULT).get<uint32_t>());
		if (obj.count(CALL_DURATION) != 0) call_duration = obj.at(CALL_DURATION).get<int32_t>();
		if (obj.count(PREVIEW) != 0) preview = obj.at(PREVIEW).get<std::string>();
		if (obj.count(DATA) != 0) data = obj.at(DATA).get<std::string>();
		if (obj.count(URL) != 0) url = obj.at(URL).get<std::string>();
		
		return true;
	}
	catch (nlohmann::json::parse_error& ex)
	{
		spdlog::get("Error")->critical("proto::message :: error parse json (byte: {0}, what: {1})", ex.byte, ex.what());
	}
	return false;
}

std::string Message::Serialize()
{
	std::string out = "{" + quot(GUID) + ":" + quot(guid) + "," +
		(dt != 0 ? quot(DT) + ":" + std::to_string(dt) + "," : "") +
		(type != MessageType::Undefined ? quot(TYPE) + ":" + std::to_string(static_cast<int32_t>(type)) + "," : "") +
		(author_id != 0 ? quot(AUTHOR_ID) + ":" + std::to_string(author_id) + "," : "") +
		(!author_name.empty() ? quot(AUTHOR_NAME) + ":" + quot(Common::JSON::Screen(author_name)) + "," : "") +
		(sender_id != 0 ? quot(SENDER_ID) + ":" + std::to_string(sender_id) + "," : "") +
		(!sender_name.empty() ? quot(SENDER_NAME) + ":" + quot(Common::JSON::Screen(sender_name)) + "," : "") +
		(subscriber_id != 0 ? quot(SUBSCRIBER_ID) + ":" + std::to_string(subscriber_id) + "," : "") +
		(!subscriber_name.empty() ? quot(SUBSCRIBER_NAME) + ":" + quot(Common::JSON::Screen(subscriber_name)) + "," : "") +
		(!conference_tag.empty() ? quot(CONFERENCE_TAG) + ":" + quot(Common::JSON::Screen(conference_tag)) + "," : "") +
		(!conference_name.empty() ? quot(CONFERENCE_NAME) + ":" + quot(Common::JSON::Screen(conference_name)) + "," : "") +
		(status != MessageStatus::Undefined ? quot(STATUS) + ":" + std::to_string(static_cast<int32_t>(status)) + "," : "") +
		(!text.empty() ? quot(TEXT) + ":" + quot(Common::JSON::Screen(text)) + "," : "") +
		(call_result != CallResult::Undefined ? quot(CALL_RESULT) + ":" + std::to_string(static_cast<int32_t>(call_result)) + "," : "") +
		(call_duration != 0 ? quot(CALL_DURATION) + ":" + std::to_string(call_duration) + "," : "") +
		(!preview.empty() ? quot(PREVIEW) + ":" + quot(Common::JSON::Screen(preview)) + "," : "") +
		(!data.empty() ? quot(DATA) + ":" + quot(Common::JSON::Screen(data)) + "," : "") +
		(!url.empty() ? quot(URL) + ":" + quot(Common::JSON::Screen(url)) + "," : "");
	out.pop_back(); // drop last ","
	return out + "}";
}

void Message::Clear()
{
    guid.clear();
    dt = 0;
    type = MessageType::Undefined;
    author_id = 0;
    author_name.clear();
    sender_id = 0;
    sender_name.clear();
    subscriber_id = 0;
    subscriber_name.clear();
    conference_tag.clear();
    conference_name.clear();
    status = MessageStatus::Undefined;
    text.clear();
    call_duration = 0;
    call_result = CallResult::Undefined;
    preview.clear();
    data.clear();
    url.clear();
}

}
