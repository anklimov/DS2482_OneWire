/*
 * \file DS2482_OneWire.h	The only purpose of this file is to #include the OneWire.h file, which contains the actual
 *							implementation of the 1-Wire driver. The implementation is designed in a manner which is
 *							intended to be compatible with the "standard" Arduino OneWire library. In the interest of
 *							avoiding library conflicts, this file should be included in the Arduino sketch. Ideally, any
 *							project which was written utilizing the "standard" OneWire library can be made to utilize
 *							the DS2482 as the OneWire controller by simply changing the #include <OneWire.h> line to
 *							#include <DS2482_OneWire.h>, with either zero, or minimal other changes to the sketch.
 *
 * \author	Gerad Munsch <gmunsch@unforgivendevelopment.com>
 * \date	2017
 *
 * See README.md for additional author/copyright info.
 *
 */

#ifndef _DS2482OW__DS2482_ONEWIRE_H__
#define _DS2482OW__DS2482_ONEWIRE_H__


#include "src/OneWire.h"


#endif	/* _DS2482OW__DS2482_ONEWIRE_H__ */
