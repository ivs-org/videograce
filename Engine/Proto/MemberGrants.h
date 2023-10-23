/**
 * MemberGrants.h - Contains member's grants enumeration
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2021
 */

#pragma once

namespace Proto
{
	enum class MemberGrants
	{
		Presenter = 10,
		Speaker = 11,
		Moderator = 12,
		Ordinary = 13,
		ReadOnly = 14,
        Deaf = 15
	};
}
