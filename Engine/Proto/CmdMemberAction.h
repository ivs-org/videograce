/**
 * CmdMemberAction.h - Contains protocol command MEMBER_ACTION
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2017
 */

#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace Proto
{
namespace MEMBER_ACTION
{
	static const std::string NAME = "member_action";

	enum class Action
	{
		Undefined = 0,

		TurnCamera,
		TurnMicrophone,
		TurnDemonstration,
		TurnSpeaker,
		MoveToTop,
		EnableRemoteControl,
		DisableRemoteControl,
        MuteMicrophone,
		DisconnectFromConference,
        ChangeGrants
	};

	enum class Result
	{
		Undefined = 0,

		OK,
		NotAllowed,
		Accepted,
		Rejected,
		Busy
	};

	struct Command
	{
		std::vector<int64_t> ids;
		Action action;
		Result result;

		int64_t actor_id;
		std::string actor_name;

        uint32_t grants;

		Command();
        Command(const std::vector<int64_t> &ids, Action action, uint32_t grants);
		Command(Action action, Result result, int64_t actor_id, const std::string &actor_name);
		Command(int64_t actor_id, Result result);
		Command(Result result);

		~Command();

		bool Parse(const std::string &message);
		std::string Serialize();
	};
}
}
