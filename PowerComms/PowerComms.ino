/*
  SD card basic file example

 This example shows how to create and destroy an SD card file
 The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4

 created   Nov 2010
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

 */

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "plc_i2c.h"

PLC_I2C plc;
byte destinationAddress;
byte localAddress;

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  Serial.print("Initializing");
  //pinMode(10, OUTPUT);

//  plc.init();
//  plc.ReadFromOffset(Local_LA_LSB, &localAddress, 1);
//  if (localAddress == 0x01)
//  {
//    destinationAddress = 0x02; 	/* Set the destination address to logical address 0x02 */
//  }
//  else
//  {		
//    destinationAddress = 0x01; /* Set the destination address to logical address 0x01 */ 
//  }
//  plc.SetDestinationAddress (TX_DA_Type_Log, &destinationAddress); 
}

void loop()
{
  // nothing happens after setup finishes.
  Serial.print("Initializing");
  delay(5000);
}



