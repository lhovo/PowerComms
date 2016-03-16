/*
* File Name: plc_i2c.h
**
Version: 2.1
**
Description:
* This file contains the global constants and function declarations for controlling a PLC device via I2C commands
**
Note:

 */

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>
#include "plc_commands.h"

#define PLC_ADDRESS 0x01

#define I2C_FAIL 0x00
#define I2C_SUCCESS 0x01
#define PLC_INVALID 0x02

#define MAX_PLC_PACKET_LENGTH 31

class PLC_I2C {
  public:
    byte init(bool transmitter);
    byte SetDestinationAddress (byte bAddrType, byte *pbDestinationAddress);
    byte TransmitPacket(byte bCommand, byte *pbTXData, byte bDataLength);
    byte IsPacketReceived(void);
    
    byte ReadFromOffset (byte bOffset, byte *pbData, byte bDataLength);
    byte WriteToOffset(byte bOffset, byte *pbData, byte bDataLength);
  private:
    void Start(void);
    byte IsUpdated(void);
};
