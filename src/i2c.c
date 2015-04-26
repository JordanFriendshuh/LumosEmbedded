#include "../headers/i2c.h"
#include "../headers/sysctrl.h"

//*****************************************************************************
// Initializes a given I2C peripheral to operate at 100KHz.  This assumes
// MCU core is running at 50MHz
//
// Paramters:
//    base_addr:  The base address of the I2C peripheral that is being
//                configured
//
// Return Value:
//    Returns I2C_OK if the base address is a valid I2C peripheral
//    Returns I2C_INVALID_BASE if the base address is not a valid I2C address
//*****************************************************************************
i2c_status_t initializeI2CMaster(uint32_t base_addr)
{
//  I2C0_Type *myI2C;
  
//  myI2C = (I2C0_Type *) base_addr;
  
  // Validate that a correct base address has been passed
    // Turn on the Clock Gating Register
    switch (base_addr) 
    {
      case I2C0_BASE :
    	  HWREG(SYSCTL_RCGCI2C) |= SYSCTL_RCGCI2C_R0;
          //SYSCTL->RCGCI2C |= SYSCTL_RCGCI2C_R0;
          while ((HWREG(SYSCTL_PRI2C) & SYSCTL_PRI2C_R0) == 0);    /* wait until SSI is ready */
          break;
      case I2C1_BASE :
          HWREG(SYSCTL_RCGCI2C) |= SYSCTL_RCGCI2C_R1;
          while ((HWREG(SYSCTL_PRI2C) & SYSCTL_PRI2C_R1) == 0);    /* wait until SSI is ready */
          break;
      case I2C2_BASE :
    	  HWREG(SYSCTL_RCGCI2C) |= SYSCTL_RCGCI2C_R2;
          while ((HWREG(SYSCTL_PRI2C) & SYSCTL_PRI2C_R2) == 0);    /* wait until SSI is ready */
          break;
      case I2C3_BASE :
    	  HWREG(SYSCTL_RCGCI2C) |= SYSCTL_RCGCI2C_R3;
          while ((HWREG(SYSCTL_PRI2C) & SYSCTL_PRI2C_R3) == 0);    /* wait until SSI is ready */
          break;
      default:
          return I2C_INVALID_BASE;
    }
    
    // Enable the I2C port as master
    HWREG(base_addr + I2C_O_MCR) = I2C_MCR_MFE;
    
    // Set the clock speed to be 100Kpbs assuming a 50MHz clock
    // TPR = (System Clock/(2*(SCL_LP + SCL_HP)*SCL_CLK))-1;
    // TPR = (50MHz/(2*(6+4)*100000))-1
    HWREG(base_addr + I2C_O_MTPR) = 0x18;
    
    return I2C_OK;
}

