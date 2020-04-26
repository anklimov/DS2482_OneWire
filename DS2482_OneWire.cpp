#include <Arduino.h>
#include "DS2482_OneWire.h"
#include <Wire.h>

// Constructor with no parameters for compatability with OneWire lib
OneWire::OneWire()
{
	// Address is determined by two pins on the DS2482 AD1/AD0
	// Pass 0b00, 0b01, 0b10 or 0b11
	mAddress = 0x18;
	mError = 0;
	APU=0;
	_idle=0;
	Wire.begin();
}

OneWire::OneWire(uint8_t address)
{
	// Address is determined by two pins on the DS2482 AD1/AD0
	// Pass 0b00, 0b01, 0b10 or 0b11
	mAddress = 0x18 | address;
	mError = 0;
	APU=0;
	_idle=0;
	Wire.begin();
}

void OneWire::idle(void (*idle)())
{
  _idle = idle;
}

uint8_t OneWire::getAddress()
{
	return mAddress;
}

uint8_t OneWire::getError()
{
	return mError;
}

// Helper functions to make dealing with I2C side easier
void OneWire::begin()
{
	Wire.beginTransmission(mAddress);
}

uint8_t OneWire::end()
{
	return Wire.endTransmission();
}

void OneWire::writeByte(uint8_t data)
{
	Wire.write(data); 
}

uint8_t OneWire::readByte()
{
	Wire.requestFrom(mAddress,1u);
	return Wire.read();
}

// Simply starts and ends an Wire transmission
// If no devices are present, this returns false
uint8_t OneWire::checkPresence()
{
	#if defined(__SAM3X8E__)
	if (waitOnBusy() & DS2482_STATUS_BUSY) return false;  //device access error
	return true;
	#else 
	begin();
	return !end() ? true : false;
	#endif
}

// Performs a global reset of device state machine logic. Terminates any ongoing 1-Wire communication.
void OneWire::deviceReset()
{
	begin();
	write(DS2482_COMMAND_RESET);
	end();
}

// Sets the read pointer to the specified register. Overwrites the read pointer position of any 1-Wire communication command in progress.
void OneWire::setReadPointer(uint8_t readPointer)
{
	begin();
	writeByte(DS2482_COMMAND_SRP);
	writeByte(readPointer);
	end();
}

// Read the status register
uint8_t OneWire::readStatus()
{
	setReadPointer(DS2482_POINTER_STATUS);
	return readByte();
}

// Read the data register
uint8_t OneWire::readData()
{
	setReadPointer(DS2482_POINTER_DATA);
	return readByte();
}

// Read the config register
uint8_t OneWire::readConfig()
{ int conf;
	setReadPointer(DS2482_POINTER_CONFIG);
	conf=readByte();
///	Serial.print("Conf: ");Serial.println(conf,BIN);
	return conf;
}

void OneWire::setStrongPullup()
{
	writeConfig(readConfig() | DS2482_CONFIG_SPU);
}

void OneWire::setActivePullup()
{
APU=1;
}

void OneWire::clearStrongPullup()
{
	writeConfig(readConfig() & !DS2482_CONFIG_SPU);
}

// Churn until the busy bit in the status register is clear
uint8_t OneWire::waitOnBusy()
{
	uint8_t status;
        unsigned long ms=millis()+1000;
         
	while (millis()<ms)
	{
		status = readStatus();
		if (!(status & DS2482_STATUS_BUSY))
			return status;
		   if (_idle)
    			{
    			_idle();
    			}	
		//Serial.print(".");
		delayMicroseconds(20);
	}

	// if we have reached this point and we are still busy, there is an error
		mError = DS2482_ERROR_TIMEOUT;
		Serial.println("1w Timeout");
	// Return the status so we don't need to explicitly do it again
	return status;
}

