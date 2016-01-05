
#include "plc_i2c.h"

#define I2C_GAP 	  1		/* The gap between the stop bit and start bit of the next I2C message. In multiples of 1ms. */
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

void PLC_I2C::Start(void)
{
  int i;
  /* Initial delay of 1.25s is required before I2C communication should start with PLC */
  for (i = 0; i < 1000; i++)
  {
    delay(1250); /* 50us * 25 = 1.25 ms * 1000 = 1.25 s */
  }
	
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

