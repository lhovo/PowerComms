/*****************************************************************************
*
* File Name: main.c
**
Version: 2.1
**
Description:
* This project performs two primary functions:
* 	1)When the pin P0[7] transitions from '1' to '0', this device sends I2C data to the PLC device to transmit a packet
*		over the powerline. The payload of the packet is determined by reading the analog voltage on pin P0[1] of the device. 
*		The voltage is converted into a byte by an ADC and then sent over I2C to the PLC device.
*		This device reads the transmission status from the PLC device and displays the transmission statistics on the LCD
*	2) When a message is received, this device will show the data and the reception statistics on the LCD.
*
* The following user modules are used:
*	1) I2CHW: Interfaces to the PLC device and accesses the device's memory array.
*	2) Programmable Gain Amplifier (PGA): Unity-gain input for the analog voltage on P0[1]
*	3) Incremental A/D Converter (ADCINC): Configured to output an 8-bit digital value representing the voltage on P0[1]
*	4) LCD: Displays the P0[1] value on the first row. 
*		When transmitting, displays the P0[1] value on the first row, displays the number of transmitted packets and successful transmissions on the second row.
*		When receiving, displays the received data value on the first row, displays the number of received packets on the second row.
*
*
Hardware Setup:
* This project is intended to be used on the PSoCEVAL1 development kit. 
* It is inteded to interface to the CY3272 or CY3273 PLC evaluation kits.
* The following connections need to be made:
*	1) Connect a 5-pin ribbon cable from the 5-pin header J11 on the PSoCEVAL1 kit to the 5-pin I2C header on the PLC kit
*	2) Connect a jumper wire from P17 on the PSoCEVAL1 kit to the INT pin on the PLC kit
*	3) On the PSoCEVAL1 kit, connect a jumper wire from P01 to VR
*	4) On the PSoCEVAL1 kit, connect a jumper wire from P07 to SW
*	5) On the PSoCEVAL1 kit, connect the LCD to the LCD header
*	6) On the PLC kit, connect three jumper shunts across SDA, SCL, and PWR

**
Owner:
* Jeff Hushley, Applications Engineer, fre@cypress.com
**
Code Tested With:
* 1. PSoC Designer 5.1
* 2. ImageCraft C Compiler
*
******************************************************************************
* Copyright (2010), Cypress Semiconductor Corporation.
******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of Cypress.

* This file contains Source Code. You may not use this file except in compliance with the Cypress
* Semiconductor Corporation IP Library License and Usage Agreement. Please obtain a copy of the
* Agreement at http://www.cypress.com/IP_Library_License_Agreement.pdf and read it before using this
* file.
* This file and all files distributed under the Agreement are distributed on an ’AS IS’ basis,
* WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS, IMPLIED OR STATUTORY, REGARDING THE
* SUFFICIENCY, ACCURACY OR COMPLETENESS OF THE INFORMATION AND CYPRESS
* HEREBY DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT. Cypress
* Semiconductor Corporation reserves the right to make changes to this file and any files distributed
* under the Agreement without further notice.
* Please see the License and Usage Agreement for the specific language governing rights and
* limitations under the Agreement.

*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the
* materials described herein. Cypress does not assume any liability arising out
* of the application or use of any product or circuit described herein. Cypress
* does not authorize its products for use as critical components in life-support
* systems where a malfunction or failure may reasonably be expected to result in
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
*****************************************************************************/

#include <m8c.h>        /* part specific constants and macros			*/
#include "PSoCAPI.h"    /* PSoC API definitions for all User Modules	*/

#include "delay.h"
#include "plc.h"
#include "plc_i2c.h"

#define MAX_TX_PACKETS 1000 /* The maximum number of packets to transmit. */

void Print_Decimal (WORD wHexData, BYTE bNumBytes);

BYTE bStateChange = 0x00;			/* Indicates whether a GPIO falling edge interrupt has occurred on pin P0[1]*/

