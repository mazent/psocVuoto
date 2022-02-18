#define STAMPA_DBG
#include "utili/includimi.h"

#include "bl_tel.h"
#include "xmem/xmem.h"
#include "soc/soc.h"
#include "BootloaderEmulator_PVT.h"
#include "ExternalMemoryInterface.h"
#include "cif/cif.h"

// Se definita forza errore erase
//#define DBG_ERR_ERASE		1

// Se definita non scrive
//#define DBG_NO_SCRIVI       1

#define DIM_SHA         32

static uint16_t appSizeInRows ;
static uint16_t appFirstRowNum ;
static uint16_t appExtMemChecksum ;


bool BLT_iniz(void)
{
#ifdef DBG_ERR_ERASE
#warning OKKIO
    appSizeInRows = 0 ;
    appFirstRowNum = 0 ;
    appExtMemChecksum = 0 ;

    return false ;
#else
    uint32_t addr = 0 ;
    int i ;
    const int NUM_BLOCKS = (256 * 1024) / XMEM_BLOCK_SIZE ;

    appSizeInRows = 0 ;
    appFirstRowNum = 0 ;
    appExtMemChecksum = 0 ;

    DBG_PUTS("inizio erase") ;

    for ( i = 0 ; i < NUM_BLOCKS ; ++i, addr += XMEM_BLOCK_SIZE ) {
        WDOG_calcia() ;

        if ( !XMEM_erase_block(addr) ) {
            DBG_ERR ;
            break ;
        }
    }

    DBG_PUTS("fine erase") ;

    return NUM_BLOCKS == i ;
#endif
}

bool BLT_scrivi(
    uint8_t aid,
    uint16_t rn,
    const uint8_t * dati)
{
    if ( 0 == appSizeInRows ) {
        // Stessi nomi di BootloaderEmulator.c
        uint8_t btldrData = aid ;
        uint16_t dataOffset = rn ;

        appFirstRowNum =
            (uint16) (btldrData *
                      BootloaderEmulator_NUMBER_OF_ROWS_IN_ARRAY) +
            dataOffset ;
    }

    for ( size_t i = 0 ; i < CYDEV_FLS_ROW_SIZE ; ++i ) {
        appExtMemChecksum += dati[i] ;
    }

//    DBG_PRINTF("scrivo %d[%d]=%d", aid, rn, appSizeInRows) ;
    bool esito =
#ifdef DBG_NO_SCRIVI
        true ;
#warning OKKIO
#else
        XMEM_write(EMI_APP_ABS_ADDR(appSizeInRows),
                   dati,
                   CYDEV_FLS_ROW_SIZE) ;
#endif
    appSizeInRows++ ;

    return esito ;
}

#ifdef DBG_AGG_QUASIVERO
// Non serve
#else
static PF_VALIDA_RIGA pfVR = NULL ;

