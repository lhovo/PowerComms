/*
* File Name: plc.c
**
Version: 2.1
**
Description:
* This file contains APIs for controlling the CY8CPLC10 device
**
Note:

**
Owner:
* Jeff Hushley, Applications Engineer, fre@cypress.com
**
Code Tested With:
* 1. PSoC Designer 5.1
* 2. ImageCraft C Compiler
*
******************************************************************************
* Copyright (2009), Cypress Semiconductor Corporation.
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
**
Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH
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
#include "plc.h"
#include "delay.h"
#include "plc_i2c.h"

#define PLC_INT_BYPASS 1 	/* When 0, this device will check if the PLC device has asserted its HOST_INT pin high to 
							 * 	indicate than a PLC event has occurred. This device will then check the event type via I2C.
						  	 * When 1, this device will ignore the HOST_INT pin and continuosly poll the PLC device for 
							 * 	event updates with I2C. */

/*****************************************************************************
* Function Name: PLC_Init()
******************************************************************************
* Summary:
* Initialize the PLC interface
**
Parameters:
* None
**
Return:
* I2C_SUCCESS if I2C communication was successful. I2C_FAIL otherwise.
**
Note:
* 
*****************************************************************************/
BYTE PLC_Init(void)
{  
	BYTE bTemp; 
	BYTE bI2CResult = I2C_SUCCESS;
	
	/* Start the I2C master and enable the global and local interrupts */    
    PLC_I2C_Start();
		
	/* Enable the PLC device */
  	bTemp = (TX_Enable | RX_Enable | RX_Override);
  	bI2CResult &= PLC_I2C_WriteToOffset(PLC_Mode, &bTemp, 1);
	
	/* Enable Interrupt reporting for all events */
	bTemp = (INT_UnableToTX | INT_TX_NO_ACK | INT_TX_NO_RESP | INT_RX_Packet_Dropped | INT_RX_Data_Available | INT_TX_Data_Sent);
	bI2CResult &= PLC_I2C_WriteToOffset(INT_Enable, &bTemp, 1);
	
	/* Set the PLC device to acknowledgment mode and 1 retry*/
	bTemp = (TX_Service_Type | 0x01) ;
  	bI2CResult &= PLC_I2C_WriteToOffset(TX_Config, &bTemp, 1);
			
	/* Set the transmitter gain. */
	bTemp = 0x0b;
	bI2CResult &= PLC_I2C_WriteToOffset (TX_Gain, &bTemp, 1);
	
	/* Set the receiver gain. */
	bTemp = 0x06;
	bI2CResult &= PLC_I2C_WriteToOffset (RX_Gain, &bTemp, 1);

	return bI2CResult;	 
}  

/*****************************************************************************
* Function Name: PLC_SetDestinationAddress()
******************************************************************************
* Summary:
* Sets the PLC Destination Address according to the address type
**
Parameters:
* bAddrType: Destiation Address Type. Refer to TX_DA_Type constants
* pbDestinationAddress: pointer to the destination address
**
Return:
* Status of the I2C communication and address type validity.  
**
Note:
* 
*****************************************************************************/
BYTE PLC_SetDestinationAddress (BYTE bAddrType, BYTE *pbDestinationAddress)
{
	BYTE bI2CResult = I2C_SUCCESS;
	BYTE bTxConfigTemp = 0x00;
	
	/* Read the PLC TX_Config setting and prepare to update the value */
	bI2CResult &= PLC_I2C_ReadFromOffset(TX_Config, &bTxConfigTemp, 1);
	bTxConfigTemp &= ~TX_DA_Type;
	bTxConfigTemp |= bAddrType;
	
	/* Based on the addressing type, set the destination address with the corresponding number of bytes */
	if (bAddrType == TX_DA_Type_Log)
	{
		bI2CResult &= PLC_I2C_WriteToOffset(TX_DA, pbDestinationAddress, 1);
	}
	else if (bAddrType == TX_DA_Type_Grp)
	{
		bI2CResult &= PLC_I2C_WriteToOffset(TX_DA, pbDestinationAddress, 1);
	}
	else if (bAddrType == TX_DA_Type_Phy)
	{
		bI2CResult &= PLC_I2C_WriteToOffset(TX_DA, pbDestinationAddress, 8);
	}
	else
		return PLC_INVALID;
	
	/* Since the address type was valid, the PLC TX_Config setting can get set */
	bI2CResult &= PLC_I2C_WriteToOffset(TX_Config, &bTxConfigTemp, 1);
	
	return bI2CResult;
}

