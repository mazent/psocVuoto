/*******************************************************************************
* File Name: BootloaderEmulator.c
*
* Version: 1.0
*
* Description:
*  Provides an API for the bootloader emulator. The bootloader emulator is
*  a light-weight version of the Bootloader component that updates external
*  memory with the new application code and data.
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

//#define STAMPA_DBG
#include "utili/includimi.h"

#ifdef CY_BOOTLOADABLE_Bootloadable_H

#include "BootloaderEmulator_PVT.h"
#include "ExternalMemoryInterface.h"
#include "BT_bts.h"

extern void WDOG_calcia(void) ;

#define BootloaderEmulator_activeApp      (BootloaderEmulator_MD_BTLDB_ACTIVE_0)

uint8 metadata[CY_FLASH_SIZEOF_ROW] ;

uint16 appSizeInRows ;
uint16 appFirstRowNum ;
uint16 appExtMemChecksum = 0 ;

#if (ENCRYPTION_ENABLED == YES)
uint8 emiKey[KEY_LENGTH] ;
#endif /*ENCRYPTION_ENABLED == YES*/

/***************************************
*     Function Prototypes
***************************************/
static cystatus BootloaderEmulator_WritePacket(
    uint8 status,
    uint8 buffer[],
    uint16 size) ;
static uint16 BootloaderEmulator_CalcPacketChecksum(
    const uint8 buffer[],
    uint16 size) ;

/*******************************************************************************
* Function Name: BootloaderEmulator_CalcPacketChecksum
********************************************************************************
*
* Summary:
*  This computes the 16 bit checksum for the provided number of bytes contained
*  in the provided buffer
*
* Parameters:
*  buffer:
*     The buffer containing the data to compute the checksum for
*  size:
*     The number of bytes in the buffer to compute the checksum for
*
* Returns:
*  16 bit checksum for the provided data
*
*******************************************************************************/
static uint16 BootloaderEmulator_CalcPacketChecksum(
    const uint8 buffer[],
    uint16 size)
{
#if (0 != BootloaderEmulator_PACKET_CHECKSUM_CRC)

    uint16 CYDATA crc = BootloaderEmulator_CRC_CCITT_INITIAL_VALUE ;
    uint16 CYDATA tmp ;
    uint8 CYDATA i ;
    uint16 CYDATA tmpIndex = size ;

    if ( 0 == size ) {
        crc = ~crc ;
    }
    else {
        do {
            tmp = buffer[tmpIndex - size] ;

            for ( i = 0 ; i < 8 ; i++ ) {
                if ( 0 != ( (crc & 0x0001u) ^ (tmp & 0x0001u) ) ) {
                    crc =
                        (crc >> 1) ^ BootloaderEmulator_CRC_CCITT_POLYNOMIAL ;
                }
                else {
                    crc >>= 1 ;
                }

                tmp >>= 1 ;
            }

            size-- ;
        } while ( 0 != size ) ;

        crc = ~crc ;
        tmp = crc ;
        crc = ( uint16 ) (crc << 8) | (tmp >> 8) ;
    }

    return crc ;

#else

    uint16 CYDATA sum = 0 ;

    while ( size > 0 ) {
        sum += buffer[size - 1] ;
        size-- ;
    }

    return ( uint16 ) 1 + ( uint16 ) (~sum) ;

#endif     /* (0 != BootloaderEmulator_PACKET_CHECKSUM_CRC) */
}

/*******************************************************************************
* Function Name: BootloaderEmulator_Calc8BitSum
********************************************************************************
*
* Summary:
*  This computes the 8 bit sum for the provided number of bytes contained in
*  Serial NOR Flash.
*
* Parameters:
* rowNum :
*   Flash row number as located in the external memory.
*
* Returns:
*   The 8 bit sum for the provided data.
*
*******************************************************************************/
uint32 BootloaderEmulator_Calc8BitSum(uint32 rowNum)
{
    uint32 sum = 0 ;
    uint8 flashRowFromExtMem[CY_FLASH_SIZEOF_ROW] ;
    uint32 size = CY_FLASH_SIZEOF_ROW ;

    (void) EMI_ReadData(EMI_APP_ABS_ADDR(rowNum),
                        CY_FLASH_SIZEOF_ROW,
                        flashRowFromExtMem) ;

    DBG_PUTS("") ;
    DBG_PUTS("BootloaderEmulator_Calc8BitSum() function.") ;

    DBG_PRINTF("rowNum  = %d", rowNum) ;

    DBG_PRINTF( "EMI_APP_ABS_ADDR(rowNum)  = %d",
                EMI_APP_ABS_ADDR( (uint16) rowNum ) ) ;

//    DBG_PUTS("EMI_ReadData():") ;
//    DBG_PRINT_ARRAY(flashRowFromExtMem, size) ;

    while ( size > 0 ) {
        size-- ;
        sum += flashRowFromExtMem[size] ;
    }

    return sum ;
}

