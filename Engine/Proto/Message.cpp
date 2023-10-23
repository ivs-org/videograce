/**
 * Message.cpp - Contains member structure impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2019
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <Proto/Message.h>

#include <Common/Common.h>
#include <Common/Quoter.h>
#include <Common/JSONSymbolsScreener.h>

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

Message::Message(const std::string &guid_, time_t dt_, MessageType type_, int64_t author_id_, const std::string &author_name_, int64_t sender_id_, const std::string &sender_name_, int64_t subscriber_id_, const std::string &subscriber_name_,
	const std::string &conference_tag_, const std::string &conference_name_, MessageStatus status_, const std::string &text_, int32_t call_duration_, CallResult call_result_, const std::string &preview_, const std::string &data_, const std::string &url_)
	: guid(guid_), dt(dt_), type(type_), author_id(author_id_), author_name(author_name_), sender_id(sender_id_), sender_name(sender_name_), subscriber_id(subscriber_id_), subscriber_name(subscriber_name_),
	conference_tag(conference_tag_), conference_name(conference_name_), status(status_), text(text_), call_duration(call_duration_), call_result(call_result_), preview(preview_), data(data_), url(url_)
{
}

Message::~Message()
{
}

bool Message::Parse(const boost::property_tree::ptree &pt)
{
	try
	{
		guid = pt.get<std::string>(GUID);
		
		auto dt_opt = pt.get_optional<uint64_t>(DT);
		if (dt_opt) dt = dt_opt.get();
		
		auto type_opt = pt.get_optional<uint32_t>(TYPE);
		if (type_opt) type = static_cast<MessageType>(type_opt.get());
		
		auto author_id_opt = pt.get_optional<int64_t>(AUTHOR_ID);
		if (author_id_opt) author_id = author_id_opt.get();
		
		auto author_name_opt = pt.get_optional<std::string>(AUTHOR_NAME);
		if (author_name_opt) author_name = author_name_opt.get();
		
		auto sender_id_opt = pt.get_optional<int64_t>(SENDER_ID);
		if (sender_id_opt) sender_id = sender_id_opt.get();
		
		auto sender_name_opt = pt.get_optional<std::string>(SENDER_NAME);
		if (sender_name_opt) sender_name = sender_name_opt.get();
		
		auto subscriber_id_opt = pt.get_optional<int64_t>(SUBSCRIBER_ID);
		if (subscriber_id_opt) subscriber_id = subscriber_id_opt.get();
		
		auto subscriber_name_opt = pt.get_optional<std::string>(SUBSCRIBER_NAME);
		if (subscriber_name_opt) subscriber_name = subscriber_name_opt.get();
		
		auto conference_tag_opt = pt.get_optional<std::string>(CONFERENCE_TAG);
		if (conference_tag_opt) conference_tag = conference_tag_opt.get();
		
		auto conference_name_opt = pt.get_optional<std::string>(CONFERENCE_NAME);
		if (conference_name_opt) conference_name = conference_name_opt.get();
		
		auto status_opt = pt.get_optional<uint32_t>(STATUS);
		if (status_opt) status = static_cast<MessageStatus>(status_opt.get());
		
		auto text_opt = pt.get_optional<std::string>(TEXT);
		if (text_opt) text = text_opt.get();
		
		auto call_result_opt = pt.get_optional<uint32_t>(CALL_RESULT);
		if (call_result_opt) call_result = static_cast<CallResult>(call_result_opt.get());
		
		auto call_duration_opt = pt.get_optional<uint32_t>(CALL_DURATION);
		if (call_duration_opt) call_duration = call_duration_opt.get();
		
		auto preview_opt = pt.get_optional<std::string>(PREVIEW);
		if (preview_opt) preview = preview_opt.get();
		
		auto data_opt = pt.get_optional<std::string>(DATA);
		if (data_opt) data = data_opt.get();
		
		auto url_opt = pt.get_optional<std::string>(URL);
		if (url_opt) url = url_opt.get();
		
		return true;
	}
	catch (std::exception const& e)
	{
		DBGTRACE("Error parsing message, %s\n", e.what());
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
