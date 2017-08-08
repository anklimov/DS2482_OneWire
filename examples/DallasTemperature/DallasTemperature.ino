/**
 * DallasTemperature.ino
 * Example Sketch for the DS2482_OneWire library
 *
 */

/* ---[ INCLUDES ]--- */
#include <Wire.h>
#include <DS2482_OneWire.h>

/* ---[ DEFINITIONS ]--- */
#define UART_BAUD_RATE 115200
#define SEARCH_DELAY 10000		/* Time in ms to wait after each full search cycle */

/* ---[ GLOBAL OBJECTS & VARIABLES ]--- */
OneWire oneWire;

/* ---[ FUNCTION PROTOTYPES ]--- */
void printAddress(uint8_t deviceAddress[8]);


/**
 * Standard Arduino "setup()" function
 * @brief Arduino setup function
 */
void setup() {
	Serial.begin(UART_BAUD_RATE);
}

/**
 * Standard Arduino "loop()" function
 * @brief Arduino loop function
 */
void loop() {

	Serial.println("Checking for I2C devices...:");

	if (oneWire.checkPresence()) {
		Serial.println("DS2482-100 present");

		oneWire.deviceReset();
		Serial.println("\tChecking for 1-Wire devices...");

		if (oneWire.wireReset()) {
			Serial.println("\tDevices present on 1-Wire bus");

			/* create an 64-bit (8x uint8_t/"byte") array to hold discovered address(es) */
			uint8_t currAddress[8];

			Serial.println("\t\tSearching 1-Wire bus...");

			while (oneWire.wireSearch(currAddress)) {
				Serial.print("\t\t\tFound device: ");
				printAddress(currAddress);
				Serial.println();
			}

			oneWire.wireResetSearch();

		} else {
			Serial.println("No devices on 1-Wire bus");
		}
	 } else {
		 Serial.println("No DS2482-100 present");
	 }

	delay(SEARCH_DELAY);	/* Wait for another round of searching */
}


/**
 * Print the OneWire device's 64-bit address to the serial line
 *
 * @brief Print the OneWire device address to serial
 *
 * @param[in]	deviceAddress	An array of 8 unsigned integer (bytes), representing the OneWire device's 64-bit address
 */
void printAddress(uint8_t deviceAddress[8]) {
	/* Encapsulate the address in curly brackets */
	Serial.print("{ ");

	/* iterate over the 8 octets belonging to the device address, printing each in hexadecimal form */
	for (uint8_t i = 0; i < 8; i++) {

		/* Prefix each octet with the "0x" radix */
		Serial.print("0x");

		/* Octets < 16 get zero-padded, so the output remains consistent as '0x00' ("byte-style") formatted output */
		if (deviceAddress[i] < 16) {
			Serial.print("0");
		}

		/* Print the octet as a hexidecimal value */
		Serial.print(deviceAddress[i], HEX);

		/* Place a comma and a space between octets */
		if (i < 7) {
			Serial.print(", ");
		}
	}

	Serial.print(" }");
}