/*******************************************************************************
* Function Name: BootloaderEmulator_Start
********************************************************************************
* Summary:
*  This function is called in order to execute the following algorithm:
*
*  - Identify the active bootloadable application (applicable only to
*    the Multi-application bootloader)
*
*  - Validate the bootloader application (design-time configurable, Bootloader
*    application validation option of the component customizer)
*
*  - Validate the active bootloadable application. If active bootloadable
*    application is not valid, and the other bootloadable application (inactive)
*    is valid, the last one is started.
*
*  - Run a communication subroutine (design-time configurable, Wait for command
*    option of the component customizer)
*
*  - Schedule the bootloadable and reset the device
*
* Parameters:
*  None
*
* Return:
*  This method will never return. It will either load a new application and
*  reset the device or jump directly to the existing application. The CPU is
*  halted, if validation failed when "Bootloader application validation" option
*  is enabled.
*  PSoC 3/PSoC 5: The CPU is halted if Flash initialization fails.
*
* Side Effects:
*  If Bootloader application validation option is enabled and this method
*  determines that the bootloader application itself is corrupt, this method
*  will not return, instead it will simply hang the application.
*
*******************************************************************************/
//MZ void BootloaderEmulator_Start(void)
//MZ {
//MZ     EMI_Start() ;
//MZ
//     BootloaderEmulator_HostLink(BootloaderEmulator_WAIT_FOR_COMMAND_FOREVER)
// ;
//MZ }

/*******************************************************************************
* Function Name: BootloaderEmulator_ValidateBootloadable
********************************************************************************
* Summary:
*  Performs the bootloadable application validation by calculating the
*  application image checksum and comparing it with the checksum value stored
*  in the Bootloadable Application Checksum field of the metadata section.
*
*  If the Fast bootloadable application validation option is enabled in the
*  component customizer and bootloadable application successfully passes
*  validation, the Bootloadable Application Verification Status field of the
*  metadata section is updated. Refer to the Metadata Layout section for the
*  details.
*
*  If the Fast bootloadable application validation option is enabled and
*  Bootloadable Application Verification Status field of the metadata section
*  claims that bootloadable application is valid, the function returns
*  CYRET_SUCCESS without further checksum calculation.
*
* Parameters:
*  appId:
*  The number of the bootloadable application should be 0 for the normal
*  bootloader and 0 or 1 for the Multi-Application bootloader.
*
* Returns:
*  Returns CYRET_SUCCESS if the specified bootloadable application is valid.
*
*******************************************************************************/
cystatus BootloaderEmulator_ValidateBootloadable(void)
{
    uint8 appChecksum ;
    uint8 calcedChecksum = 0 ;
    uint8 appFlashRow[CY_FLASH_SIZEOF_ROW] ;
    uint32 row ;
    uint32 i ;
    uint32 valid = 0 ;

    cystatus status = CYRET_SUCCESS ;

    DBG_PRINTF("BootloaderEmulator_ValidateBootlodable(): ") ;
    if ( 0 != appSizeInRows ) {
        DBG_PRINTF("appSizeInRows: %d", appSizeInRows) ;

        /* Get bootloadable application checksum from external memory */
        (void) EMI_ReadData(EMI_APP_ABS_ADDR(appSizeInRows - 1),
                            CY_FLASH_SIZEOF_ROW,
                            appFlashRow) ;

//        DBG_PUTS("App Row here: ") ;
//        DBG_PRINT_ARRAY(appFlashRow, CY_FLASH_SIZEOF_ROW) ;
//        DBG_PUTS("") ;

        appChecksum = appFlashRow[BootloaderEmulator_MD_APP_CHECKSUM] ;

        for ( row = 0 ; row < (appSizeInRows - 1U) ; row++ ) {
            /* Read flash row data from external memory */
            (void) EMI_ReadData(EMI_APP_ABS_ADDR(row),
                                CY_FLASH_SIZEOF_ROW,
                                appFlashRow) ;

            /* Calculate checksum of application row */
            for ( i = 0 ; i < CY_FLASH_SIZEOF_ROW ; i++ ) {
                if ( (appFlashRow[i] != 0) && (appFlashRow[i] != 0xFF) ) {
                    valid = 1 ;
                }
                calcedChecksum += appFlashRow[i] ;
            }
        }

        DBG_PRINTF("calcedChecksum = %x", calcedChecksum) ;

        calcedChecksum = ( uint8 ) 1 + ( uint8 ) (~calcedChecksum) ;

        if ( (calcedChecksum != appChecksum) || (0 == valid) ) {
            status = CYRET_BAD_DATA ;
        }

        DBG_PRINTF("BootloaderEmulator_ValidateBootloadable() Status: %d",
                   (uint16) status) ;

        DBG_PRINTF("Stored Bootloadable Application Checksum  = %d",
                   (uint16) appChecksum) ;

        DBG_PRINTF("Calculated Bootloadable Application Checksum  = %d",
                   (uint16) calcedChecksum) ;
    }
    else {
        status = CYRET_BAD_DATA ;
        DBG_PUTS("Error: Stored data length = 0.") ;
        DBG_PUTS("") ;
    }

    return status ;
}