void main(void)
{
	BYTE bPLC_Success = FALSE;		/* True when the packet was acknowledged */
	BYTE bPLC_LocalAddress;
	BYTE bPLC_DestinationAddress;
	BYTE bIndex;					/* For indexing loops */
	BYTE bI2C_Temp;					/* For temporarily storing data to be written to or read from I2C */
	BYTE abTxArray[16];				/* Transmit data */
	BYTE bStreamPackets = 0x00;		/* Indicates whether the device is in transmit or receive mode */
	WORD wTxCount = 0;				/* Number of packets transmitted */
	WORD wRxCount = 0;				/* Number of packets received */
	WORD wSuccessCount = 0;			/* Number of packets successfully acknowledged by the receiver */
	
	/* Initialize the user modules */
	M8C_EnableIntMask (INT_MSK0, INT_MSK0_GPIO);
	M8C_EnableGInt;						/* Enable M8C Interrupts for GPIO  */
	PGA_Start(PGA_HIGHPOWER);
	ADCINC_Start(ADCINC_HIGHPOWER);
	LCD_Start();
	LCD_Position(0,0);
	LCD_PrCString("I2C Test       ");	/* Display this message until the I2C communication is successful */
	
	/* Initialize the PLC device.
	 * Read the logical address from the device. On the CY3272 and CY3273 boards, the address is set by the DIP switches.
	 * If the address is 0x01, this device will be the master. Set the destination address to 0x02.
	 * If the address is not 0x01, set the destination address to 0x01 so that all messages are sent to the master. */
	PLC_Init();	
	PLC_I2C_ReadFromOffset(Local_LA_LSB, &bPLC_LocalAddress, 1);
	if (bPLC_LocalAddress == 0x01)
	{	
		bPLC_DestinationAddress = 0x02;
		PLC_SetDestinationAddress (TX_DA_Type_Log, &bPLC_DestinationAddress);	/* Set the destination address to logical address 0x02 */
	}
	else
	{		
		bPLC_DestinationAddress = 0x01;
		PLC_SetDestinationAddress (TX_DA_Type_Log, &bPLC_DestinationAddress); /* Set the destination address to logical address 0x01 */
	}
	
	/* Display the logical address of this node. It will be shown until a message is received or transmitted. */
	LCD_Start();
	LCD_Position(0,0);
	LCD_PrCString("LA=");
	LCD_PrHexByte(bPLC_LocalAddress); 		/* Display the logical address of the PLC device */
	LCD_PrCString(" RX Data=  ");	/* Top row will display the received data byte */
	LCD_Position(1,0);
	LCD_PrCString("RX# = 0000      ");	/* Bottom row will display the nummber of messages received */
	
	/* Infinite loop to send and receive messages. */ 
	while (1)
	{	
		/* 	If a message is received, increment the count and display it on the LCD */
		if (PLC_IsPacketReceived() == TRUE)
		{
			wRxCount++;
			
			/* Get the source address of the message. 
			 * Store it as the destination address, so that when this node starts transmitting messages, it will send them to the last node that it received a message from */
			PLC_I2C_ReadFromOffset(RX_SA, &bI2C_Temp, 1);
			bPLC_DestinationAddress = bI2C_Temp;
			PLC_I2C_WriteToOffset(TX_DA, &bPLC_DestinationAddress, 1);
			
			/* LCD top row will display the source address of the received message and the data */ 
			LCD_Position(0,0);
			LCD_PrCString("SA=");			
			LCD_PrHexByte(bI2C_Temp); 		/* Display the source address of the PLC message */
			LCD_PrCString(" RX Data=  ");	/* Display the 8-bit value of the received message */
						
			/* LCD bottom row will display the number of messages received */
			LCD_Position(1,0);
			LCD_PrCString("RX# =           ");	
			LCD_Position(1,6);
			/* To reduce processing time, comment out Print_Decimal and use LCD_PrHexInt to display the hex value */ 
			Print_Decimal(wRxCount, 2);
			
			/* If the received message command ID is a normal message, display the received data on the LCD */
			PLC_I2C_ReadFromOffset(RX_CommandID, &bI2C_Temp, 1);
			if (bI2C_Temp == CMD_SENDMSG)
			{ 
				PLC_I2C_ReadFromOffset(RX_Data, &bI2C_Temp, 1);
				LCD_Position(0,14);
				LCD_PrHexByte(bI2C_Temp);	
			}
			
			/* Reset the RX Message Info array variable to clear the RX buffer for new messages */
			bI2C_Temp = 0x00; 
			PLC_I2C_WriteToOffset(RX_Message_INFO, &bI2C_Temp, 1);
		}
		
		/* If there was a GPIO interrupt (button press/release) on pin P0[7], then invert the state of the transmitter. 
		 * Wait 1ms for debouncing the GPIO input */ 
		if (bStateChange == TRUE)
		{
			Delay50uTimes(20); 		
			bStreamPackets ^= TRUE;
			bStateChange = FALSE;
		}
		
		/* If the transmitter state is enabled, it will continuously transmit packets.
		 * Before it transmits, it increments the TX count and displays on the bottom row of the LCD.
		 * Then, it transmits a packet with the ADC voltage to the current destination address. 
		 * If it is a success, it will increment the TX success count and display it on the bottom row of the LCD. */		
		if ((bStreamPackets == TRUE) && (wTxCount < MAX_TX_PACKETS))
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
}


/*****************************************************************************
* Function Name: Print_Decimal()
******************************************************************************
* Summary:
* Display the input hex data in decimal form on the LCD
**
Parameters:
* wHexData: The data that is currently in hex form
* bNumBytes: The number of bytes of the data. It can be either 1 (BYTE) or 2 (WORD)
**
Return:
* None
**
Note:
* 
*****************************************************************************/
void Print_Decimal (WORD wHexData, BYTE bNumBytes)
{
	WORD wDecimalData;
	WORD wBitLocation = 16;
	WORD wDivisor = 10000;
	wDecimalData = 0x0000;
	do
	{
		wBitLocation -= 4;
		wDivisor /= 10;
		wDecimalData += (wHexData / wDivisor) << wBitLocation;
		wHexData %= wDivisor;
	}while (wBitLocation > 0x00);

	if (bNumBytes == 2 )
	{
		LCD_PrHexInt(wDecimalData);
	}
	else
	{
		LCD_PrHexByte(wDecimalData);
	}
}


/*****************************************************************************
* Interrupt Service Routine Name: TX_Trigger_Int()
******************************************************************************
* Summary:
* When a falling edge occurs on the GPIO pin P0[7], the state of transmitting packets changes.
**
Parameters:
* None
**
Return:
* None
**
Note:
* 
* This function is called from the PSoC_GPIO_ISR function in the file PSoCGPIOINT.asm (in the lib\Library Source Files folder).
* The line ljmp _TX_Trigger_Int was added to the PSoC_GPIO_ISR function so that it would call this function.
*****************************************************************************/
#pragma interrupt_handler TX_Trigger_Int
void TX_Trigger_Int( void )
{
	bStateChange = TRUE;
}