// cystatus BootloaderEmulator_ValidateBootloadable(void)
static bool blt_ValidateBootloadable(void)
{
    bool esito = false ;
    uint8_t appFlashRow[CY_FLASH_SIZEOF_ROW] ;

    DBG_PUTS(__func__) ;

    do {
        if ( 0 == appSizeInRows ) {
            DBG_ERR ;
            break ;
        }

        /* Get bootloadable application checksum from external memory */
        if ( !XMEM_read(EMI_APP_ABS_ADDR(appSizeInRows - 1), appFlashRow,
                        CY_FLASH_SIZEOF_ROW) ) {
            DBG_ERR ;
            break ;
        }

        uint8_t appChecksum = appFlashRow[BootloaderEmulator_MD_APP_CHECKSUM] ;
        uint8_t calcedChecksum = 0 ;
        uint32_t valid = 0 ;

        const uint32_t NUM_ROWS = appSizeInRows - 1 ;
        uint32_t row = 0 ;
        for ( ; row < NUM_ROWS ; row++ ) {
        	// MZ
        	WDOG_calcia();

            /* Read flash row data from external memory */
            if ( !XMEM_read(EMI_APP_ABS_ADDR(row), appFlashRow,
                            CY_FLASH_SIZEOF_ROW) ) {
                DBG_ERR ;
                break ;
            }

//            DBG_PRINTF("leggo %d", row) ;

            /* Calculate checksum of application row */
            for ( uint32_t i = 0 ; i < CY_FLASH_SIZEOF_ROW ; i++ ) {
                if ( (appFlashRow[i] != 0) && (appFlashRow[i] != 0xFF) ) {
                    valid = 1 ;
                }
                calcedChecksum += appFlashRow[i] ;
            }

            // Aggiungo all'hash
            if ( pfVR ) {
                (void) pfVR(appFlashRow) ;
            }
        }

        if ( row < NUM_ROWS ) {
            break ;
        }

        // BootloaderEmulator_ValidateBootloadable salta l'ultima riga
        // ma io devo aggiungerla ad sha
        if ( pfVR ) {
            if ( !XMEM_read(EMI_APP_ABS_ADDR(row), appFlashRow,
                            CY_FLASH_SIZEOF_ROW) ) {
                DBG_ERR ;
                break ;
            }
//            DBG_PRINTF("leggo %d", row) ;
            (void) pfVR(appFlashRow) ;
        }

        calcedChecksum = ( uint8_t ) 1 + ( uint8_t ) (~calcedChecksum) ;

        if ( (calcedChecksum != appChecksum) || (0 == valid) ) {
            DBG_ERR ;
            break ;
        }

        // Manca questo
        if ( pfVR ) {
            esito = pfVR(NULL) ;
        }
        else {
            esito = true ;
        }
    } while ( false ) ;

    return esito ;
}
#endif

// BootloaderEmulator_COMMAND_EXIT

bool BLT_esci(PF_VALIDA_RIGA vr)
{
#ifdef DBG_AGG_QUASIVERO
	return false;
#else
    uint8_t metadata[CY_FLASH_SIZEOF_ROW] ;

    pfVR = vr ;

    if ( blt_ValidateBootloadable() ) {
        BootloaderEmulator_SET_RUN_TYPE(BootloaderEmulator_SCHEDULE_BTLDR) ;

        metadata[EMI_MD_APP_STATUS_ADDR] = EMI_MD_APP_STATUS_VALID ;
        metadata[EMI_MD_ENCRYPTION_STATUS_ADDR] = ENCRYPTION_ENABLED ;
    }
    else {
        metadata[EMI_MD_APP_STATUS_ADDR] = EMI_MD_APP_STATUS_INVALID ;

        return false ;
    }

    /* Update metadata section and save it to the external memory */
    metadata[EMI_MD_APP_SIZE_IN_ROWS_ADDR] = LO8(appSizeInRows) ;
    metadata[EMI_MD_APP_SIZE_IN_ROWS_ADDR + 1] = HI8(appSizeInRows) ;

    metadata[EMI_MD_APP_FIRST_ROW_NUM_ADDR] = LO8(appFirstRowNum) ;
    metadata[EMI_MD_APP_FIRST_ROW_NUM_ADDR + 1] = HI8(appFirstRowNum) ;

    appExtMemChecksum = ( uint16_t ) 1 + ( uint16_t ) (~appExtMemChecksum) ;
    metadata[EMI_MD_APP_EM_CHECKSUM_ADDR] = LO8(appExtMemChecksum) ;
    metadata[EMI_MD_APP_EM_CHECKSUM_ADDR + 1] = HI8(appExtMemChecksum) ;

    metadata[EMI_MD_EXTERNAL_MEMORY_PAGE_SIZE_ADDR] =
        LO8(EMI_EXTERNAL_MEMORY_PAGE_SIZE) ;
    metadata[EMI_MD_EXTERNAL_MEMORY_PAGE_SIZE_ADDR + 1] =
        HI8(EMI_EXTERNAL_MEMORY_PAGE_SIZE) ;

    return XMEM_write(0,
                      metadata,
                      CYDEV_FLS_ROW_SIZE) ;
#endif
}

#ifndef CY_BOOTLOADABLE_Bootloadable_H

volatile uint32_t cyBtldrRunType ;

//cystatus BootloaderEmulator_ValidateBootloadable(void)
//{
//    return CYRET_SUCCESS ;
//}

#endif