/*
 * Contiene la prima parte di BootloaderEmulator_HostLink
 */

static CYBIT communicationState = BootloaderEmulator_COMMUNICATION_STATE_IDLE ;

static uint8 packetBuffer[BootloaderEmulator_SIZEOF_COMMAND_BUFFER] ;
static uint8 dataBuffer[BootloaderEmulator_SIZEOF_COMMAND_BUFFER] ;

static uint16 CYDATA dataOffset = 0 ;

void BootloaderEmulator_iniz(void)
{
    communicationState = BootloaderEmulator_COMMUNICATION_STATE_IDLE ;
    dataOffset = 0 ;

    /* Initialize communications channel. */
    CyBLE_CyBtldrCommStart() ;
}

/*******************************************************************************
* Function Name: BootloaderEmulator_HostLink
********************************************************************************
*
* Summary:
*  Causes the bootloader to attempt to read data being transmitted by the
*  host application.  If data is sent from the host, this establishes the
*  communication interface to process all requests.
*
* Parameters:
*  timeOut:
*   The amount of time to listen for data before giving up. Timeout is
*   measured in 10s of ms.  Use 0 for an infinite wait.
*
* Return:
*   None
*
*******************************************************************************/
void BootloaderEmulator_HostLink(const uint8 timeOut)
{
    uint16 CYDATA numberRead ;
    uint16 CYDATA rspSize ;
    uint8 CYDATA ackCode ;
    uint16 CYDATA pktChecksum ;
    cystatus CYDATA readStat ;
    uint16 CYDATA pktSize = 0 ;
//MZ    uint16 CYDATA dataOffset = 0 ;
    uint8 CYDATA timeOutCnt = 10 ;

#if (0 != BootloaderEmulator_FAST_APP_VALIDATION)
    uint8 CYDATA clearedMetaData = 0 ;
#endif      /* (0 != BootloaderEmulator_FAST_APP_VALIDATION) */

//MZ    CYBIT communicationState = BootloaderEmulator_COMMUNICATION_STATE_IDLE ;
//MZ
//MZ    uint8 packetBuffer[BootloaderEmulator_SIZEOF_COMMAND_BUFFER] ;
//MZ    uint8 dataBuffer[BootloaderEmulator_SIZEOF_COMMAND_BUFFER] ;
//MZ
//MZ    /* Initialize communications channel. */
//MZ    CyBLE_CyBtldrCommStart() ;
//MZ
//MZ    /* Enable global interrupts */
//MZ    CyGlobalIntEnable ;

    do {
        ackCode = CYRET_SUCCESS ;

        // MZ
        WDOG_calcia() ;

        do {
            readStat = CyBLE_CyBtldrCommRead(
                packetBuffer,
                BootloaderEmulator_SIZEOF_COMMAND_BUFFER,
                &numberRead,
                (0 == timeOut) ? 0xFF : timeOut) ;
            if ( 0 != timeOut ) {
                timeOutCnt-- ;
            }
        } while ( (0 != timeOutCnt) && (readStat != CYRET_SUCCESS) ) ;

        if ( readStat != CYRET_SUCCESS ) {
            break ;
        }

        if ( (numberRead < BootloaderEmulator_MIN_PKT_SIZE) ||
             (packetBuffer[BootloaderEmulator_SOP_ADDR] !=
              BootloaderEmulator_SOP) ) {
            DBG_ERR ;
            ackCode = BootloaderEmulator_ERR_DATA ;
        }
        else {
            pktSize =
                ( (uint16) ( (uint16) packetBuffer[BootloaderEmulator_SIZE_ADDR
                                                   + 1] << 8 ) ) |
                packetBuffer[BootloaderEmulator_SIZE_ADDR] ;

            pktChecksum =
                ( (uint16) ( (uint16) packetBuffer[BootloaderEmulator_CHK_ADDR(
                                                       pktSize) +
                                                   1] << 8 ) ) |
                packetBuffer[BootloaderEmulator_CHK_ADDR(pktSize)] ;

            if ( (pktSize + BootloaderEmulator_MIN_PKT_SIZE) > numberRead ) {
                DBG_ERR ;
                DBG_PRINTF("\t %d + %d > %d",
                           pktSize,
                           BootloaderEmulator_MIN_PKT_SIZE,
                           numberRead) ;
                ackCode = BootloaderEmulator_ERR_LENGTH ;
            }
            else if ( packetBuffer[BootloaderEmulator_EOP_ADDR(pktSize)] !=
                      BootloaderEmulator_EOP ) {
                DBG_ERR ;
                ackCode = BootloaderEmulator_ERR_DATA ;
            }
            else if ( pktChecksum !=
                      BootloaderEmulator_CalcPacketChecksum(packetBuffer,
                                                            pktSize +
                                                            BootloaderEmulator_DATA_ADDR) )
            {
                DBG_ERR ;
                ackCode = BootloaderEmulator_ERR_CHECKSUM ;
            }
            else {
                /* Empty section */
            }
        }

        rspSize = 0 ;
        if ( ackCode == CYRET_SUCCESS ) {
            uint8 CYDATA btldrData =
                packetBuffer[BootloaderEmulator_DATA_ADDR] ;

            ackCode = BootloaderEmulator_ERR_DATA ;
            switch ( packetBuffer[BootloaderEmulator_CMD_ADDR] ) {
            /***************************************************************************
            *   Verify checksum
            ***************************************************************************/
            case BootloaderEmulator_COMMAND_CHECKSUM:

                DBG_PUTS("Command: Checksum") ;
                if ( (BootloaderEmulator_COMMUNICATION_STATE_ACTIVE ==
                      communicationState) && (pktSize == 0) ) {
                    packetBuffer[BootloaderEmulator_DATA_ADDR] =
                        (uint8) (BootloaderEmulator_ValidateBootloadable() ==
                                 CYRET_SUCCESS) ;

                    DBG_PUTS("") ;
                    DBG_PUTS("BootloaderEmulator:") ;
                    DBG_PRINTF("\tVerify Checksum Command: %d",
                               packetBuffer[BootloaderEmulator_DATA_ADDR]) ;
                    DBG_PUTS("") ;

                    rspSize = 1 ;
                    ackCode = CYRET_SUCCESS ;
                }
                break ;

                /***************************************************************************
                *   Get flash size
                ***************************************************************************/

#if (0 != BootloaderEmulator_CMD_GET_FLASH_SIZE_AVAIL)

            case BootloaderEmulator_COMMAND_REPORT_SIZE:

                DBG_PUTS("Command: Report Size") ;
                if ( (BootloaderEmulator_COMMUNICATION_STATE_ACTIVE ==
                      communicationState) && (pktSize == 1) ) {
                    packetBuffer[BootloaderEmulator_DATA_ADDR] = LO8(
                        BootloaderEmulator_FIRST_ROW_IN_ARRAY) ;
                    packetBuffer[BootloaderEmulator_DATA_ADDR + 1] = HI8(
                        BootloaderEmulator_FIRST_ROW_IN_ARRAY) ;

                    packetBuffer[BootloaderEmulator_DATA_ADDR + 2] = LO8(
                        BootloaderEmulator_NUMBER_OF_ROWS_IN_ARRAY - 1) ;

                    packetBuffer[BootloaderEmulator_DATA_ADDR + 3] = HI8(
                        BootloaderEmulator_NUMBER_OF_ROWS_IN_ARRAY - 1) ;

                    rspSize = 4 ;
                    ackCode = CYRET_SUCCESS ;

                    DBG_PRINTF("First Available Row in Array: 0x%04X",
                               BootloaderEmulator_FIRST_ROW_IN_ARRAY) ;
                    DBG_PUTS("") ;

                    DBG_PRINTF("Last Available Row in Array: 0x%04X",
                               BootloaderEmulator_NUMBER_OF_ROWS_IN_ARRAY) ;
                }
                break ;

#endif              /* (0 != BootloaderEmulator_CMD_GET_FLASH_SIZE_AVAIL) */

            /***************************************************************************
            *   Program / Erase row
            ***************************************************************************/
            case BootloaderEmulator_COMMAND_PROGRAM:

                DBG_PUTS("Command: Program. ") ;
                /* The btldrData variable holds Flash Array ID */

#if (0 != BootloaderEmulator_CMD_ERASE_ROW_AVAIL)

            case BootloaderEmulator_COMMAND_ERASE:
                DBG_PUTS("Command: Erase") ;
                if ( BootloaderEmulator_COMMAND_ERASE ==
                     packetBuffer[BootloaderEmulator_CMD_ADDR] ) {
                    if ( (BootloaderEmulator_COMMUNICATION_STATE_ACTIVE ==
                          communicationState) && (pktSize == 3) ) {
                        /* Size of FLASH row (no ECC available) */
                        dataOffset = BootloaderEmulator_FROW_SIZE ;

                        (void) memset(dataBuffer, 0, (uint32) dataOffset) ;
                    }
                    else {
                        break ;
                    }
                }

#endif          /* (0 != BootloaderEmulator_CMD_ERASE_ROW_AVAIL) */

                if ( (BootloaderEmulator_COMMUNICATION_STATE_ACTIVE ==
                      communicationState) && (pktSize >= 3) ) {
                    /* The command may be sent along with the last block of
                      data, to program the row. */
                    (void) memcpy(&dataBuffer[dataOffset],
                                  &packetBuffer[BootloaderEmulator_DATA_ADDR +
                                                3],
                                  (uint32) pktSize - 3) ;

                    dataOffset += (pktSize - 3) ;

                    /* Size of FLASH row (no ECC available) */
                    pktSize = CY_FLASH_SIZEOF_ROW ;

                    /* Check if we have all data to program */
                    if ( dataOffset == pktSize ) {
                        uint16 row ;
                        //MZ uint8 counter;
                        uint32 size = CY_FLASH_SIZEOF_ROW ;

                        DBG_PRINTF("Have all the data to program.") ;

                        /* Save 1st bootloadable application flash row number to
                          the metadata in external memory */
                        if ( appSizeInRows == 0 ) {
                            /* Get Flash row number inside of the array */
                            dataOffset =
                                ( (uint16) ( (uint16) packetBuffer[
                                                 BootloaderEmulator_DATA_ADDR
                                                 + 2] <<
                                             8 ) ) |
                                packetBuffer[
                                    BootloaderEmulator_DATA_ADDR + 1] ;

                            /* btldrData  - holds flash array Id sent by host */
                            /* dataOffset - holds flash row Id sent by host   */
                            row =
                                (uint16) (btldrData *
                                          BootloaderEmulator_NUMBER_OF_ROWS_IN_ARRAY)
                                + dataOffset ;

                            /* Save number of the bootloadable application first
                              flash row */
                            appFirstRowNum = row ;
                            DBG_PRINTF(
                                "appSizeinRows = 0. appFirstRowNum = %d",
                                appFirstRowNum) ;
                        }

                        /* Erase metadata row */
                        if ( appSizeInRows == 0 ) {
                            /* Erase content of the external memory */
                            //MZ uint8  erase[CY_FLASH_SIZEOF_ROW] = {0};
//#if (DEBUG_UART_ENABLED == YES)
//                            uint8 tmp[CY_FLASH_SIZEOF_ROW] ;
//#endif                             /* #if (DEBUG_UART_ENABLED == YES) */
//
//#if (DEBUG_UART_ENABLED == YES)
//                            (void) EMI_ReadData(EMI_MD_BASE_ADDR,
//                                                CY_FLASH_SIZEOF_ROW,
//                                                tmp) ;
//#endif                             /* #if (DEBUG_UART_ENABLED == YES) */

//                            DBG_PUTS(
//                                "Metadata Row Before Erase: ") ;
//                            DBG_PRINT_ARRAY(tmp, CY_FLASH_SIZEOF_ROW) ;
//                            DBG_PUTS("") ;

                            /* Erase the entire external memory before writing.
                             */
                            DBG_PRINTF("Erasing metadata and entire app...") ;
                            CHECK( EMI_EraseAll() ) ;

//#if (DEBUG_UART_ENABLED == YES)
//                            (void) EMI_ReadData(EMI_MD_BASE_ADDR,
//                                                CY_FLASH_SIZEOF_ROW,
//                                                tmp) ;
//#endif                             /* #if (DEBUG_UART_ENABLED == YES) */
//                            DBG_PUTS("Metadata Row After Erase: ") ;
//                            DBG_PRINT_ARRAY(tmp, CY_FLASH_SIZEOF_ROW) ;
//                            DBG_PUTS("") ;
                        }

                        /* External memory application checksum calculation */
                        while ( size > 0 ) {
                            size-- ;
                            appExtMemChecksum += dataBuffer[size] ;
                        }
                        DBG_PRINTF("Calculated checksum of the app row: %x",
                                   appExtMemChecksum) ;

                        /* Write row to the external memory */
                        (void) EMI_WriteData(NOR_FLASH_INSTRUCTION_PP,
                                             EMI_APP_ABS_ADDR(appSizeInRows),
                                             CY_FLASH_SIZEOF_ROW,
                                             dataBuffer) ;
#if (0 == BootloaderEmulator_CMD_VERIFY_ROW_AVAIL)
                        DBG_PRINTF( "Verifico 0x%08X",
                                    EMI_APP_ABS_ADDR(appSizeInRows) ) ;
                        if ( EMI_verifica(EMI_APP_ABS_ADDR(appSizeInRows),
                                          CY_FLASH_SIZEOF_ROW, dataBuffer) ) {
                            appSizeInRows++ ;
                            ackCode = CYRET_SUCCESS ;
                        }
                        else {
                            ackCode = BootloaderEmulator_ERR_VERIFY ;
                        }
#else
                        // Arrivera' il comando di verifica
                        appSizeInRows++ ;
                        DBG_PRINTF( "EMI Address: 0x%04X",
                                    EMI_APP_ABS_ADDR(appSizeInRows - 1) ) ;

                        DBG_PRINTF("numOfTxedRows = 0x%02X", appSizeInRows - 1) ;

                        ackCode = CYRET_SUCCESS ;
#endif
                    }
                    else {
                        ackCode = BootloaderEmulator_ERR_LENGTH ;
                    }

                    dataOffset = 0 ;
                }
                break ;

                /***************************************************************************
                *   Sync bootloader
                ***************************************************************************/
#if (0 != BootloaderEmulator_CMD_SYNC_BOOTLOADER_AVAIL)

            case BootloaderEmulator_COMMAND_SYNC:

                DBG_PUTS("Command: Sync") ;
                if ( BootloaderEmulator_COMMUNICATION_STATE_ACTIVE ==
                     communicationState ) {
                    /* If something failed the host would send this command to
                      reset the bootloader. */
                    dataOffset = 0 ;

                    /* Don't acknowledge the packet, just get ready to accept
                      the next one */
                    continue ;
                }
                break ;

#endif              /* (0 != BootloaderEmulator_CMD_SYNC_BOOTLOADER_AVAIL) */

                /***************************************************************************
                *   Send data
                ***************************************************************************/
#if (0 != BootloaderEmulator_CMD_SEND_DATA_AVAIL)

            case BootloaderEmulator_COMMAND_DATA:
                DBG_PUTS("Command: Data. ") ;
                if ( BootloaderEmulator_COMMUNICATION_STATE_ACTIVE ==
                     communicationState ) {
                    /*  Make sure that dataOffset is valid before copying the
                      data */
                    if ( (dataOffset + pktSize) <=
                         BootloaderEmulator_SIZEOF_COMMAND_BUFFER ) {
                        //MZ uint16 i;

                        ackCode = CYRET_SUCCESS ;

#if (CY_PSOC3)
                        (void) memcpy(&dataBuffer[dataOffset],
                                      &packetBuffer[
                                          BootloaderEmulator_DATA_ADDR],
                                      ( int16 ) pktSize) ;
#else
                        (void) memcpy(&dataBuffer[dataOffset],
                                      &packetBuffer[
                                          BootloaderEmulator_DATA_ADDR],
                                      (uint32) pktSize) ;
#endif                              /* (CY_PSOC3) */

                        dataOffset += pktSize ;
                    }
                    else {
                        ackCode = BootloaderEmulator_ERR_LENGTH ;
                    }
                }

                break ;

#endif              /* (0 != BootloaderEmulator_CMD_SEND_DATA_AVAIL) */

            /***************************************************************************
            *   Enter bootloader
            ***************************************************************************/
            case BootloaderEmulator_COMMAND_ENTER:

                DBG_PUTS("Command: Enter. ") ;
                if ( pktSize == 0 ) {
                    uint32 i ;

                    BootloaderEmulator_ENTER CYDATA BtldrVersion = {
                        CYDEV_CHIP_JTAG_ID, CYDEV_CHIP_REV_EXPECT,
                        BootloaderEmulator_VERSION
                    } ;

                    communicationState =
                        BootloaderEmulator_COMMUNICATION_STATE_ACTIVE ;

                    rspSize = sizeof(BootloaderEmulator_ENTER) ;

                    (void) memcpy(&packetBuffer[BootloaderEmulator_DATA_ADDR],
                                  &BtldrVersion,
                                  (uint32) rspSize) ;

                    DBG_PRINTF("Initializing...") ;

                    /* Perform initializations */
                    appSizeInRows = 0 ;
                    appExtMemChecksum = 0 ;
                    for ( i = 0 ; i < CY_FLASH_SIZEOF_ROW ; i++ ) {
                        metadata[i] = 0 ;
                    }

#if (ENCRYPTION_ENABLED == YES)
                    /*Generate key*/
                    CR_GenerateKey(emiKey) ;
                    DBG_PUTS("Generated Key: ") ;
                    DBG_PRINT_ARRAY(emiKey, KEY_LENGTH) ;
                    DBG_PUTS("") ;
                    CR_WriteKey(emiKey) ;
                    CR_ReadKey(emiKey) ;
                    DBG_PUTS("Read Key     : ") ;
                    DBG_PRINT_ARRAY(emiKey, KEY_LENGTH) ;
                    DBG_PUTS("") ;
#endif                     /*(ENCRYPTION_ENABLED == YES)*/

                    ackCode = CYRET_SUCCESS ;
                }
                break ;

                /***************************************************************************
                *   Verify row
                ***************************************************************************/
#if (0 != BootloaderEmulator_CMD_VERIFY_ROW_AVAIL)

            case BootloaderEmulator_COMMAND_VERIFY:

                DBG_PUTS("Command: Verify") ;
                if ( (BootloaderEmulator_COMMUNICATION_STATE_ACTIVE ==
                      communicationState) && (pktSize == 3) ) {
                    uint8 CYDATA checksum ;

                    checksum =
                        BootloaderEmulator_Calc8BitSum(appSizeInRows - 1) ;

                    packetBuffer[BootloaderEmulator_DATA_ADDR] = (uint8) 1 +
                                                                 (uint8) (~
                                                                          checksum) ;
                    ackCode = CYRET_SUCCESS ;
                    rspSize = 1 ;

                    DBG_PUTS("") ;
                    DBG_PRINTF("\t\tRow Checksum: 0x%04X",
                               packetBuffer[BootloaderEmulator_DATA_ADDR]) ;
                }
                break ;

#endif             /* (0 != BootloaderEmulator_CMD_VERIFY_ROW_AVAIL) */

            /***************************************************************************
            *   Exit bootloader
            ***************************************************************************/
            case BootloaderEmulator_COMMAND_EXIT:

                DBG_PUTS("Command: Exit.") ;
                if ( CYRET_SUCCESS ==
                     BootloaderEmulator_ValidateBootloadable() ) {
                    BootloaderEmulator_SET_RUN_TYPE(
                        BootloaderEmulator_SCHEDULE_BTLDR) ;

                    DBG_PUTS("") ;
                    DBG_PUTS("Bootloadable Application is valid.") ;
                    metadata[EMI_MD_APP_STATUS_ADDR] = EMI_MD_APP_STATUS_VALID ;
                    metadata[EMI_MD_ENCRYPTION_STATUS_ADDR] =
                        ENCRYPTION_ENABLED ;
                }
                else {
                    DBG_PUTS("") ;
                    DBG_PUTS("Bootloadable Application is invalid.") ;
                    metadata[EMI_MD_APP_STATUS_ADDR] =
                        EMI_MD_APP_STATUS_INVALID ;
                }

                /* Update metadata section and save it to the external memory */

                metadata[EMI_MD_APP_SIZE_IN_ROWS_ADDR] =
                    LO8(appSizeInRows) ;
                metadata[EMI_MD_APP_SIZE_IN_ROWS_ADDR +
                         1] = HI8(appSizeInRows) ;
                metadata[EMI_MD_APP_FIRST_ROW_NUM_ADDR] = LO8(
                    appFirstRowNum) ;
                metadata[EMI_MD_APP_FIRST_ROW_NUM_ADDR + 1] = HI8(
                    appFirstRowNum) ;
                appExtMemChecksum = ( uint16 ) 1 +
                                    ( uint16 ) (~appExtMemChecksum) ;
                metadata[EMI_MD_APP_EM_CHECKSUM_ADDR] = LO8(
                    appExtMemChecksum) ;
                metadata[EMI_MD_APP_EM_CHECKSUM_ADDR + 1] = HI8(
                    appExtMemChecksum) ;
                metadata[EMI_MD_EXTERNAL_MEMORY_PAGE_SIZE_ADDR] = LO8(
                    EMI_EXTERNAL_MEMORY_PAGE_SIZE) ;
                metadata[EMI_MD_EXTERNAL_MEMORY_PAGE_SIZE_ADDR + 1] = HI8(
                    EMI_EXTERNAL_MEMORY_PAGE_SIZE) ;

                (void) EMI_WriteData(NOR_FLASH_INSTRUCTION_PP,
                                     EMI_MD_BASE_ADDR,
                                     CY_FLASH_SIZEOF_ROW,
                                     metadata) ;

                DBG_PRINTF("\t\tApplication Status: 0x%02X",
                           metadata[EMI_MD_APP_STATUS_ADDR]) ;

                DBG_PRINTF("\t\tSize Application in Rows: 0x%04X",
                           appSizeInRows) ;

                DBG_PRINTF("\t\tFirst Flash Row Number: 0x%04X", appFirstRowNum) ;

                DBG_PRINTF("\t\tExternal Memory Page Size: 0x%02X KB",
                           metadata[EMI_MD_EXTERNAL_MEMORY_PAGE_SIZE_ADDR]) ;

                DBG_PUTS(
                    "===============================================================================") ;
                DBG_PUTS(
                    "=    BLE_External_Memory_Bootloadable Application Performs Software Reset     =") ;
                DBG_PUTS(
                    "===============================================================================") ;
                DBG_PUTS("") ;
                DBG_PUTS("") ;

                CySoftwareReset() ;

                /* Will never get here */
                break ;

            /***************************************************************************
            *   Unsupported command
            ***************************************************************************/
            default:
                DBG_ERR ;
                ackCode = BootloaderEmulator_ERR_CMD ;
                break ;
            }
        }

        /* Reply with acknowledge or not acknowledge packet */
        (void) BootloaderEmulator_WritePacket(ackCode, packetBuffer, rspSize) ;

        // MZ: un comando alla volta
        break ;
    } while ( (0 == timeOut) ||
              (BootloaderEmulator_COMMUNICATION_STATE_ACTIVE ==
               communicationState) ) ;
}

