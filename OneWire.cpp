/**********************************************************************************************************************
 * OneWire.cpp                                                                                                        *
 * - Part of the "DS2482_OneWire" Arduino library                                                                     *
 *                                                                                                                    *
 * Portions Copyright (C) 2017 Gerad Munsch <gmunsch@unforgivendevelopment.com>                                       *
 * See README.md for additional author/copyright info.                                                                *
 *                                                                                                                    *
 **********************************************************************************************************************/

/*----------------------------------------------------------------------------*/
/* INCLUDES                                                                   */
/*----------------------------------------------------------------------------*/
#include "OneWire.h"
#include <Wire.h>

/**
 * Constructor with no parameters for compatability with OneWire lib
 */
OneWire::OneWire() {
	/* Address is determined by two pins on the DS2482 AD1/AD0 */
	/* Pass 0b00, 0b01, 0b10 or 0b11 */
	mAddress = 0x18;
	mError = 0;
	Wire.begin();
}


OneWire::OneWire(uint8_t address) {
	/* Address is determined by two pins on the DS2482 AD1/AD0 */
	/* Pass 0b00, 0b01, 0b10 or 0b11 */
	mAddress = 0x18 | address;
	mError = 0;
	Wire.begin();
}


uint8_t OneWire::getAddress() {
	return mAddress;
}

uint8_t OneWire::getError() {
	return mError;
}

/**
 * Helper functions to make dealing with I2C side easier
 */
void OneWire::begin() {
	Wire.beginTransmission(mAddress);
}


uint8_t OneWire::end() {
	return Wire.endTransmission();
}


void OneWire::writeByte(uint8_t data) {
	Wire.write(data);
}


uint8_t OneWire::readByte() {
	Wire.requestFrom(mAddress, 1u);
	return Wire.read();
}

/**
 * Simply starts and ends an Wire transmission
 * If no devices are present, this returns false
 */
uint8_t OneWire::checkPresence() {
	begin();
	return !end() ? true : false;
}

/**
 * Performs a global reset of device state machine logic. This action
 * terminates any ongoing 1-Wire communication.
 */
void OneWire::deviceReset() {
	begin();
	write(DS2482_COMMAND_RESET);
	end();
}


/**
 * Sets the read pointer to the specified register. This action overwrites the
 * read pointer position of any 1-Wire communication command that is currently
 * in progress.
 */
void OneWire::setReadPointer(uint8_t readPointer) {
	begin();
	writeByte(DS2482_COMMAND_SRP);
	writeByte(readPointer);
	end();
}


/**
 * Read the status register
 */
uint8_t OneWire::readStatus() {
	setReadPointer(DS2482_POINTER_STATUS);
	return readByte();
}


/**
 * Read the data register
 */
uint8_t OneWire::readData() {
	setReadPointer(DS2482_POINTER_DATA);
	return readByte();
}


/**
 * Read the config register
 */
uint8_t OneWire::readConfig() {
	setReadPointer(DS2482_POINTER_CONFIG);
	return readByte();
}


/**
 * Set the strong pullup bit. The strong pullup bit is used to activate the strong pullup function prior to a '1-Wire
 * Write Byte' or a '1-Wire Single Bit' command. Strong pullup is commonly used with 1-Wire EEPROM devices when copying
 * scratchpad data to the main memory, when performing an SHA-1 computation, and/or with parasitically-powered devices,
 * such as temperature sensors or A/D converters. See the respective device data sheets for information as to the timing
 * and use of the SPU (strong pullup) bit throughout the communications protocol.
 *
 * The SPU bit must be set immediately prior to issuing the command that puts the 1-Wire device into the state where it
 * needs the extra power.
 *
 * IMPORTANT: The SPU bit also affects the 1-Wire Reset command. If enabled, it can cause incorrect reading of the
 *            presence pulse, and may cause a violation of the device's absolute maximum rating.
 *
 * Many details about the use of the SPU bit are located on Page 7 of the DS2482-100 datasheet.
 *
 * @brief Activates the strong pullup function for the following transaction.
 */
void OneWire::setStrongPullup() {
	writeConfig(readConfig() | DS2482_CONFIG_SPU);
}


