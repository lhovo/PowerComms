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

bool bStreamPackets = false;    /* Indicates whether the device is in transmit or receive mode */

int wTxCount = 0;        /* Number of packets transmitted */
int wRxCount = 0;        /* Number of packets received */
int wSuccessCount = 0;   /* Number of packets successfully acknowledged by the receiver */

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  Serial.println("Initializing");
  //pinMode(10, OUTPUT);

  plc.init();
  Serial.println("init finised");
  plc.ReadFromOffset(Local_LA_LSB, &localAddress, 1);
  Serial.println("local address found");
  Serial.println(localAddress);
  if (localAddress == 0x01)
  {
    destinationAddress = 0x02; 	/* Set the destination address to logical address 0x02 */
  }
  else
  {		
    destinationAddress = 0x01; /* Set the destination address to logical address 0x01 */ 
  }
  plc.SetDestinationAddress (TX_DA_Type_Log, &destinationAddress); 
  Serial.println("Set Destination");
  Serial.println(destinationAddress);
}

void loop()
{
  // nothing happens after setup finishes.
  Serial.println("looping");
  delay(5000);
  if (plc.IsPacketReceived() == TRUE)
  {
    wRxCount++;
    plc.ReadFromOffset(RX_SA, &temp, 1);
    destinationAddress = temp;
    plc.WriteToOffset(TX_DA, &destinationAddress, 1);

    Serial.print("SA=");
    Serial.println(temp);
    Serial.print("RX# = ");
    plc.ReadFromOffset(RX_CommandID, &temp, 1);
    if (temp == CMD_SENDMSG)
    { 
      plc.ReadFromOffset(RX_Data, &temp, 1);
      Serial.println(temp);
    }
    temp = 0x00; 
    plc.WriteToOffset(RX_Message_INFO, &bI2C_Temp, 1);
  }

  if ((bStreamPackets == true) && (wTxCount < MAX_TX_PACKETS))
  {
    /* Read the voltage of the input P0[1], store it as the data to transmit, and display on the first row of the LCD*/
    M8C_DisableIntMask (INT_MSK0, INT_MSK0_GPIO);
    ADCINC_bClearFlagGetData();   
    ADCINC_GetSamples(1);             
    while(ADCINC_fIsDataAvailable() == FALSE);
    abTxArray[0] = ADCINC_bClearFlagGetData(); /* Send the voltage as the data payload */
    M8C_EnableIntMask (INT_MSK0, INT_MSK0_GPIO);
    
    /* LCD top row will display destination address and the 8-bit value of the analog voltage */
    LCD_Position(0,0);
    LCD_PrCString("DA=");
    LCD_PrHexByte(bPLC_DestinationAddress);
    LCD_PrCString(" TX Data=  ");
    LCD_Position(0,14);
    LCD_PrHexByte(abTxArray[0]);
              
    /* Transmit the packet with the data read from the ADC */
    bPLC_Success = PLC_TransmitPacket(CMD_SENDMSG, abTxArray, 1);
    if (bPLC_Success & Status_TX_Data_Sent)
    {
      wSuccessCount++;
    }
    wTxCount++;
    
    /* LCD bottom row will display #Success / #Transmitted */
    LCD_Position(1,0);
    LCD_PrCString("TX# = ");    
    Print_Decimal(wSuccessCount, 2);
    LCD_PrCString("/");       
    Print_Decimal(wTxCount, 2);
  }
}



