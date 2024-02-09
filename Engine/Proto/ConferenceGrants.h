/**
 * ConferenceGrants.h - Contains the conference grants enumeration
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#pragma once

namespace Proto
{
	enum class ConferenceGrants
	{
		DenyTurnSpeak = 0,
		DisableMicrophoneIfNoSpeak = 1,
		DisableCameraIfNoSpeak = 2,
        DontAskTurnDevices = 3,
		AutoConnect = 4,
		DisableSpeakerChange = 5,
		RecordOnServer = 6,
		DenyTurnMicrophone = 7,
		DenyTurnCamera = 8,
        DenyRecord = 9,
		DenySelfConnectMembers = 10,
		EnableCameraOnConnect = 11,
		EnableMicrophoneOnConnect = 12,

		Deactivated = 20
	};
}
