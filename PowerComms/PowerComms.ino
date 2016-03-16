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
uint32_t data = 0, oldData = 0;

bool transmitter = false;
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
        // 2 RX
        // 3 TX
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
    data = 0;
    for(i=0; i<sizeof(pinArray);i++){
      data |= digitalRead( pinArray[i] ) << i;
    }
    
    // Some magic
    // The first byte should be zero repesenting the first packet.
    // Second byte should be 1 repesenting the second packet.
    // We need to offset the data to fit this id in the packet
    data = data << 1;
    data = data & 0xff | (data & 0xff00 << 1) | 0x0100 ;

    if (oldData != data)
    {
      oldData = data;
      transmit((byte*)(&data));
      delay(1);
      transmit((byte*)(&data)+1);
      Serial.print("Tx:");
      Serial.println(data);
    }
  }
  else if (receiver) {
    receive();
    if(oldData != data)
    {
      oldData == data;
      Serial.print("Rx:");
      Serial.println(data);
      for(i=0; i<sizeof(pinArray);i++){
        if ( data & (0x01<< i) )
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

void transmit(uint8_t *message) {
    // Transmit the packet with the data read from the ADC
    bPLC_Success = plc.TransmitPacket(CMD_SENDMSG, message, 1);
    if (bPLC_Success & Status_TX_Data_Sent)
    {
      wSuccessCount++;
    }
    wTxCount++;
}

void receive() {
  byte temp;
  while (plc.IsPacketReceived() == true) {
    wRxCount++;
    plc.ReadFromOffset(RX_SA, &temp, 1);
    destinationAddress = temp;
    plc.WriteToOffset(TX_DA, &destinationAddress, 1);

    //Serial.print("SA =");
    //Serial.println(temp);
    Serial.print("RX# = ");
    plc.ReadFromOffset(RX_CommandID, &temp, 1);
    if (temp == CMD_SENDMSG)
    { 
      plc.ReadFromOffset(RX_Data, &temp, 1);
      Serial.println(temp);
      if ((temp & 0x01) == 0x00) {
        data = (data & 0xff80) | (temp >> 1);
      }
      else if ((temp & 0x01) == 0x01) {
        data = (data & 0x7f) | ((temp & 0xFE) << 7);
      }
    }
    temp = 0x00; 
    plc.WriteToOffset(RX_Message_INFO, &temp, 1);
  }
}


