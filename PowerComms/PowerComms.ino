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
byte data[32];

uint32_t* dataVal = (uint32_t*) data;
uint32_t oldData = 0, timeoutCount = 0;

bool transmitter = true;
bool receiver = !transmitter;

bool bStreamPackets = false;    /* Indicates whether the device is in transmit or receive mode */
#define MAX_TX_PACKETS 1000 /* The maximum number of packets to transmit. */

int wTxCount = 0;        /* Number of packets transmitted */
int wRxCount = 0;        /* Number of packets received */
int wSuccessCount = 0;   /* Number of packets successfully acknowledged by the receiver */
int bPLC_Success = 0;
int i=0; //itterator

uint8_t pinArray[] = {
        // 1 VDD
        // 2 RX (0)
        // 3 TX (1)
     9, // 4 - PWM
    10, // 5 - PWM

        // 6 GND
    14, // 7 - A0
    15, // 8 - A1
    16, // 9 - A2
     8, // 10
};

void setup()
{

  // Open serial communications and wait for port to open:
//  Serial.begin(9600);


//  Serial.println("Init Start");
  plc.init(transmitter);
//  Serial.println("Init End");
  
  if (transmitter) {
    localAddress = 0x01;
    destinationAddress = 0x02;
  }

  if (receiver) {
    localAddress = 0x02;
    destinationAddress = 0x01;
  }
  
//  Serial.println("Local:");
//  Serial.println(localAddress);
//  Serial.println("Dest:");
//  Serial.println(destinationAddress);

  plc.WriteToOffset(Local_LA_LSB, &localAddress, 1);
  plc.SetDestinationAddress (TX_DA_Type_Log, &destinationAddress);

  if(receiver) {
    for(i=0; i<sizeof(pinArray);i++){
      pinMode( pinArray[i], OUTPUT);
    }
  }
  else if (transmitter) {
    for(i=0; i<sizeof(pinArray);i++){
      pinMode( pinArray[i], INPUT_PULLUP);
    }
  }
}

void loop()
{
  if (transmitter) {
    (*dataVal) = 0;
    for(i=0; i<sizeof(pinArray);i++){
      (*dataVal) |= digitalRead( pinArray[i] ) << i;
    }

    if (oldData != (*dataVal) || timeoutCount > 0xFFF)
    {
      oldData = (*dataVal);
      transmit(data, 2);
//      Serial.print("Tx:");
//      Serial.println(data[0]);
      timeoutCount = 0;
    }
    delay(1);
    timeoutCount ++;
  }
  else if (receiver) {
    receive();
    if(oldData != (*dataVal))
    {
      oldData = (*dataVal);
//      Serial.print("Rx:");
//      Serial.println(*dataVal);
      for(i=0; i<sizeof(pinArray);i++){
        if ( (*dataVal) & (0x01<< i) )
        {
          digitalWrite( pinArray[i], true );
        }
        else
        {
          digitalWrite( pinArray[i], false );
        }
      }
    }
  }
}

void transmit(byte *message, byte dataLength) {
    // Transmit the packet with the data read from the ADC
    bPLC_Success = plc.TransmitPacket(CMD_SENDMSG, message, dataLength);
    if (bPLC_Success & Status_TX_Data_Sent)
    {
      wSuccessCount++;
    }
    wTxCount++;
}

void receive() {
  byte temp;
  while (plc.IsPacketReceived() == true) {
//    wRxCount++;
//    plc.ReadFromOffset(RX_SA, &temp, 1);
//    destinationAddress = temp;
//    plc.WriteToOffset(TX_DA, &destinationAddress, 1);

    //Serial.print("SA =");
    //Serial.println(temp);
//    Serial.print("RX# = ");
    plc.ReadFromOffset(RX_CommandID, &temp, 1);
    if (temp == CMD_SENDMSG)
    { 
      plc.ReadFromOffset(RX_Message_INFO, &temp, 1);
      plc.ReadFromOffset(RX_Data, data, temp & 0xF);
//      Serial.println(*dataVal);
      //Serial.print("PK Len# = ");
      //Serial.println(temp & 0xF);
    }
    temp = 0x00; 
    plc.WriteToOffset(RX_Message_INFO, &temp, 1);
  }
}


