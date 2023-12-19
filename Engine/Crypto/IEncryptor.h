/**
 * IEncryptor.h - Contains encryptor interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2016
 */

#pragma once

#include <string>

namespace Crypto
{
	class IEncryptor
	{
	public:
		virtual void Start(std::string_view secureKey) = 0;
		virtual void Stop() = 0;

        virtual bool Started() const = 0;
		
	protected:
		~IEncryptor() {}
	};
}
