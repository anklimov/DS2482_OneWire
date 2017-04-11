/*
 * OneWireExtraHelpers.h
 * - Part of the "DS2482_OneWire" Arduino library
 *
 * Copyright (C) 2017 Gerad Munsch <gmunsch@unforgivendevelopment.com>
 * See README.md for additional author/copyright info.
 *
 */

#ifndef __DS2482OW__OneWireExtraHelpers_h__
#define __DS2482OW__OneWireExtraHelpers_h__

#include <DallasTemperature.h>


class OneWireHelpersClass {
public:
	void printDeviceAddress(uint8_t devAddr[8]);
	void printDeviceAddress(DeviceAddress devAddr);
}


#endif	/* __DS2482OW__OneWireExtraHelpers_h__ */
