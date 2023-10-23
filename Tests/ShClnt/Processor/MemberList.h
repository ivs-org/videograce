/**
 * IMemberList.h - Contains member list interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2017
 */

#pragma once

#include <cstdint>
#include <vector>
#include <mutex>

#include <Controller/IMemberList.h>

namespace Processor
{
	class MemberList : public Controller::IMemberList
	{
	public:
        virtual std::recursive_mutex &GetItemsMutex() { return dummyMutex; }

		virtual void UpdateItem(const Proto::Member& item) {}
        virtual void DeleteItem(int64_t memberId) {}
		virtual void ClearItems() {}

        virtual const Controller::members_t &GetItems() const { return dummyMembers; }

		virtual bool ExistsMember(int64_t id) { return false; }
        virtual std::string GetMemberName(int64_t id) { return ""; }

		virtual uint32_t GetMaximumInputBitrate() { return 0; }

		virtual uint32_t GetSpeakersCount() { return 0; }

        virtual bool IsMePresenter() { return false; }
		virtual bool IsMeModerator() { return false; }
		virtual bool IsMeReadOnly() { return false; }

		virtual bool IsMeSpeaker() { return false; }
		virtual void TurnMeSpeaker() {}

		virtual void Update() {}

        virtual void Add() {}
        virtual void Kick() {}
        virtual void ToTop() {}
        virtual void MuteAll() {}
        virtual void TurnSpeak() {}
        virtual void Devices() {}
        virtual void TurnCamera() {}
        virtual void TurnMicrophone() {}
        virtual void TurnDemostration() {}
	
        MemberList() {}
		~MemberList() {}

    private:
        std::recursive_mutex dummyMutex;
        Controller::members_t dummyMembers;
	};
}
