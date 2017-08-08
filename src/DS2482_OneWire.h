/*
 * \file DS2482_OneWire.h
 * The only purpose of this file is to \c #include the \c OneWire.h file, which contains the actual implementation of
 * the 1-Wire driver.\n
 * The implementation is designed in a manner which is intended to be compatible with the "standard" Arduino \b OneWire
 * library. In the interest of avoiding library conflicts, this file should be included in the Arduino sketch. Ideally,
 * any project which was written utilizing the "standard" \b OneWire library can be made to utilize the \b DS2482 as the
 * 1-Wire controller by simply changing the
 * \code{.cpp}
 * #include <OneWire.h>
 * \endcode
 * line to
 * \code{.cpp}
 * #include <DS2482_OneWire.h>
 * \endcode
 * Aside from the change in the \c #include file, the goal is to achieve API compatibility, so that sketches can be used
 * with either zero, or minimal other changes to the sketch.
 *
 * \author		Gerad Munsch <gmunsch@unforgivendevelopment.com>
 * \date		2017
 * \version		1.0.1
 * \copyright	See README.md for additional author/copyright info.
 *
 */

#ifndef _DS2482OW__DS2482_ONEWIRE_H__
#define _DS2482OW__DS2482_ONEWIRE_H__


#include "OneWire.h"


#endif	/* _DS2482OW__DS2482_ONEWIRE_H__ */
