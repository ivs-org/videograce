/**
 * Message.h - Contains the message structure
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2019
 */

#pragma once

#include <string>
#include <cstdint>

#include <boost/property_tree/ptree.hpp>

namespace Proto
{
	enum class MessageType
	{
		Undefined = 0,

		TextMessage,
		Call,
		Join,
		Leave,
		Image,
		Document,
		Forwarded,
		Video,
		VoiceMessage,
		VideoMessage,
		Typing,
		RecordingVoice,
		RecordingVideo,
		ServiceMessage
	};

	enum class CallResult
	{
		Undefined = 0,

		Answered,
		Missed,
		Rejected,
		Busy,
		Offline
	};

	enum class MessageStatus
	{
		Undefined = 0,
		Created,
		Sended,
		Delivered,
		Readed,
		Modified,
		Deleted
	};

	struct Message
	{
		std::string guid;

		time_t dt;

		MessageType type;

		int64_t author_id;
		std::string author_name;

		int64_t sender_id;
		std::string sender_name;

		int64_t subscriber_id;
		std::string subscriber_name;

		std::string conference_tag, conference_name;

		MessageStatus status;

		/// Payload
		std::string text;

		int32_t call_duration;
		CallResult call_result;
		
		std::string preview;
		std::string data;
		std::string url;

		Message();
		Message(const std::string &guid, time_t dt, MessageType type, int64_t author_id, const std::string &author_name, int64_t sender_id, const std::string &sender_name, int64_t subscriber_id, const std::string &subscriber_name,
			const std::string &conference_tag, const std::string &conference_name, MessageStatus status, const std::string &text, int32_t call_duration, CallResult call_result, const std::string &preview, const std::string &data, const std::string &url);

		~Message();

		bool Parse(const boost::property_tree::ptree &pt);
		std::string Serialize();

        void Clear();

		inline bool operator==(const std::string &guid_)
		{
			return guid == guid_;
		}
	};
}
