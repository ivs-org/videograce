/**
 * Grants.h - Contains license grants parser
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#pragma once

#include <cstdint>

namespace License
{

struct Grants
{
	/// Licence's grants
	bool allowedWork;
	bool freeLicense;
	bool allowedAutonomousWork;
	bool allowedCrypto;
	bool allowedRecord;
	bool allowedSymmetricConf;
	bool allowedAsymmetricConf;

	/// User's grants
	bool allowedCreatingConferences;
	bool denyUDP;
	
	Grants();
};

Grants Parse(uint32_t grants);
uint32_t Serialize(Grants grants);

}
