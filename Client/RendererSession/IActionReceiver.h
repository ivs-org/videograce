/**
 * IActionReceiver.h - Contains interface of mouse and keyboard actions
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#pragma once

#include <cstdint>

namespace RendererSession
{
	class IActionReceiver
	{
	public:
		virtual void MouseMove(int32_t x, int32_t y) = 0;
		virtual void MouseLeftDown(int32_t x, int32_t y) = 0;
		virtual void MouseLeftUp(int32_t x, int32_t y) = 0;
		virtual void MouseCenterDown(int32_t x, int32_t y) = 0;
		virtual void MouseCenterUp(int32_t x, int32_t y) = 0;
		virtual void MouseRightDown(int32_t x, int32_t y) = 0;
		virtual void MouseRightUp(int32_t x, int32_t y) = 0;
		virtual void MouseRightDblClick(int32_t x, int32_t y) = 0;
		virtual void MouseLeftDblClick(int32_t x, int32_t y) = 0;
		virtual void MouseWheel(int32_t delta) = 0;

		virtual void KeyDown(int32_t virtkey) = 0;
		virtual void KeyUp(int32_t virtkey) = 0;
		
	protected:
		~IActionReceiver() {}
	};
}