/**
 * Manually clears the SPU (strong pullup) bit manually, in the event that the other triggers have not been fired.
 *
 * @brief Manually clear the strong pullup bit in the DS2482 config register.
 */
void OneWire::clearStrongPullup() {
	writeConfig(readConfig() & !DS2482_CONFIG_SPU);
}


/**
 * Wait for a limited period of time for the busy bit in the status register to clear. If the timeout is reached, it is
 * likely an error has occurred.
 *
 * @brief Wait for a brief period of time to allow the 1-Wire bus to become free.
 */
uint8_t OneWire::waitOnBusy() {
	uint8_t status;

	/* Check register status every 20 microseconds */
	for (int i = 1000; i > 0; i--) {
		status = readStatus();

		/* Break out of loop if the busy status bit clears up */
		if (!(status & DS2482_STATUS_BUSY)) {
			break;
		}

		/* Wait 20 microseconds before checking the status again... */
		delayMicroseconds(20);
	}

	/* It is likely an error has occurred if the busy status bit is still set */
	if (status & DS2482_STATUS_BUSY) {
		mError = DS2482_ERROR_TIMEOUT;
	}

	/* Return the status so we don't need to explicitly do it again */
	return status;
}


/**
 * Write data to the config register. The function accepts an unsigned byte value, with the lower 4 bits containing the
 * desired config register values. The necessary function of setting the upper 4 bits to the one's complement of the
 * lower 4 bits before writing the data to the register is handled within this function. Confirmation that the write was
 * successful is done by reading back the config register, and comparing it against the original byte value, as any read
 * actions from the config register have the upper 4 bits set to 0000b; as such, the values should match.
 *
 * @brief Write data to the config register.
 *
 * @param[in]	config	An unsigned byte value, with the lower 4 bytes containing the desired config register values.
 */
void OneWire::writeConfig(uint8_t config) {
	waitOnBusy();
	begin();
	writeByte(DS2482_COMMAND_WRITECONFIG);

	/*
	 * The config register expects its data in the following format:
	 * - Bytes 0-3: config data
	 * - Bytes 4-7: one's complement of bytes 0-3
	 */
	writeByte(config | (~config)<<4);
	end();

	/*
	 * Readback of the config register will return data in the following format:
	 * - Bytes 0-3: config data
	 * - Bytes 4-7: 0000b
	 */
	if (readByte() != config) {
		mError = DS2482_ERROR_CONFIG;
	}
}


/**
 * Generates a 1-Wire reset/presence-detect cycle on the 1-Wire line. Note that
 * a diagram of this process can be found at figure 4 of the official datasheet
 * for the DS2482-100.
 *
 * The state of the 1-Wire line is sampled at tSI and tMSP, and the result is
 * reported to the host processor through the status register bits 'PPD' & 'SD'.
 */
uint8_t OneWire::wireReset() {
	waitOnBusy();

	/*
	 * Ensure that the SPU (strong pullup) bit is cleared before execution, as
	 * its use may result in 'PPD' containing invalid data, and/or device(s)
	 * violating their absolute maximum ratings.
	 * (Noted on page 10 of the DS2482-100 datasheet)
	 */
	clearStrongPullup();

	waitOnBusy();

	begin();
	writeByte(DS2482_COMMAND_RESETWIRE);
	end();

	uint8_t status = waitOnBusy();

	if (status & DS2482_STATUS_SD) {
		mError = DS2482_ERROR_SHORT;
	}

	return (status & DS2482_STATUS_PPD) ? true : false;
}


/**
 * Write a single data byte to the 1-Wire line. Optionally, the strong pullup bit may be activated, causing the strong
 * pullup to take effect for this transaction.
 *
 * @brief Write one byte of data to the 1-Wire bus.
 *
 * @param[in]	data	An unsigned byte value containing the byte to be written to the 1-Wire line.
 * @param[in]	power	An optional, unsigned byte value, activates the SPU function when containing a value >= 1.
 */
void OneWire::wireWriteByte(uint8_t data, uint8_t power) {
	waitOnBusy();

	if (power) {
		setStrongPullup();
	}

	begin();
	writeByte(DS2482_COMMAND_WRITEBYTE);
	writeByte(data);
	end();
}

/**
 * Generates eight read-data time slots on the 1-Wire line and stores result in the Read Data Register.
 */
