/*
 * OneWire.h
 * - Part of the "DS2482_OneWire" Arduino library
 *
 * Portions Copyright (C) 2017 Gerad Munsch <gmunsch@unforgivendevelopment.com>
 * See README.md for additional author/copyright info.
 *
 */

#ifndef __DS2482OW__ONEWIRE_H__
#define __DS2482OW__ONEWIRE_H__


#include <inttypes.h>

#if defined(__AVR__)
#include <util/crc16.h>
#endif

#if ARDUINO >= 100
#include "Arduino.h"		// for delayMicroseconds, digitalPinToBitMask, etc
#else
#include "WProgram.h"		// for delayMicroseconds
#include "pins_arduino.h"	// for digitalPinToBitMask, etc
#endif


/**
 * The definition below ("USE_ONEWIRE_CRC8_TABLE") defines the behavior of CRC
 * calculation.
 *
 * VALUE:
 * * (defined)		- use pre-computed, table-based CRC calculation: fast calculations, but uses more flash memory
 * * (commented)	- compute the CRC values on-demand: smaller code size, but computation slower than the lookup table
 *
 */
#define ONEWIRE_CRC8_TABLE 				1


/**
 * Define device constants, such as commands, register values, etc.
 */
#define DS2482_COMMAND_RESET			0xF0	/* Device reset */
#define DS2482_COMMAND_SRP				0xE1	/* Set read pointer */

#define DS2482_POINTER_STATUS			0xF0
#define DS2482_POINTER_DATA				0xE1
#define DS2482_POINTER_CONFIG			0xC3

#define DS2482_COMMAND_WRITECONFIG		0xD2
#define DS2482_COMMAND_RESETWIRE		0xB4
#define DS2482_COMMAND_WRITEBYTE		0xA5
#define DS2482_COMMAND_READBYTE			0x96
#define DS2482_COMMAND_SINGLEBIT		0x87
#define DS2482_COMMAND_TRIPLET			0x78

#define WIRE_COMMAND_SKIP				0xCC
#define WIRE_COMMAND_SELECT				0x55
#define WIRE_COMMAND_SEARCH				0xF0


#define DS2482_STATUS_BUSY				(1<<0)
#define DS2482_STATUS_PPD				(1<<1)
#define DS2482_STATUS_SD				(1<<2)
#define DS2482_STATUS_LL				(1<<3)
#define DS2482_STATUS_RST 				(1<<4)
#define DS2482_STATUS_SBR				(1<<5)
#define DS2482_STATUS_TSB 				(1<<6)
#define DS2482_STATUS_DIR 				(1<<7)

#define DS2482_CONFIG_APU				(1<<0)
#define DS2482_CONFIG_SPU				(1<<2)
#define DS2482_CONFIG_1WS				(1<<3)

#define DS2482_ERROR_TIMEOUT			(1<<0)
#define DS2482_ERROR_SHORT				(1<<1)
#define DS2482_ERROR_CONFIG				(1<<2)


class OneWire {
public:
	OneWire();
	OneWire(uint8_t address);

	uint8_t getAddress();
	uint8_t getError();
	uint8_t checkPresence();

	void deviceReset();
	void setReadPointer(uint8_t readPointer);
	uint8_t readStatus();
	uint8_t readData();
	uint8_t waitOnBusy();
	uint8_t readConfig();
	void writeConfig(uint8_t config);
	void setStrongPullup();
	void clearStrongPullup();
	uint8_t wireReset();
	void wireWriteByte(uint8_t data, uint8_t power = 0);
	uint8_t wireReadByte();
	void wireWriteBit(uint8_t data, uint8_t power = 0);
	uint8_t wireReadBit();
	void wireSkip();
	void wireSelect(const uint8_t rom[8]);
	void wireResetSearch();
	uint8_t wireSearch(uint8_t *address);

	/**
	 * Emulation of the "standard" Arduino OneWire library.
	 * Homepage:	https://www.pjrc.com/teensy/td_libs_OneWire.html
	 * Source Code:	https://github.com/PaulStoffregen/OneWire
	 */
	uint8_t reset(void);
	void select(const uint8_t rom[8]);
	void skip(void);

	/**
	 * Write a byte. If 'power' is '1', then the wire is held high at the end for parasitically-powered devices. It
	 * is necessary to manually depower the line at some point, either by calling "depower()", or doing another read or
	 * write operation.
	 *
	 * @brief Write a byte to the OneWire bus.
	 *
	 * @param[in] v The byte to be written to the bus.
	 * @param[in] power Indicates that there are parasitically-powered devices on the bus, and to hold the line high at
	 *                  the end of the operation.
	 *
	 */
	void write(uint8_t v, uint8_t power = 0);

	void write_bytes(const uint8_t *buf, uint16_t count, bool power = 0);


	/**
	 * Read a single byte from the OneWire bus.
	 *
	 * @brief Read one byte from the OneWire bus.
	 *
	 * @return The byte that was read from the OneWire bus.
	 *
	 */
	uint8_t read(void);

	void read_bytes(uint8_t *buf, uint16_t count);


	/**
	 * Write a bit to the OneWire bus.
	 *
	 * @brief Write one bit to the bus.
	 *
	 * @param[in] v Contains the bit to be written to the bus.
	 *
	 */
	void write_bit(uint8_t v);


	/**
	 * Read a bit from the OneWire bus.
	 *
	 * @brief Read one bit from the bus.
	 *
	 * @return The bit that has been read from the bus.
	 *
	 */
	uint8_t read_bit(void);

	/**
	 * Remove power from the bus. Only necessary after a write() call with the 'power' parameter set to '1', or if the
	 * write_bit() function was called.
	 *
	 * @brief Remove power from the bus.
	 *
	 */
	void depower(void);

	void reset_search();

	void target_search(uint8_t family_code);

	uint8_t search(uint8_t *newAddr);

	static uint8_t crc8(const uint8_t *addr, uint8_t len);


private:
	void begin();
	uint8_t end();
	void writeByte(uint8_t);
	uint8_t readByte();

	uint8_t mAddress;
	uint8_t mError;

	uint8_t searchAddress[8];
	uint8_t searchLastDiscrepancy;
	uint8_t searchLastDeviceFlag;
};

#endif	/* __DS2482OW__ONEWIRE_H__ */
