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
byte temp;               /* For temporarily storing data to be written to or read from I2C */
byte TxArray[16];        /* Transmit data */

bool transmitter = true;
bool receiver = false;

bool bStreamPackets = false;    /* Indicates whether the device is in transmit or receive mode */
#define MAX_TX_PACKETS 1000 /* The maximum number of packets to transmit. */

int wTxCount = 0;        /* Number of packets transmitted */
int wRxCount = 0;        /* Number of packets received */
int wSuccessCount = 0;   /* Number of packets successfully acknowledged by the receiver */
int bPLC_Success = 0;

void setup()
{

  // Open serial communications and wait for port to open:
  Serial.begin(9600);


  Serial.println("Init Start");
  plc.init();
  Serial.println("Init End");
  
  if (transmitter) {
    localAddress = 0x01;
    destinationAddress = 0x02;
  }

  if (receiver) {
    localAddress = 0x02;
    destinationAddress = 0x01;
  }
  
  Serial.println("Local:");
  Serial.println(localAddress);
  Serial.println("Dest:");
  Serial.println(destinationAddress);

  plc.WriteToOffset(Local_LA_LSB, &localAddress, 1);
  plc.SetDestinationAddress (TX_DA_Type_Log, &destinationAddress); 

  TxArray[0] = 23;
}

void loop()
{
  Serial.println("looping");
  delay(1000);

  if (transmitter) {
    transmit(TxArray);
  }

  if (receiver) {
    receive();
  }
}

void transmit(byte *message) {
    // Transmit the packet with the data read from the ADC
    bPLC_Success = plc.TransmitPacket(CMD_SENDMSG, message, 1);
    if (bPLC_Success & Status_TX_Data_Sent)
    {
      wSuccessCount++;
    }
    wTxCount++;
}

void receive() {
  if (plc.IsPacketReceived() == true) {
    wRxCount++;
    plc.ReadFromOffset(RX_SA, &temp, 1);
    destinationAddress = temp;
    plc.WriteToOffset(TX_DA, &destinationAddress, 1);

    Serial.print("SA =");
    Serial.println(temp);
    Serial.print("RX# = ");
    plc.ReadFromOffset(RX_CommandID, &temp, 1);
    if (temp == CMD_SENDMSG)
    { 
      plc.ReadFromOffset(RX_Data, &temp, 1);
      Serial.println(temp);
    }
    temp = 0x00; 
    plc.WriteToOffset(RX_Message_INFO, &temp, 1);
  }
}


