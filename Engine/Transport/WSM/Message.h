/**
 * Message.h - Contains TCP message impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <cstdint>

#ifndef _WIN32
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif

namespace Transport
{

class message
{
public:
	enum { header_length = 6 };
	enum { max_body_length = 2048 };

	message()
		: body_length_(0)
	{
	}

	const char* data() const
	{
		return data_;
	}

	char* data()
	{
		return data_;
	}

	size_t length() const
	{
		return header_length + body_length_;
	}

	const char* body() const
	{
		return data_ + header_length;
	}

	char* body()
	{
		return data_ + header_length;
	}

	uint16_t body_length() const
	{
		return body_length_;
	}

	void body_length(uint16_t new_length)
	{
		body_length_ = new_length;
		if (body_length_ > max_body_length)
			body_length_ = max_body_length;
	}

	uint16_t dest_port() const
	{
		return dest_port_;
	}

	void dest_port(uint16_t new_port)
	{
		dest_port_ = new_port;
	}

	uint16_t src_port() const
	{
		return src_port_;
	}

	void src_port(uint16_t new_port)
	{
		src_port_ = new_port;
	}

	bool decode_header()
	{
		body_length_ = ntohs(*reinterpret_cast<const uint16_t*>(data_));
		
		if (body_length_ > max_body_length)
		{
			body_length_ = 0;
			return false;
		}

		dest_port_ = ntohs(*reinterpret_cast<const uint16_t*>(data_ + 2));
		src_port_ = ntohs(*reinterpret_cast<const uint16_t*>(data_ + 4));
		
		return true;
	}

	void encode_header()
	{
		*reinterpret_cast<uint16_t*>(data_) = htons(body_length_);
		*reinterpret_cast<uint16_t*>(data_ + 2) = htons(dest_port_);
		*reinterpret_cast<uint16_t*>(data_ + 4) = htons(src_port_);
	}

	void write(const uint8_t *data__, uint16_t len)
	{
		memcpy(data_ + header_length, data__, len < body_length_ ? len : body_length_);
	}

private:
	char data_[header_length + max_body_length];
	uint16_t body_length_;
	uint16_t dest_port_, src_port_;
};

}
