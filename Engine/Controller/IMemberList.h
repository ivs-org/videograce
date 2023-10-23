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

#include <Proto/Member.h>

namespace Controller
{
	typedef std::vector<Proto::Member> members_t;

	class IMemberList
	{
	public:
		virtual std::recursive_mutex &GetItemsMutex() = 0;

		virtual void UpdateItem(const Proto::Member& item) = 0;
		virtual void DeleteItem(int64_t memberId) = 0;
		virtual void ClearItems() = 0;

		virtual const members_t &GetItems() const = 0;

		virtual bool ExistsMember(int64_t id) = 0;
		virtual std::string GetMemberName(int64_t id) = 0;

		virtual uint32_t GetMaximumInputBitrate() = 0;

		virtual uint32_t GetSpeakersCount() = 0;

		virtual bool IsMePresenter() = 0;
		virtual bool IsMeModerator() = 0;
		virtual bool IsMeReadOnly() = 0;

		virtual bool IsMeSpeaker() = 0;
		virtual void TurnMeSpeaker() = 0;

		virtual void Update() = 0;

        virtual void Add() = 0;
        virtual void Kick() = 0;
        virtual void ToTop() = 0;
        virtual void MuteAll() = 0;
        virtual void TurnSpeak() = 0;
        virtual void Devices() = 0;
        virtual void TurnCamera() = 0;
        virtual void TurnMicrophone() = 0;
        virtual void TurnDemostration() = 0;

	protected:
		~IMemberList() {}
	};
}