uint8_t OneWire::wireReadByte() {
	waitOnBusy();

	begin();
	writeByte(DS2482_COMMAND_READBYTE);
	end();

	waitOnBusy();

	return readData();
}

/**
 * Generates a single 1-Wire Time Slot with a bit value “V”, as specified by the bit byte, at the 1-Wire line (Table 2).
 * A "V" value of 0b generates a Write-Zero Time Slot (Figure 5); a "V" value of 1b generates a Write-One Time Slot,
 * which also functions as a Read-Data Time Slot (Figure 6). In any case, the logic level at the 1-Wire line is queried
 * at 'tMSR', and 'SBR' is updated.
 *
 * NOTE: See the DS2482-100 datasheet for the tables and figures referenced above.
 *
 * @brief Generates a single 1-Wire timeslot.
 *
 * @param[in]	data	An unsigned byte value, with the MSB containing the bit which determines the action to be taken.
 * @param[in]	power	An optional, unsigned byte value, activates the SPU function when containing a value >= 1.
 */
void OneWire::wireWriteBit(uint8_t data, uint8_t power) {
	waitOnBusy();
	if (power)
		setStrongPullup();
	begin();
	writeByte(DS2482_COMMAND_SINGLEBIT);
	writeByte(data ? 0x80 : 0x00);
	end();
}

// As wireWriteBit
uint8_t OneWire::wireReadBit() {
	wireWriteBit(1);
	uint8_t status = waitOnBusy();
	return status & DS2482_STATUS_SBR ? 1 : 0;
}

// 1-Wire skip
void OneWire::wireSkip() {
	wireWriteByte(WIRE_COMMAND_SKIP);
}

void OneWire::wireSelect(const uint8_t rom[8]) {
	wireWriteByte(WIRE_COMMAND_SELECT);
	for (int i=0;i<8;i++)
		wireWriteByte(rom[i]);
}

//  1-Wire reset seatch algorithm
void OneWire::wireResetSearch() {
	searchLastDiscrepancy = 0;
	searchLastDeviceFlag = 0;

	for (int i = 0; i < 8; i++) 	{
		searchAddress[i] = 0;
	}

}

// Perform a search of the 1-Wire bus
uint8_t OneWire::wireSearch(uint8_t *address) {
	uint8_t direction;
	uint8_t last_zero=0;

	if (searchLastDeviceFlag) {
		return 0;
	}

	if (!wireReset()) {
		return 0;
	}

	waitOnBusy();

	wireWriteByte(WIRE_COMMAND_SEARCH);

	for (uint8_t i = 0; i < 64; i++) {
		int searchByte = i / 8;
		int searchBit = 1 << i % 8;

		if (i < searchLastDiscrepancy) {
			direction = searchAddress[searchByte] & searchBit;
		} else {
			direction = i == searchLastDiscrepancy;
		}

		waitOnBusy();
		begin();
		writeByte(DS2482_COMMAND_TRIPLET);
		writeByte(direction ? 0x80 : 0x00);
		end();

		uint8_t status = waitOnBusy();

		uint8_t id = status & DS2482_STATUS_SBR;
		uint8_t comp_id = status & DS2482_STATUS_TSB;
		direction = status & DS2482_STATUS_DIR;

		if (id && comp_id) {
			return 0;
		} else {
			if (!id && !comp_id && !direction) {
				last_zero = i;
			}
		}

		if (direction) {
			searchAddress[searchByte] |= searchBit;
		} else {
			searchAddress[searchByte] &= ~searchBit;
		}
	}

	searchLastDiscrepancy = last_zero;

	if (!last_zero) {
		searchLastDeviceFlag = 1;
	}

	for (uint8_t i = 0; i < 8; i++) {
		address[i] = searchAddress[i];
	}

	return 1;
}