/*******************************************************************************
* Function Name: BootloaderEmulator_WritePacket
********************************************************************************
*
* Summary:
*  Creates a bootloader response packet and transmits it back to the bootloader
*  host application over the already established communications protocol.
*
* Parameters:
*  status:
*      The status code to pass back as the second byte of the packet
*  buffer:
*      The buffer containing the data portion of the packet
*  size:
*      The number of bytes contained within the buffer to pass back
*
* Return:
*   CYRET_SUCCESS if successful. Any other non-zero value if failure occurred.
*
*******************************************************************************/
static cystatus BootloaderEmulator_WritePacket(
    uint8 status,
    uint8 buffer[],
    uint16 size)
{
    uint16 CYDATA checksum ;

    /* Start of packet. */
    buffer[BootloaderEmulator_SOP_ADDR] = BootloaderEmulator_SOP ;
    buffer[BootloaderEmulator_CMD_ADDR] = status ;
    buffer[BootloaderEmulator_SIZE_ADDR] = LO8(size) ;
    buffer[BootloaderEmulator_SIZE_ADDR + 1] = HI8(size) ;

    /* Compute checksum. */
    checksum = BootloaderEmulator_CalcPacketChecksum(
        buffer,
        size +
        BootloaderEmulator_DATA_ADDR) ;

    buffer[BootloaderEmulator_CHK_ADDR(size)] = LO8(checksum) ;
    buffer[BootloaderEmulator_CHK_ADDR(1 + size)] = HI8(checksum) ;
    buffer[BootloaderEmulator_EOP_ADDR(size)] = BootloaderEmulator_EOP ;
    DBG_PRINTF("notifico %s", (0 == status) ? "OK" : "ERR") ;

    /* Start packet transmit. */
    cystatus ris = CyBLE_CyBtldrCommWrite(
        buffer,
        size + BootloaderEmulator_MIN_PKT_SIZE,
        &size,
        150) ;
    if ( ris != CYRET_SUCCESS ) {
        DBG_PRINTF("? %08X = CyBLE_CyBtldrCommWrite ?", ris) ;
    }
    return ris ;
}

/* [] END OF FILE */
#else

void BootloaderEmulator_iniz(void)
{}

void BootloaderEmulator_HostLink(const uint8 x)
{
    UNUSED(x) ;
}

#endif