// Write to the config register
void OneWire::writeConfig(uint8_t config)
{
	waitOnBusy();
	begin();
	writeByte(DS2482_COMMAND_WRITECONFIG);
	// Write the 4 bits and the complement 4 bits
	writeByte(config | (~config)<<4);
	end();
	
	// This should return the config bits without the complement
	if (readByte() != config)
		mError = DS2482_ERROR_CONFIG;
}

// Generates a 1-Wire reset/presence-detect cycle (Figure 4) at the 1-Wire line. The state
// of the 1-Wire line is sampled at tSI and tMSP and the result is reported to the host 
// processor through the Status Register, bits PPD and SD.
uint8_t OneWire::wireReset()
{
	if (waitOnBusy() & DS2482_STATUS_BUSY) return false;  //device access error
	
	// Datasheet warns that reset with SPU set can exceed max ratings
	clearStrongPullup();

	waitOnBusy();

	begin();
	writeByte(DS2482_COMMAND_RESETWIRE);
	end();

	uint8_t status = waitOnBusy();

	if (status & DS2482_STATUS_SD)
	{
		mError = DS2482_ERROR_SHORT;
	}
	
	if (APU) writeConfig(readConfig() | DS2482_CONFIG_APU);// | DS2482_CONFIG_SPU);
//        Serial.print("Reseted: :");readConfig();

	return (status & DS2482_STATUS_PPD) ? true : false;
}

// Writes a single data byte to the 1-Wire line.
void OneWire::wireWriteByte(uint8_t data, uint8_t power)
{
	waitOnBusy();
	if (power)
		setStrongPullup();
	begin();
	writeByte(DS2482_COMMAND_WRITEBYTE);
	writeByte(data);
	end();
}

// Generates eight read-data time slots on the 1-Wire line and stores result in the Read Data Register.
uint8_t OneWire::wireReadByte()
{
	waitOnBusy();
	begin();
	writeByte(DS2482_COMMAND_READBYTE);
	end();
	waitOnBusy();
	return readData();
}

// Generates a single 1-Wire time slot with a bit value “V” as specified by the bit byte at the 1-Wire line
// (see Table 2). A V value of 0b generates a write-zero time slot (Figure 5); a V value of 1b generates a 
// write-one time slot, which also functions as a read-data time slot (Figure 6). In either case, the logic
// level at the 1-Wire line is tested at tMSR and SBR is updated.
void OneWire::wireWriteBit(uint8_t data, uint8_t power)
{
	waitOnBusy();
	if (power)
		setStrongPullup();
	begin();
	writeByte(DS2482_COMMAND_SINGLEBIT);
	writeByte(data ? 0x80 : 0x00);
	end();
}

// As wireWriteBit
uint8_t OneWire::wireReadBit()
{
	wireWriteBit(1);
	uint8_t status = waitOnBusy();
	return status & DS2482_STATUS_SBR ? 1 : 0;
}

// 1-Wire skip
void OneWire::wireSkip()
{
	wireWriteByte(WIRE_COMMAND_SKIP);
}

void OneWire::wireSelect(const uint8_t rom[8])
{
	wireWriteByte(WIRE_COMMAND_SELECT);
	for (int i=0;i<8;i++)
		wireWriteByte(rom[i]);
}



///Alternative
uint8_t OneWire::wireReadStatus(bool setPtr)
{
	if (setPtr)
		setReadPointer(DS2482_POINTER_STATUS);
	return readByte();
}

///Alternative
uint8_t OneWire::busyWait(bool setReadPtr)
{
	uint8_t status;
	int loopCount = 1000;
	while((status = wireReadStatus(setReadPtr)) & DS2482_STATUS_BUSY)
	{
		if (--loopCount <= 0)
		{
	///		mTimeout = 1;
			break;
		}
		delayMicroseconds(20);
	}
	return status;
}



//define ALTSEARCH
#ifdef ALTSEARCH
void OneWire::wireResetSearch()
{
	searchExhausted = 0;
	searchLastDisrepancy = 0;
	
	for(uint8_t i = 0; i<8; i++) 
		searchAddress[i] = 0;
}