#ifdef ONEWIRE_CRC8_TABLE
// This table comes from Dallas sample code where it is freely reusable,
// though Copyright (C) 2000 Dallas Semiconductor Corporation
static const uint8_t PROGMEM dscrc_table[] = {
	  0,  94, 188, 226,  97,  63, 221, 131, 194, 156, 126,  32, 163, 253,  31,  65, 157, 195,  33, 127, 252, 162,  64,
	 30,  95,   1, 227, 189,  62,  96, 130, 220,  35, 125, 159, 193,  66,  28, 254, 160, 225, 191,  93,   3, 128, 222,
	 60,  98, 190, 224,   2,  92, 223, 129,  99,  61, 124,  34, 192, 158,  29,  67, 161, 255,  70,  24, 250, 164,  39,
	121, 155, 197, 132, 218,  56, 102, 229, 187,  89,   7, 219, 133, 103,  57, 186, 228,   6,  88,  25,  71, 165, 251,
	120,  38, 196, 154, 101,  59, 217, 135,   4,  90, 184, 230, 167, 249,  27,  69, 198, 152, 122,  36, 248, 166,  68,
	 26, 153, 199,  37, 123,  58, 100, 134, 216,  91,   5, 231, 185, 140, 210,  48, 110, 237, 179,  81,  15,  78,  16,
	242, 172,  47, 113, 147, 205,  17,  79, 173, 243, 112,  46, 204, 146, 211, 141, 111,  49, 178, 236,  14,  80, 175,
	241,  19,  77, 206, 144, 114,  44, 109,  51, 209, 143,  12,  82, 176, 238,  50, 108, 142, 208,  83,  13, 239, 177,
	240, 174,  76,  18, 145, 207,  45, 115, 202, 148, 118,  40, 171, 245,  23,  73,   8,  86, 180, 234, 105,  55, 213,
	139,  87,   9, 235, 181,  54, 104, 138, 212, 149, 203,  41, 119, 244, 170,  72,  22, 233, 183,  85,  11, 136, 214,
	 52, 106,  43, 117, 151, 201,  74,  20, 246, 168, 116,  42, 200, 150,  21,  75, 169, 247, 182, 232,  10,  84, 215,
	137, 107,  53
};

//
// Compute a Dallas Semiconductor 8 bit CRC. These show up in the ROM
// and the registers.  (note: this might better be done without to
// table, it would probably be smaller and certainly fast enough
// compared to all those delayMicrosecond() calls.  But I got
// confused, so I use this table from the examples.)
//
uint8_t OneWire::crc8(const uint8_t *addr, uint8_t len) {
	uint8_t crc = 0;

	while (len--) {
		crc = pgm_read_byte(dscrc_table + (crc ^ *addr++));
	}

	return crc;
}

#else	/* ONEWIRE_CRC8_TABLE */

// Compute a Dallas Semiconductor 8 bit CRC directly.
// this is much slower, but much smaller, than the lookup table.
//
uint8_t OneWire::crc8(const uint8_t *addr, uint8_t len) {
	uint8_t crc = 0;

	while (len--) {
		uint8_t inbyte = *addr++;
		for (uint8_t i = 8; i; i--) {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;

			if (mix) {
				crc ^= 0x8C;
			}

			inbyte >>= 1;
		}
	}

	return crc;
}
#endif

// ****************************************
// These are here to mirror the functions in the original OneWire
// ****************************************

// This is a lazy way of getting compatibility with DallasTemperature
// Not all functions are implemented, only those used in DallasTemeperature
void OneWire::reset_search() {
	wireResetSearch();
}

uint8_t OneWire::search(uint8_t *newAddr) {
	return wireSearch(newAddr);
}

// Perform a 1-Wire reset cycle. Returns 1 if a device responds
// with a presence pulse.  Returns 0 if there is no device or the
// bus is shorted or otherwise held low for more than 250uS
uint8_t OneWire::reset(void) {
	return wireReset();
}

// Issue a 1-Wire rom select command, you do the reset first.
void OneWire::select(const uint8_t rom[8]) {
	wireSelect(rom);
}

// Issue a 1-Wire rom skip command, to address all on bus.
void OneWire::skip(void) {
	wireSkip();
}

// Write a byte.
// Ignore the power bit
void OneWire::write(uint8_t v, uint8_t power) {
	wireWriteByte(v, power);
}

// Read a byte.
uint8_t OneWire::read(void) {
	return wireReadByte();
}

// Read a bit.
uint8_t OneWire::read_bit(void) {
	return wireReadBit();
}

// Write a bit.
void OneWire::write_bit(uint8_t v) {
	wireWriteBit(v);
}

// ****************************************
// End mirrored functions
// ****************************************
