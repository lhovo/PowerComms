/*
* File Name: plc_i2c.h
**
Version: 2.1
**
Description:
* This file contains the global constants and function declarations for controlling a PLC device via I2C commands
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
#include <I2CHWCommon.h>  
#include <I2CHWmstr.h>  
  
#define I2C_SLAVE_ADDRESS 0x01

#define I2C_FAIL 0x00
#define I2C_SUCCESS 0x01
#define PLC_INVALID 0x02

#define MAX_PLC_PACKET_LENGTH 31

void PLC_I2C_Start(void);
BYTE PLC_I2C_WriteToOffset(BYTE bOffset, BYTE *pbData, BYTE bDataLength);
BYTE PLC_I2C_ReadFromOffset (BYTE bOffset, BYTE *pbData, BYTE bDataLength);
BYTE PLC_I2C_IsUpdated(void);

