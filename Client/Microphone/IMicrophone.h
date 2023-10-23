/**
* IMicrophone.h - Contains interface of microphone (or other audio capture source)
*
* Author: Anton (ud) Golovkov, udattsk@gmail.com
* Copyright (C), Infinity Video Soft LLC, 2014
*/

#pragma once

#include <cstdint>
#include <string>

namespace MicrophoneNS
{
	class IMicrophone
	{
	public:
		virtual void SetDeviceName(const std::string &name) = 0;
		virtual void SetDeviceId(uint32_t id) = 0;
		
		virtual void Start() = 0;
		virtual void Stop() = 0;

		virtual void SetGain(uint16_t gain) = 0;
        virtual uint16_t GetGain() const = 0;
		
		virtual void SetMute(bool yes) = 0;
		virtual bool GetMute() const = 0;

        virtual void SetSampleFreq(int32_t freq) = 0;
        virtual int32_t GetSampleFreq() const = 0;
    
    protected:
		~IMicrophone() {}
	};
}
