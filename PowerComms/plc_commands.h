/*
* File Name: plc.h
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


/**********************  PLT_Memory_Array ************************/
typedef enum {	
	INT_Enable = 0,
	Local_LA_LSB = 0x01,
	Local_LA_MSB = 0x02,
	Local_Group = 0x03,
	Local_Group_Hot = 0x04,
	PLC_Mode = 0x05,
	TX_Message_Length = 0x06,
	TX_Config = 0x07,
	TX_DA = 0x08,
	TX_CommandID = 0x10,
	TX_Data = 0x11,
		
	Threshold_Noise = 0x30,
	Modem_Config = 0x31,
	TX_Gain = 0x32,
	RX_Gain = 0x33,
	Timing_Config = 0x34,
	RX_Message_INFO = 0x40,
	RX_SA = 0x41,
	RX_CommandID = 0x49,
	RX_Data = 0x4A,
	INT_Status = 0x69,
	Local_PA = 0x6A,
	Local_FW = 0x72,
	
} Concrete_Memory_offset;

/**********************  B I T  M A S K S  ************************/

// Constant Table - 
// Memory offset - 00 Interrupt Enable
//
#define INT_Clear					0x80
#define INT_Polarity				0x40
#define INT_UnableToTX				0x20
#define INT_TX_NO_ACK				0x10
#define INT_TX_NO_RESP				0x08
#define INT_RX_Packet_Dropped		0x04
#define INT_RX_Data_Available		0x02
#define INT_TX_Data_Sent			0x01
//
// Memory offset - 01 Logical Address Normal
//
// Memory offset - 02 Logical Address Extended
//
// Memory offset - 03 Single Group Member
//
// Memory offset - 04 Multiple Group
//
// Memory offset - 05 PLC Mode
//
#define TX_Enable					0x80 
#define RX_Enable					0x40
#define Lock_Configuration			0x20
#define Disable_BIU					0x10
#define RX_Override					0x08
#define Set_Ext_Address				0x04
#define Promiscuous_MASK			0x02
#define Promiscuous_CRC_MASK		0x01
//
// Memory offset - 06 TX_Message_Length
//
#define Send_Message				0x80
#define Payload_Length_MASK			0x1F
//
// Memory offset - 07 TX_Config
//
#define TX_SA_Type					0x80
#define TX_DA_Type					0x60
#define TX_Ext_Address				0x10 // For Legacy Support
#define TX_Service_Type				0x10 
#define TX_Response					0x02 // For Legacy Support

#define TX_SA_Type_Log				0x00
#define TX_SA_Type_Phy				0x80

#define TX_DA_Type_Log				0x00
#define TX_DA_Type_Grp				0x20
#define TX_DA_Type_Phy				0x40
//
#define TX_Retry					0x0F
//
// Memory offset - 08 - 10 TX_DA
//
// Memory offset - 11 - 2F TX_Data
//
// Memory offset - 30 Threshold_Noise
//
#define BIU_Threshold_Mask			0x07


// Memory offset - 31 Modem_Config
//
// Transmit Modem Delay
#define Modem_TXDelay 				0x60
#define Modem_TXDelay_7ms 			0x00
#define Modem_TXDelay_13ms 			0x20
#define Modem_TXDelay_19ms 			0x40
#define Modem_TXDelay_25ms 			0x60

// FSK Deviation (Bandwidth) 
#define Modem_FSKBW					0x08
#define Modem_FSKBW_3M				0x08	
#define Modem_FSKBW_1_5M			0x00 
// Baud Rate
#define Modem_BPS					0x03
#define Modem_BPS_2400				0x03
#define Modem_BPS_1800				0x02
#define Modem_BPS_1200				0x01
#define Modem_BPS_600				0x00 


// Memory offset - 32 TX_Gain
//
#define TX_Gain_Mask				0x0F

// Memory offset - 33 RX_Gain
//
#define RX_Gain_Mask				0x07



// Memory offset - 40 RX_Message_INFO
//
#define New_RX_Msg					0x80
#define RX_DA_Type					0x40
#define RX_SA_Type					0x20
#define RX_Msg_Length				0x1F

#define RX_DA_UNIQUE				0x00
#define RX_DA_GROUP					0x40

#define RX_SA_LOG					0x00
#define RX_SA_PHY					0x20

//
// Memory offset - 41 - 48 RX_SA
//
// Memory offset - 49 RX_CommandID
//
// Memory offset - 4A - 68 RX_Data
//
// Memory offset - 69 Status
//
#define Status_Value_Change				0x80
#define Status_UnableToTX				0x20
#define Status_TX_NO_ACK				0x10
#define Status_TX_NO_RESP				0x08
#define Status_RX_Packet_Dropped		0x04
#define Status_RX_Data_Available		0x02
#define Status_TX_Data_Sent				0x01
#define Status_TX						0x39

// Memory offset - 6A Physical Address

// Memory offset - 72 Version


/**********************  Remote Command IDs ************************/
#define CMD_START_REMOTE_TX				0x01 
#define CMD_RESET						0x02
#define CMD_SET_EXT_MODE				0x03  
#define CMD_SETLOGICALADDR				0x04
#define CMD_GETLOGICALADDR				0x05
#define CMD_GETPHYSICALADDR				0x06
#define CMD_GET_STATE					0x07 
#define CMD_GETFWVERSION				0x08 

#define CMD_SENDMSG						0x09
#define CMD_SENDMSGWITHRESPONSE			0x0A
#define CMD_RESPONSE					0x0B
#define CMD_SET_BIU						0x0C
#define CMD_SET_THRESHOLD				0x0D
#define CMD_SETGROUPMEMBERSHIP			0x0E
#define CMD_GETGROUPMEMBERSHIP			0x0F