
#include "plc_i2c.h"

#define I2C_GAP 	  1		/* The gap between the stop bit and start bit of the next I2C message. In multiples of 1ms. */
#define I2C_TIMEOUT	250		/* Set the I2C timeout to be 250ms. If there is no response, it will give up */

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
byte PLC_I2C::init()
{
	byte bTemp; 
	byte bI2CResult = I2C_SUCCESS;
	
	/* Start the I2C master and enable the global and local interrupts */   
    Start();
    
	/* Enable the PLC device */
  	bTemp = (TX_Enable | RX_Enable | RX_Override);
  	bI2CResult &= WriteToOffset(PLC_Mode, &bTemp, 1);
	
	/* Enable Interrupt reporting for all events */
	bTemp = (INT_UnableToTX | INT_TX_NO_ACK | INT_TX_NO_RESP | INT_RX_Packet_Dropped | INT_RX_Data_Available | INT_TX_Data_Sent);
	bI2CResult &= WriteToOffset(INT_Enable, &bTemp, 1);
	
	/* Set the PLC device to acknowledgment mode and 1 retry*/
	bTemp = (TX_Service_Type | 0x01) ;
  	bI2CResult &= WriteToOffset(TX_Config, &bTemp, 1);
			
	/* Set the transmitter gain. */
	bTemp = 0x0b;
	bI2CResult &= WriteToOffset (TX_Gain, &bTemp, 1);
	
	/* Set the receiver gain. */
	bTemp = 0x06;
	bI2CResult &= WriteToOffset (RX_Gain, &bTemp, 1);

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
byte PLC_I2C::SetDestinationAddress (byte bAddrType, byte *pbDestinationAddress)
{
	byte bI2CResult = I2C_SUCCESS;
	byte bTxConfigTemp = 0x00;
	
	/* Read the PLC TX_Config setting and prepare to update the value */
	bI2CResult &= ReadFromOffset(TX_Config, &bTxConfigTemp, 1);
	bTxConfigTemp &= ~TX_DA_Type;
	bTxConfigTemp |= bAddrType;
	
	/* Based on the addressing type, set the destination address with the corresponding number of bytes */
	if (bAddrType == TX_DA_Type_Log)
	{
		bI2CResult &= WriteToOffset(TX_DA, pbDestinationAddress, 1);
	}
	else if (bAddrType == TX_DA_Type_Grp)
	{
		bI2CResult &= WriteToOffset(TX_DA, pbDestinationAddress, 1);
	}
	else if (bAddrType == TX_DA_Type_Phy)
	{
		bI2CResult &= WriteToOffset(TX_DA, pbDestinationAddress, 8);
	}
	else
		return PLC_INVALID;
	
	/* Since the address type was valid, the PLC TX_Config setting can get set */
	bI2CResult &= WriteToOffset(TX_Config, &bTxConfigTemp, 1);
	
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
byte PLC_I2C::TransmitPacket(byte bCommand, byte *pbTXData, byte bDataLength)
{
	byte bI2CResult = I2C_SUCCESS;
	byte bPLCResult;
	byte bBIUThreshold;
	byte bPLCMode;
	
	if (bDataLength > MAX_PLC_PACKET_LENGTH)
	{
		return PLC_INVALID;
	}
	/* Clear the PLC device's interrupt status, set the bCommand ID and write the data payload */
	bI2CResult &= WriteToOffset(TX_CommandID, &bCommand, 1);
	bI2CResult &= WriteToOffset(TX_Data, pbTXData, bDataLength);
	
	/* Loop until the message is transmitted */
	do
	{	
		/* Set the Send_Message bit and the length of the PLC packet, which will initiate transmission */
		bDataLength |= Send_Message; 
		bI2CResult &= WriteToOffset(TX_Message_Length, &bDataLength, 1);
		
		/* Wait until the transmitter status is updated */
		do
		{
			while(!(PLC_INT_BYPASS || IsUpdated()));
			bI2CResult &= ReadFromOffset(INT_Status, &bPLCResult, 1);
		}while (!(bPLCResult & (Status_UnableToTX | Status_TX_Data_Sent | Status_TX_NO_ACK | Status_TX_NO_RESP)));
		
	
		/* If there was a Band-In-Use(BIU) Timeout condition, increase the BIU threshold until the packet is transmitted */
		if (bPLCResult & Status_UnableToTX)
		{
			bI2CResult &= ReadFromOffset(Threshold_Noise, &bBIUThreshold, 1);
			if ((bBIUThreshold & BIU_Threshold_Mask) < BIU_Threshold_Mask)
			{
				bBIUThreshold++;
				bI2CResult &= WriteToOffset(Threshold_Noise, &bBIUThreshold, 1);
			}
			/* If it is still timing out at the maximum BIU threshold, then disable BIU */
			else
			{
				bI2CResult &= ReadFromOffset(PLC_Mode, &bPLCMode, 1);
				bPLCMode |= Disable_BIU;
				bI2CResult &= WriteToOffset(PLC_Mode, &bPLCMode, 1);
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
byte PLC_I2C::IsPacketReceived(void)
{
	byte bPLC_Status;
	/* First, check if the PLC device's HOST_INT pin (which is connected to P1[7] of this device) is set to '1'
	 * This indicates that a PLC status update has occurred.
	 * To skip checking the HOST_INT pin and to continously poll the INT_Status register via I2C, set PLC_INT_BYPASS to 1 */ 
	if (PLC_INT_BYPASS || IsUpdated()) 
	{
		/* Check if the PLC device's status register indicates that a new received message is available */
		ReadFromOffset(INT_Status, &bPLC_Status, 1);
		if (bPLC_Status & Status_RX_Data_Available)
		{
			return true;
		}
	}
	return false;
}

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

void PLC_I2C::Start(void)
{
  /* Initial delay of 1.25s is required before I2C communication should start with PLC */
  delay(1250);
  Wire.begin();
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
byte PLC_I2C::WriteToOffset(byte bOffset, byte *pbData, byte bDataLength)
{
  byte bI2CResult = I2C_SUCCESS;
  
  /* Send the start bit and address byte */
  Wire.beginTransmission(PLC_ADDRESS);
	
  /* Send the offset byte */
  bI2CResult = Wire.write(bOffset);
	
  /* Send each data byte and check if there was an I2C failure*/
  bI2CResult = Wire.write(pbData, bDataLength);
  if (bI2CResult == I2C_FAIL)
    return I2C_FAIL;
  
  /* Send the stop bit */
  Wire.endTransmission();
  
  /* Ensure that there is sufficient delay between stop and start bits */
  delay(I2C_GAP);
  		
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
byte PLC_I2C::ReadFromOffset (byte bOffset, byte *pbData, byte bDataLength)
{
	byte bI2C_Timeout_Cycles = 0;
  int i = 0;
	/* Make sure the interrupts are enabled */
  
  // TODO INT
	/* I2CHW_EnableInt();  */
	
	/* Ensure that there is sufficient delay between stop and start bits */
	delay(I2C_GAP);
  
  /* Send the start bit and address byte */
  Wire.beginTransmission(PLC_ADDRESS);
  
  /* Send the offset byte */
  Wire.write(bOffset);
	
  /* Send the stop bit */
  Wire.endTransmission();
	
//	 /* Wait until the data is written or a timeout occurs*/ 
//	while(!(I2CHW_bReadI2CStatus() & I2CHW_WR_COMPLETE) && (bI2C_Timeout_Cycles < I2C_TIMEOUT))
//	{
//		delay(1);	// 1ms delay
//		bI2C_Timeout_Cycles++;
//	}
			
//	/* Clear Write Complete Status bit */  
//    I2CHW_ClrWrStatus(); 
	
	/* Ensure that there is sufficient delay between stop and start bits */
	delay(I2C_GAP);
	
  /* Read from the slave and place in pbData */;
  Wire.requestFrom(PLC_ADDRESS, (int)bDataLength);
  
  for (i=0; Wire.available(); i++) // slave may send less than requested
  {
    pbData[i] = Wire.read();    // receive a byte as character
  }
	
//  /* Wait until the data is read or a timeout occurs*/  
//	bI2C_Timeout_Cycles = 0;
//	while(!(I2CHW_bReadI2CStatus() & I2CHW_RD_COMPLETE) && (bI2C_Timeout_Cycles < I2C_TIMEOUT))
//	{
//		delay(1);	/* 1ms delay */
//		bI2C_Timeout_Cycles++;
//	}
//	 /* Clear Read Complete Status bit */  
//    I2CHW_ClrRdStatus();
	
	/* Ensure that there is sufficient delay between stop and start bits */
	delay(I2C_GAP);
		
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
byte PLC_I2C::IsUpdated(void)
{
//	/* Check the status of the pin P0[7] to see if the PLC device has asserted the HOST_INT pin. */
//	if (PLC_INT_Data_ADDR & PLC_INT_MASK)
//	{
//		return TRUE;
//	}
//	else
//	{
//		return FALSE;
//	}
}

