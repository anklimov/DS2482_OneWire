# **DS2482_OneWire** Arduino Library #

A OneWire library using the **[DS2482][5]** I²C → 1-Wire bridge. Released under GPL.

This is designed to use Maxim _(Dallas)_ **[DS18B20][4]** temperature sensors connected to a **[DS2482][5]** I²C → 1-Wire bridge as part of a temperature monitoring system.


## Library History ##

_The code heavily borrows from two sources:_

- **[Paeaetech's DS2482 library][2]** - the bulk of the functionality if from here, slighty changed.
- **[OneWire library][3]** - the CRC code is from here, along with all method names so we can use new library in place of this.


## OneWire Devices ##

- **[DS18B20][4]** is a 1-Wire temperature sensor. These are commonly driven directly from an I/O pin using the OneWire library. This is fine for short and simple networks, but errors start to occur with large numbers (6+ devices) and networks longer than 5m or so.
- **[DS2482-100][5]** is a I²C → 1-Wire bridge, containly functionality that means it is better at driving long and complex networks. 


## Hardware ##

Breakouts for the Raspberry Pi _(which can also be connected to Arduino)_, are available from [Sheepwalk Electronics][6].









[2]:	<https://github.com/paeaetech/paeae/tree/master/Libraries/ds2482>
[3]:	<http://www.pjrc.com/teensy/td_libs_OneWire.html>
[4]:	<https://www.maximintegrated.com/en/products/analog/sensors-and-sensor-interface/DS18B20.html>
[5]:	<https://www.maximintegrated.com/en/products/interface/controllers-expanders/DS2482-100.html>
[6]:	<http://www.sheepwalkelectronics.co.uk/product_info.php?cPath=22&products_id=30>
