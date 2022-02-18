/*******************************************************************************
* File Name: ExternalMemoryInterface.c
*
* Version: 1.0
*
* Description:
*  Provides an API for the external memory access.
*
* Hardware Dependency:
*  CY8CKIT-042 BLE
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

#include "main.h"

#include "Options.h"
#include "ExternalMemoryInterface.h"

#ifndef CY_SCB_EMI_SPIM_H

#include "xmem/xmem.h"

//uint8 emiWriteBuffer[EMI_SIZE_OF_WRITE_BUFFER];

/*******************************************************************************
* Function Name: EMI_Start
********************************************************************************
*
* Summary:
*  Starts the external memory interface.
*
* Parameters:
*  None
*
* Return:
*  None
*******************************************************************************/
void EMI_Start(void)
{
    /* Start the SCB component */
	(void) XMEM_iniz();
}


/*******************************************************************************
* Function Name: EMI_WriteData
********************************************************************************
*
* Summary:
*  Write data to the external memory.
*
* Parameters:
*  uint8 instruction:
*   Whether the master wants to read/write/erase/program the NOR Flash etc.
*   The instruction set is as per the Serial NOR Flash part's datasheet.
*  uint32 addressBytes:
*   Address of the location in the Serial NOR Flash to write to.
*  uint32 dataSize:
*   Amount of data to write.
*  uint8 *data:
*   Pointer to data that is written to external memory
*
* Return:
*  Status
*     Value               Description
*    CYRET_SUCCESS           Successful
*    Other non-zero          Failure
*******************************************************************************/
cystatus EMI_WriteData(uint8 instruction, uint32 addressBytes, uint32 dataSize, uint8 *data)
{
    if (XMEM_write(addressBytes, data, dataSize) )
    	return CYRET_SUCCESS ;
    else
    	return CYRET_UNKNOWN ;
}


/*******************************************************************************
* Function Name: EMI_IsBusy
********************************************************************************
*
* Summary:
*  Read whether the external memory is busy.
*
* Parameters:
*  None   
*
* Return:
*  bool
*     Value               Description
*     false                 Not busy
*     true                  Busy
*
*******************************************************************************/
bool EMI_IsBusy(void)
{
    bool status = false;
    
    return status;
}


/*******************************************************************************
* Function Name: EMI_ReadData
********************************************************************************
*
* Summary:
*  Read data from the external memory.
*
* Parameters:
*  uint32 addressBytes: The address in external memory from where to start.
*   
*  uint32 dataSize: Amount of data to be read
*   
*  uint8 *data:     Data is copied to this array.
*   
*
* Return:
*  Status
*     Value               Description
*    CYRET_SUCCESS           Successful
*    Other non-zero          Failure
*    CYBLE_ERROR_INVALID_PARAMETER - problems with decryption
*******************************************************************************/
cystatus EMI_ReadData(uint32 addressBytes, uint32 dataSize, uint8 *data)
{
	if (XMEM_read(addressBytes, data, dataSize))
    	return CYRET_SUCCESS ;
    else
    	return CYRET_UNKNOWN ;
}

/*******************************************************************************
* Function Name: EMI_EraseAll
********************************************************************************
*
* Summary:
*  Erases content of the external memory.
*  Caution: All data is erased, including metadata.
*
* Parameters:
*  None
*
* Return:
*  Status
*     Value                     Description
*    CYRET_SUCCESS                  Successful
*    Other non-zero                 Failure
*
* Side Effects:
*  None
*
*******************************************************************************/
cystatus EMI_EraseAll(void)
{
    /* Erase in terms of blocks - each block is of 64 kB.
     * So erasing four blocks should suffice for 256 kB chips as well.
     * Caution: All data is erased, including metadata.
     */
	uint32_t addr = 0 ;
	int i ;
	const int NUM_BLOCKS = (256 * 1024) / XMEM_BLOCK_SIZE ;
	for (i=0 ; i<NUM_BLOCKS ; ++i, addr += XMEM_BLOCK_SIZE) {
		if (!XMEM_erase_block(addr))
			break ;
	}

	return NUM_BLOCKS == i ? CYRET_SUCCESS : CYRET_UNKNOWN ;
}


#endif