int8_t OneWire::wireSearch(uint8_t *newAddr)
{
	uint8_t i;
	uint8_t direction;
	uint8_t last_zero=0;
	
	if (searchExhausted) 
		return 0;
	
	if (!wireReset()) 
		return 0;

	busyWait(true);
	wireWriteByte(0xf0);
	
	for(i=1;i<65;i++) 
	{
		int romByte = (i-1)>>3;
		int romBit = 1<<((i-1)&7);
		
		if (i < searchLastDisrepancy)
			direction = searchAddress[romByte] & romBit;
		else
			direction = i == searchLastDisrepancy;
		
		busyWait();
		begin();
		Wire.write(0x78); 
		Wire.write(direction ? 0x80 : 0);
		end();
		uint8_t status = busyWait();
		
		uint8_t id = status & DS2482_STATUS_SBR;
		uint8_t comp_id = status & DS2482_STATUS_TSB;
		direction = status & DS2482_STATUS_DIR;
		
		if (id && comp_id)
			return 0;
		else
		{ 
			if (!id && !comp_id && !direction)
				last_zero = i;
		}
		
		if (direction)
			searchAddress[romByte] |= romBit;
		else
			searchAddress[romByte] &= (uint8_t)~romBit;
	}

	searchLastDisrepancy = last_zero;

	if (last_zero == 0) 
		searchExhausted = 1;
	
	for (i=0;i<8;i++) 
		newAddr[i] = searchAddress[i];
	
	return 1;  
}
#else



//  1-Wire reset seatch algorithm
void OneWire::wireResetSearch()
{
	searchLastDiscrepancy = -1;
	searchLastDeviceFlag = 0;

	for (int i = 0; i < 8; i++)
	{
		searchAddress[i] = 0;
	}

}

// Perform a search of the 1-Wire bus
int8_t OneWire::wireSearch(uint8_t *address)
{
	uint8_t direction;
	int8_t last_zero=-1; ///

	if (searchLastDeviceFlag)
		{
		//Serial.println("last device");
		return 0;}

	if (!wireReset())
	
	{Serial.println("Reset error");return -1;}
//Serial.print("S reseted: :");readConfig();		

	waitOnBusy();
        //setStrongPullup();///
	wireWriteByte(WIRE_COMMAND_SEARCH);
//Serial.print("comser: :");readConfig();
	for(uint8_t i=0;i<64;i++)
	{
		int searchByte = i / 8; 
		int searchBit = 1 << i % 8;

		if (i < searchLastDiscrepancy)
			direction = searchAddress[searchByte] & searchBit;
		else
			direction = i == searchLastDiscrepancy;

		waitOnBusy();
		begin();
		writeByte(DS2482_COMMAND_TRIPLET);
		writeByte(direction ? 0x80 : 0x00);
		end();

		uint8_t status = waitOnBusy();
//Serial.print("triplet: :");readConfig();
		uint8_t id = status & DS2482_STATUS_SBR;
		uint8_t comp_id = status & DS2482_STATUS_TSB;
		direction = status & DS2482_STATUS_DIR;

		if (id && comp_id)
		{
			
		{Serial.print(status,BIN);Serial.println(" comp_id!");return -2;}
		}
		
	///	if (1){
			if (!id && !comp_id && !direction)
			{
				last_zero = i;
			}
	///	}

		if (direction)
			searchAddress[searchByte] |= searchBit;
		else
			searchAddress[searchByte] &= ~searchBit;

	}

	searchLastDiscrepancy = last_zero;

	if (last_zero==-1)//
		searchLastDeviceFlag = 1;

	for (uint8_t i=0; i<8; i++)
		address[i] = searchAddress[i];

//Serial.print("SF: :");readConfig();

	return 1;
}
#endif

