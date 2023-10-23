/**
 * Grants.cpp - Contains license grants parser impl
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2018
 */

#include <License/Grants.h>

#include <Common/BitHelpers.h>

namespace License
{

Grants::Grants()
	: allowedWork(false),
	freeLicense(false),
	allowedAutonomousWork(false),
	allowedCrypto(false),
	allowedRecord(false),
	allowedSymmetricConf(false),
	allowedAsymmetricConf(false),
	allowedCreatingConferences(false),
	denyUDP(false)
{}


Grants Parse(uint32_t grants)
{
	Grants out;

	out.allowedWork                = BitIsSet(grants, 0);
	out.freeLicense                = BitIsSet(grants, 1);
	out.allowedAutonomousWork      = BitIsSet(grants, 2);
	out.allowedCrypto              = BitIsSet(grants, 3);
	out.allowedRecord              = BitIsSet(grants, 10);
	out.allowedSymmetricConf       = BitIsSet(grants, 11);
	out.allowedAsymmetricConf      = BitIsSet(grants, 12);
	
	out.allowedCreatingConferences = BitIsSet(grants, 17);
	out.denyUDP                    = BitIsSet(grants, 30);

	return out;
}

uint32_t Serialize(Grants grants)
{
	uint32_t out = 0;

	if (grants.allowedWork)                SetBit(out, 0);
	if (grants.freeLicense)                SetBit(out, 1);
	if (grants.allowedAutonomousWork)      SetBit(out, 2);
	if (grants.allowedCrypto)              SetBit(out, 3);
	if (grants.allowedRecord)              SetBit(out, 10);
	if (grants.allowedSymmetricConf)       SetBit(out, 11);
	if (grants.allowedAsymmetricConf)      SetBit(out, 12);

	if (grants.allowedCreatingConferences) SetBit(out, 17);
	if (grants.denyUDP)                    SetBit(out, 30);
	
	return out;
}

}
