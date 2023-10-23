/**
 * BitHelpers.h - Contains bit helpers macros
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#define SetBit(reg, bit)         reg |= (1<<bit)            
#define ClearBit(reg, bit)       reg &= (~(1<<bit))
#define InvBit(reg, bit)         reg ^= (1<<bit)
#define BitIsSet(reg, bit)       ((reg & (1<<bit)) != 0)
#define BitIsClear(reg, bit)     ((reg & (1<<bit)) == 0)
