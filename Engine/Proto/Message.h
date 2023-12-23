/**
 * Message.h - Contains the message structure
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2019
 */

#pragma once

#include <string>
#include <cstdint>

#include <nlohmann/json.hpp>

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
		Message(std::string_view guid, time_t dt, MessageType type, int64_t author_id, std::string_view author_name, int64_t sender_id, std::string_view sender_name, int64_t subscriber_id, std::string_view subscriber_name,
			std::string_view conference_tag, std::string_view conference_name, MessageStatus status, std::string_view text, int32_t call_duration, CallResult call_result, std::string_view preview, std::string_view data, std::string_view url);

		~Message();

		bool Parse(const nlohmann::json::object_t& obj);
		std::string Serialize();

        void Clear();

		inline bool operator==(std::string_view guid_)
		{
			return guid == guid_;
		}
	};
}