//*****************************************************************************
// verifies if the base addressed passed to a function is a valid I2C address
//*****************************************************************************
bool
i2cVerifyBaseAddr(
  uint32_t i2c_base
)
{
    if( i2c_base == I2C0_BASE ||
        i2c_base == I2C1_BASE ||
        i2c_base == I2C2_BASE ||
        i2c_base == I2C3_BASE
    )
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

//*****************************************************************************
// Sets the slave address that is currently being addressed.
//
// Paramters:
//    baseAddr:  The base address of the I2C peripheral that is being
//                configured
//    slaveAddr: The address of the slave device being accessed.  This is a 
//                7-bit slave address
//    readWrite: If the the transaction will be a read or write operation.
//               read = 0x01.  write = 0x00
// Return Value:
//    Returns I2C_OK if the base address is a valid I2C peripheral
//    Returns I2C_INVALID_BASE if the base address is not a valid I2C address
//*****************************************************************************
i2c_status_t i2cSetSlaveAddr(
  uint32_t baseAddr, 
  uint8_t slaveAddr,
  i2c_read_write_t readWrite
)
{
  //I2C0_Type *myI2C;
  if( i2cVerifyBaseAddr(baseAddr) == 0)
  {
    return I2C_INVALID_BASE;
  }
  
  //myI2C = (I2C0_Type *) baseAddr;
  
  // Set the slave address to transmit data
   //myI2C->MSA = (slaveAddr << 1) | readWrite;
   HWREG(baseAddr + I2C_O_MSA) = (slaveAddr << 1) | readWrite;
  //myI2C->MSA = slaveAddr | readWrite;
  
  return I2C_OK;
}


//*****************************************************************************
// Initiates a stop condition.  This function is used when the master does
// not receive and ACK.
//
// Paramters:
//    baseAddr:  The base address of the I2C peripheral that is being
//                configured
// Return Value:
//    Returns I2C_OK if the base address is a valid I2C peripheral
//    Returns I2C_INVALID_BASE if the base address is not a valid I2C address
//*****************************************************************************
i2c_status_t i2cStop(
  uint32_t baseAddr
)
{
  //I2C0_Type *myI2C;
  if( i2cVerifyBaseAddr(baseAddr) == 0)
  {
    return I2C_INVALID_BASE;
  }
  
  //myI2C = (I2C0_Type *) baseAddr;
  
  // Stop the interface
  //myI2C->MCS = I2C_MCS_STOP;
  HWREG(baseAddr + I2C_O_MCS) = I2C_MCS_STOP;
  
  return I2C_OK;
}

//*****************************************************************************
// Determines if the I2C device is busy transmitting data.  
//
// Paramters:
//    baseAddr:  The base address of the I2C peripheral that is being accessed
//
// Returns
//    Returns 1 if the I2C device is busy
//    Returns 0 if the I2C device is NOT busy or i2c_base is invalid
//*****************************************************************************
bool
I2CMasterBusy(uint32_t i2c_base)
{
  //I2C0_Type *myI2C;
  
  if( i2cVerifyBaseAddr(i2c_base) == 0)
  {
    return 0;
  }
  
    //myI2C = (I2C0_Type *) i2c_base;
    
    if( HWREG(i2c_base + I2C_O_MCS) & I2C_MCS_BUSY)
    {
        return(1);
    }
    else
    {
        return(0);
    }
}

//*****************************************************************************
// Determines if the address (control word) was ACKed.  
//
// Paramters:
//    baseAddr:  The base address of the I2C peripheral that is being accessed
//
// Returns
//    Returns 1 if the address was ACKed
//    Returns 0 if the address was not ACKed or i2c_base is invalid
//*****************************************************************************
bool
I2CMasterAdrAck(uint32_t i2c_base)
{
  //  I2C0_Type *myI2C;
    uint32_t status;
  if( i2cVerifyBaseAddr(i2c_base) == 0)
  {
    return 0;
  }
  
  //myI2C = (I2C0_Type *) i2c_base;
  
  //status = myI2C->MCS;
  status = HWREG(i2c_base + I2C_O_MCS);
  if((status & I2C_MCS_ADRACK)!= 0)
  {
      return(0);
  }
  else
  {
      return(1);
  }
}


//*****************************************************************************
// Determines if the last byte of data transmitted was ACKed.  
//
// Paramters:
//    baseAddr:  The base address of the I2C peripheral that is being accessed
//
// Returns
//    Returns 1 if the data was ACKed
//    Returns 0 if the data was not ACKed or i2c_base is invalid
//*****************************************************************************
bool
I2CMasterDatAck(uint32_t i2c_base)
{
    //I2C0_Type *myI2C;
    uint32_t status;
  if( i2cVerifyBaseAddr(i2c_base) == 0)
  {
    return 0;
  }
  
  //myI2C = (I2C0_Type *) i2c_base;
  
  //status = myI2C->MCS;
  status = HWREG(i2c_base + I2C_O_MCS);
  if((status & I2C_MCS_DATACK)!= 0)
  {
      return(0);
  }
  else
  {
      return(1);
  }
}

//*****************************************************************************
// Sends one byte of data over a I2C bus.  
//
// Paramters:
//    baseAddr:  The base address of the I2C peripheral that is being
//                configured
//    byte:      The next byte of the data transaction.
//    mcs:       Sets master control register of the I2C peripheral. Since a
//               data transaction can consist of multiple bytes, the value
//               of mcs is used to determine when start and stop bits are 
//               generated.
//
// Examples:
//    * Sending control word (i2c address) + write 1st byte of data. This
//    * assumes that additional bytes will follow in the transaction
//    i2cSendByte(I2C0_BASE, byte, I2C_MCS_START | I2C_MCS_RUN);
//
//    * Sending a byte in the middle of a multi-byte operation. (> 2 bytes)
//    i2cSendByte(I2C0_BASE, byte, I2C_MCS_RUN);
//
//    * Sending final byte  of data
//    i2cSendByte(I2C0_BASE, byte, I2C_MCS_RUN | I2C_MCS_STOP);
//
//    * Sending control word (i2c address) + a single byte transaction.
//    i2cSendByte(I2C0_BASE, byte, I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP);
//
// Return Value:
//    Returns I2C_OK if the base address is a valid and the data was 
//    transmitted sucessfully.
//*****************************************************************************
i2c_status_t i2cSendByte(
  uint32_t baseAddr, 
  uint8_t byte, 
  uint8_t mcs
)
{
  //I2C0_Type *myI2C;
  
  if( i2cVerifyBaseAddr(baseAddr) == 0)
  {
    return I2C_INVALID_BASE;
  }
  //myI2C = (I2C0_Type *) baseAddr;
  
   // Write the upper address to the data register
  //myI2C->MDR   = byte;
  HWREG(baseAddr + I2C_O_MDR) = byte;
  // Start the transaction
  //myI2C->MCS = mcs;
  HWREG(baseAddr + I2C_O_MCS) = mcs;

  // Wait for the device to be free
  while ( I2CMasterBusy(baseAddr)) {};
  
    // Check for error conditions
  if ( HWREG(baseAddr + I2C_O_MCS) & (I2C_MCS_ERROR | I2C_MCS_ARBLST) )
  {
      return I2C_ARBLST;
  }
  else if ( HWREG(baseAddr + I2C_O_MCS) & I2C_MCS_ERROR )
  {
	HWREG(baseAddr + I2C_O_MCS) = I2C_MCS_STOP;
    return I2C_BUS_ERROR;
  }
  else if (HWREG(baseAddr + I2C_O_MCS) & I2C_MCS_DATACK )
  {
    return I2C_NO_ACK;
  }
  else
  {
    return I2C_OK;
  }
}

//*****************************************************************************
// Reads one byte of data over a I2C bus.  
//
// Paramters:
//    baseAddr:  The base address of the I2C peripheral that is being
//                configured
//    data:      The data read by the transaction.
//    mcs:       Sets master control register of the I2C peripheral. Since a
//               data transaction can consist of multiple bytes, the value
//               of mcs is used to determine when start and stop bits are 
//               generated.
//
// Examples:
//    * Sending control word (i2c address) + read 1st byte of data. This
//    * assumes that additional bytes be read after this byte.  This
//    * would be used for reading multiple bytes at consecutive addresses
//    * in the slave address.  
//    i2cGetByte(I2C0_BASE, byte, I2C_MCS_START | I2C_MCS_RUN);
//
//    * Reading a byte in the middle of a multi-byte operation. (> 2 bytes)
//    i2cGetByte(I2C0_BASE, byte, I2C_MCS_RUN);
//
//    * Reading final byte of data in a multi-byte read 
//    i2cGetByte(I2C0_BASE, byte, I2C_MCS_RUN | I2C_MCS_STOP);
//
//    * Sending control word (i2c address) + reading a single byte read.
//    i2cGetByte(I2C0_BASE, byte, I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP);
//
// Return Value:
//    Returns I2C_OK if the base address is a valid and the data was 
//    transmitted sucessfully.
//*****************************************************************************
i2c_status_t i2cGetByte(
  uint32_t baseAddr, 
  uint8_t *data, 
  uint8_t mcs
)
{
  //I2C0_Type *myI2C;
  
  if( i2cVerifyBaseAddr(baseAddr) == 0)
  {
    return I2C_INVALID_BASE;
  }
  
 //myI2C = (I2C0_Type *) baseAddr;
  
  // Start the transaction
  //myI2C->MCS = mcs;
  HWREG(baseAddr + I2C_O_MCS) = mcs;

  // Wait for the device to be free
  while ( I2CMasterBusy(baseAddr)) {};
  
  // Check for error conditions
  if ( HWREG(baseAddr + I2C_O_MCS) & I2C_MCS_ERROR  )
  {
	  HWREG(baseAddr + I2C_O_MCS) = I2C_MCS_STOP;
    return I2C_BUS_ERROR;
  }
  else
  {
    *data = HWREG(baseAddr + I2C_O_MDR);
    return I2C_OK;
  }
}
