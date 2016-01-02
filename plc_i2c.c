/*
* File Name: plc_i2c.c
**
Version: 2.1
**
Description:
* This file contains APIs for controlling a PLC device via I2C commands
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

#include <m8c.h> 
#include "PSoCAPI.h"
#include "PSoCGPIOINT.h"
#include "plc_i2c.h"
#include "delay.h"


#define I2C_GAP 	10		/* The gap between the stop bit and start bit of the next I2C message. In multiples of 50us. */

#define I2C_TIMEOUT	250		/* Set the I2C timeout to be 250ms. If there is no response, it will give up */


/*****************************************************************************
* Function Name: PLC_I2C_Start()
******************************************************************************
* Summary:
* Initialize the I2C hardware block
**
Parameters:
* None
**
Return:
* None
**
Note:
* 
*****************************************************************************/
void PLC_I2C_Start(void)
{
	INT i;
	/* Initial delay of 1.25s is required before I2C communication should start with PLC */
	for (i = 0; i < 1000; i++)
    {
        Delay50uTimes(25); /* 	50us * 25 = 1.25 ms * 1000 = 1.25 s 	*/
    }
	
	M8C_EnableGInt;  
	I2CHW_Start();  
    I2CHW_EnableMstr();  	
    I2CHW_EnableInt();  
}

/*****************************************************************************
* Function Name: PLC_I2C_WriteToOffset()
******************************************************************************
* Summary:
* Write an I2C message of bDataLength to the specified PLC memory offset.
* The offset is the first byte written after the I2C address byte. It is followed
* by the data to be written starting at the memory offset.
**
Parameters:
* bOffset: PLC memory offset to write to
* pbData: pointer to the data that will be written to the PLC device
* bDataLength: length of the data
**
Return:
* Status of the I2C communication.  
**
Note:
* 
*****************************************************************************/
BYTE PLC_I2C_WriteToOffset(BYTE bOffset, BYTE *pbData, BYTE bDataLength)
{
	INT i;
	BYTE bI2CResult = I2C_SUCCESS;
	
	/* Send the start bit and address byte */
	bI2CResult &= I2CHW_fSendStart(I2C_SLAVE_ADDRESS, I2CHW_WRITE );
		
	/* Send the offset byte */
	bI2CResult &= I2CHW_fWrite(bOffset);
	
	/* Send all of the data bytes */
	for (i = 0; i < bDataLength; i++)
	{
		/* Send each data byte and check if there was an I2C failure*/
		bI2CResult &= I2CHW_fWrite(pbData[i]);
		if (bI2CResult == I2C_FAIL)
			return I2C_FAIL;
	}
	
	/* Send the stop bit */
	I2CHW_SendStop();
	
	/* Ensure that there is sufficient delay between stop and start bits */
	Delay50uTimes(I2C_GAP);
			
	return bI2CResult;    
}

/*****************************************************************************
* Function Name: PLC_I2C_ReadFromOffset()
******************************************************************************
* Summary:
* Read an I2C message of bDataLength from the specified PLC memory offset.
* The offset is first written as a complete I2C message to set up the memory
* offset to read from. Then, the data is read from the device.
**
Parameters:
* bOffset: PLC memory offset to read from
* pbData: pointer to the data that will be stored when read from the PLC device
* bDataLength: length of the data
**
Return:
* Status of the I2C communication.  
**
Note:
* 
*****************************************************************************/
BYTE PLC_I2C_ReadFromOffset (BYTE bOffset, BYTE *pbData, BYTE bDataLength)
{
	BYTE bI2C_Timeout_Cycles = 0;
	/* Make sure the interrupts are enabled */
	I2CHW_EnableInt();  
	
	/* Ensure that there is sufficient delay between stop and start bits */
	Delay50uTimes(I2C_GAP);
		
	/* Write the offset byte to set the memory offset*/ 
	I2CHW_bWriteBytes(I2C_SLAVE_ADDRESS, &bOffset, 1, I2CHW_CompleteXfer); 
	
	 /* Wait until the data is written or a timeout occurs*/ 
	while(!(I2CHW_bReadI2CStatus() & I2CHW_WR_COMPLETE) && (bI2C_Timeout_Cycles < I2C_TIMEOUT))
	{
		Delay50uTimes(20);	// 1ms delay
		bI2C_Timeout_Cycles++;
	}
			
	/* Clear Write Complete Status bit */  
    I2CHW_ClrWrStatus(); 
	
	/* Ensure that there is sufficient delay between stop and start bits */
	Delay50uTimes(I2C_GAP);
	
	/* Read from the slave and place in pbData */;  
    I2CHW_fReadBytes(I2C_SLAVE_ADDRESS, pbData, bDataLength, I2CHW_CompleteXfer);  
	
    /* Wait until the data is read or a timeout occurs*/  
	bI2C_Timeout_Cycles = 0;
	while(!(I2CHW_bReadI2CStatus() & I2CHW_RD_COMPLETE) && (bI2C_Timeout_Cycles < I2C_TIMEOUT))
	{
		Delay50uTimes(20);	/* 1ms delay */
		bI2C_Timeout_Cycles++;
	}
	 /* Clear Read Complete Status bit */  
    I2CHW_ClrRdStatus();
	
	/* Ensure that there is sufficient delay between stop and start bits */
	Delay50uTimes(I2C_GAP);
		
	return I2C_SUCCESS;
     
}

/*****************************************************************************
* Function Name: PLC_I2C_IsUpdated()
******************************************************************************
* Summary:
* Responds TRUE if the PLC device has an event update 
**
Parameters:
* None
**
Return:
* TRUE if the PLC device has an event update. FALSE otherwise
**
Note:
* 
*****************************************************************************/
BYTE PLC_I2C_IsUpdated(void)
{
	/* Check the status of the pin P0[7] to see if the PLC device has asserted the HOST_INT pin. */
	if (PLC_INT_Data_ADDR & PLC_INT_MASK)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}