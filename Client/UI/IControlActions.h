/**
 * IControlActions.h - Contains control actions interface
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2015
 */

#pragma once

#include <cstdint>
#include <string>

namespace Client
{

class IControlActions
{
public:
    virtual void ActionCall() = 0;
    virtual void ActionConference() = 0;
    virtual void ActionHangup() = 0;

    virtual void ActionDevices() = 0;
    virtual void ActionTurnCamera(bool my, int64_t actorId = 0, std::string_view actorName = "") = 0;
    
    virtual void ActionTurnMicrophone(bool my, int64_t actorId = 0, std::string_view actorName = "") = 0;
    virtual void ActionVolumeMicrophone() = 0;
    
    virtual void ActionTurnLoudspeaker() = 0;
    virtual void ActionVolumeLoudspeaker() = 0;
		
    virtual void ActionTurnRendererGrid() = 0;
    
    virtual void ActionTurnDemonstration(bool my, int64_t actorId = 0, std::string_view actorName = "") = 0;
    virtual void ActionTurnRemoteControl(bool my, int64_t actorId = 0, std::string_view actorName = "", bool enable = true) = 0;

    virtual void ActionHand(bool my) = 0;
		
	virtual void ActionTurnListPanel() = 0;
	virtual void ActionTurnContentPanel() = 0;

    virtual void ActionTurnRecord() = 0;
		
	virtual void ActionFullScreen() = 0;

	virtual void ActionMenu() = 0;

protected:
	~IControlActions() {}
};

}