/*****************************************************************************
* Function Name: PLC_TransmitPacket()
******************************************************************************
* Summary:
* Initiates a PLC packet transmission of a data packet of specified length
**
Parameters:
* bCommand: Command ID of the PLC message
* pbTXData: pointer to the data payload that will be in the PLC message
* bDataLength: length of the data payload
**
Return:
* Status of the PLC communication.  
**
Note:
* 
*****************************************************************************/
BYTE PLC_TransmitPacket(BYTE bCommand, BYTE *pbTXData, BYTE bDataLength)
{
	BYTE bI2CResult = I2C_SUCCESS;
	BYTE bPLCResult;
	BYTE bBIUThreshold;
	BYTE bPLCMode;
	
	if (bDataLength > MAX_PLC_PACKET_LENGTH)
	{
		return PLC_INVALID;
	}
	/* Clear the PLC device's interrupt status, set the bCommand ID and write the data payload */
	bI2CResult &= PLC_I2C_WriteToOffset(TX_CommandID, &bCommand, 1);
	bI2CResult &= PLC_I2C_WriteToOffset(TX_Data, pbTXData, bDataLength);
	
	/* Loop until the message is transmitted */
	do
	{	
		/* Set the Send_Message bit and the length of the PLC packet, which will initiate transmission */
		bDataLength |= Send_Message; 
		bI2CResult &= PLC_I2C_WriteToOffset(TX_Message_Length, &bDataLength, 1);
		
		/* Wait until the transmitter status is updated */
		do
		{
			while(!(PLC_INT_BYPASS || PLC_I2C_IsUpdated()));
			bI2CResult &= PLC_I2C_ReadFromOffset(INT_Status, &bPLCResult, 1);
		}while (!(bPLCResult & (Status_UnableToTX | Status_TX_Data_Sent | Status_TX_NO_ACK | Status_TX_NO_RESP)));
		
	
		/* If there was a Band-In-Use(BIU) Timeout condition, increase the BIU threshold until the packet is transmitted */
		if (bPLCResult & Status_UnableToTX)
		{
			bI2CResult &= PLC_I2C_ReadFromOffset(Threshold_Noise, &bBIUThreshold, 1);
			if ((bBIUThreshold & BIU_Threshold_Mask) < BIU_Threshold_Mask)
			{
				bBIUThreshold++;
				bI2CResult &= PLC_I2C_WriteToOffset(Threshold_Noise, &bBIUThreshold, 1);
			}
			/* If it is still timing out at the maximum BIU threshold, then disable BIU */
			else
			{
				bI2CResult &= PLC_I2C_ReadFromOffset(PLC_Mode, &bPLCMode, 1);
				bPLCMode |= Disable_BIU;
				bI2CResult &= PLC_I2C_WriteToOffset(PLC_Mode, &bPLCMode, 1);
			}
		}	
	} while (!(bPLCResult & (Status_TX_Data_Sent | Status_TX_NO_ACK | Status_TX_NO_RESP)));
	
	return bPLCResult;
}


/*****************************************************************************
* Function Name: PLC_IsPacketReceived()
******************************************************************************
* Summary:
* Responds TRUE if a message has been received by the PLC device
**
Parameters:
* None
**
Return:
* TRUE if a message has been received. FALSE otherwise
**
Note:
* 
*****************************************************************************/
BYTE PLC_IsPacketReceived(void)
{
	BYTE bPLC_Status;
	/* First, check if the PLC device's HOST_INT pin (which is connected to P1[7] of this device) is set to '1'
	 * This indicates that a PLC status update has occurred.
	 * To skip checking the HOST_INT pin and to continously poll the INT_Status register via I2C, set PLC_INT_BYPASS to 1 */ 
	if (PLC_INT_BYPASS || PLC_I2C_IsUpdated()) 
	{
		/* Check if the PLC device's status register indicates that a new received message is available */
		PLC_I2C_ReadFromOffset(INT_Status, &bPLC_Status, 1);
		if (bPLC_Status & Status_RX_Data_Available)
		{
			return TRUE;
		}
	}
	return FALSE;
}