#if ONEWIRE_CRC8_TABLE
// This table comes from Dallas sample code where it is freely reusable,
// though Copyright (C) 2000 Dallas Semiconductor Corporation
static const uint8_t PROGMEM dscrc_table[] = {
      0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
    157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
     35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
    190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
     70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
    219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
    101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
    248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
    140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
     17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
    175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
     50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
    202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
     87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
    233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
    116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

//
// Compute a Dallas Semiconductor 8 bit CRC. These show up in the ROM
// and the registers.  (note: this might better be done without to
// table, it would probably be smaller and certainly fast enough
// compared to all those delayMicrosecond() calls.  But I got
// confused, so I use this table from the examples.)
//

uint16_t OneWire::crc16(const uint8_t* input, uint16_t len, uint16_t crc)
{
    static const uint8_t oddparity[16] =
        { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };

    for (uint16_t i = 0 ; i < len ; i++) {
      // Even though we're just copying a byte from the input,
      // we'll be doing 16-bit computation with it.
      uint16_t cdata = input[i];
      cdata = (cdata ^ crc) & 0xff;
      crc >>= 8;

      if (oddparity[cdata & 0x0F] ^ oddparity[cdata >> 4])
          crc ^= 0xC001;

      cdata <<= 6;
      crc ^= cdata;
      cdata <<= 1;
      crc ^= cdata;
    }
    return crc;
}

bool OneWire::check_crc16(const uint8_t* input, uint16_t len, const uint8_t* inverted_crc, uint16_t crc)
{
    crc = ~crc16(input, len, crc);
    return (crc & 0xFF) == inverted_crc[0] && (crc >> 8) == inverted_crc[1];
}


uint8_t OneWire::crc8(const uint8_t *addr, uint8_t len)
{
	uint8_t crc = 0;

	while (len--) {
		crc = pgm_read_byte(dscrc_table + (crc ^ *addr++));
	}
	return crc;
}
#else
//
// Compute a Dallas Semiconductor 8 bit CRC directly.
// this is much slower, but much smaller, than the lookup table.
//
uint8_t OneWire::crc8(const uint8_t *addr, uint8_t len)
{
	uint8_t crc = 0;
	
	while (len--) {
		uint8_t inbyte = *addr++;
		for (uint8_t i = 8; i; i--) {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) crc ^= 0x8C;
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
void OneWire::reset_search()
{
	wireResetSearch();
}

uint8_t OneWire::search(uint8_t *newAddr)
{       int res;
	res=  wireSearch(newAddr);
	if (res>0) return res;
	    else return 0;
}

// Perform a 1-Wire reset cycle. Returns 1 if a device responds
// with a presence pulse.  Returns 0 if there is no device or the
// bus is shorted or otherwise held low for more than 250uS
uint8_t OneWire::reset(void)
{
	return wireReset();
}

// Issue a 1-Wire rom select command, you do the reset first.
void OneWire::select(const uint8_t rom[8])
{
	wireSelect(rom);
}

// Issue a 1-Wire rom skip command, to address all on bus.
void OneWire::skip(void)
{
	wireSkip();
}

// Write a byte. 
// Ignore the power bit
void OneWire::write(uint8_t v, uint8_t power)
{
	wireWriteByte(v, power);	
}

// Read a byte.
uint8_t OneWire::read(void)
{
	return wireReadByte();
}

// Read a bit.
uint8_t OneWire::read_bit(void)
{
	return wireReadBit();
}

// Write a bit.
void OneWire::write_bit(uint8_t v)
{
	wireWriteBit(v);
}

void OneWire::write_bytes(const uint8_t *buf, uint16_t count, bool power /* = 0 */) {
  for (uint16_t i = 0 ; i < count ; i++)
    wireWriteByte(buf[i],power);
//  if (!power) {
//    noInterrupts();
//    DIRECT_MODE_INPUT(baseReg, bitmask);
//    DIRECT_WRITE_LOW(baseReg, bitmask);
//    interrupts();
//  }
}

//
// Read a byte
//

void OneWire::read_bytes(uint8_t *buf, uint16_t count) {
  for (uint16_t i = 0 ; i < count ; i++)
    buf[i] = read();
}


// ****************************************
// End mirrored functions
// ****************************************







