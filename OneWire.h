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


/**
 * The definition below ("USE_ONEWIRE_CRC8_TABLE") defines the behavior of CRC
 * calculation.
 *
 * VALUE:
 * * 1	- use pre-computed, table-based CRC calculation -- performs fast calculations, but uses more flash memory
 * * 0	- compute the CRC values on-demand -- smaller code size, but performs computations slower than the lookup table
 *
 */
#define USE_ONEWIRE_CRC8_TABLE 			1


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


/**
 * Define 'ONEWIRE_CRC8_TABLE' based on the value of 'USE_ONEWIRE_CRC8_TABLE'
 */
#ifdef USE_ONEWIRE_CRC8_TABLE	/* Check that 'USE_ONEWIRE_CRC8_TABLE' is defined before checking its value */
	if (USE_ONEWIRE_CRC8_TABLE == 1 || USE_ONEWIRE_CRC8_TABLE == true) {
		#ifdef ONEWIRE_CRC8_TABLE	/* If 'ONEWIRE_CRC8_TABLE' is already defined, redefine it ensuring proper value */
			#undef ONEWIRE_CRC8_TABLE
			#define ONEWIRE_CRC8_TABLE 1
		#else						/* Otherwise, we can just define it with the proper value. */
			#define ONEWIRE_CRC8_TABLE 1
		#endif
	} else if (USE_ONEWIRE_CRC8_TABLE == 0 || USE_ONEWIRE_CRC8_TABLE == false) {
		#ifdef ONEWIRE_CRC8_TABLE
			#undef ONEWIRE_CRC8_TABLE
		#endif
	} else {	/* Handle cases where 'USE_ONEWIRE_CRC8_TABLE' contains an unexpected value */
		/*
		* If 'ONEWIRE_CRC8_TABLE' is defined, assume that the CRC table should be used, and
		* redefine it properly. Otherwise, leave things 'as-is'.
		*/
		#ifdef ONEWIRE_CRC8_TABLE
			#undef ONEWIRE_CRC8_TABLE
			#define ONEWIRE_CRC8_TABLE 1
		#endif
	}
#else	/* 'USE_ONEWIRE_CRC8_TABLE' -- if not defined, use the same logic as if it were to contain an invalid value */
	/*
	 * If 'ONEWIRE_CRC8_TABLE' is defined, assume that the CRC table should be used, and
	 * redefine it properly. Otherwise, leave things 'as-is'.
	 */
	#ifdef ONEWIRE_CRC8_TABLE
		#undef ONEWIRE_CRC8_TABLE
		#define ONEWIRE_CRC8_TABLE 1
	#endif
#endif


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
	void reset_search();
	uint8_t search(uint8_t *newAddr);
	static uint8_t crc8(const uint8_t *addr, uint8_t len);
	uint8_t reset(void);
	void select(const uint8_t rom[8]);
	void skip(void);
	void write(uint8_t v, uint8_t power = 0);
	uint8_t read(void);
	uint8_t read_bit(void);
	void write_bit(uint8_t v);

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
