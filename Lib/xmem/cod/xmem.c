//#define STAMPA_DBG
#include "xmem/xmem.h"
#include "xmem/at25sf041.h"

//#define PARANOID_ACTIVITY		1

#ifdef CY_PINS_SPI_CS_N_H


//// Aggiungo un margine al valore di tCE
//#define MAX_CHIP_ERASE_TIME_S 	(S25FL064L_CHIP_ERASE_TIME_S + 10)

// tPP = 1350 us + margine
#define MAX_PAGE_PROGRAMMING_MS	(AT25SF041_PAGE_PROGRAMMING_MS + 2)

////
//#define MAX_SECTOR_ERASE_TIME_MS 	(S25FL064L_SECTOR_ERASE_TIME_MS + 10)

#define MAX_BLOCK_ERASE_TIME_MS 	(AT25SF041_BLOCK_ERASE_TIME_MS + 100)


static bool iniz = false ;


#ifdef PARANOID_ACTIVITY
#	define is_erased(a, b)		XMEM_is_erased
#else
#	define is_erased(a, b)		true
#endif

bool XMEM_iniz(void)
{
    AT25_READ_ID rid = { 0 } ;

    DBG_PUTS(__func__) ;

    AT25_Read_Identification(&rid) ;

    if (AT25SF041_MANUF != rid.manuf)
        return false ;
    else {
        iniz = rid.device == AT25SF041_DEVICE ;

        return iniz ;
    }
}

//#define ABORT_ERASE		1

//bool XMEM_erase_sector(uint32_t addr)
//{
//	bool esito = false ;
//#ifdef ABORT_ERASE
//	static bool abort = false ;
//#endif
//
//	do {
//		if (!iniz) {
//			DBG_ERR ;
//			break ;
//		}
//
//		AT25_Write_Enable() ;
//
//		uint8_t sr1 = AT25_Read_Status_Register_1() ;
//		if (0 == (sr1 & SR1_WEL)) {
//			DBG_ERR ;
//			break ;
//		}
//
//		S25_Sector_Erase(addr) ;
//
//		int s ;
//#ifdef ABORT_ERASE
//		if (abort) {
//			abort = false ;
//			for (s=0 ; s<5 ; ++s) {
//				CyDelay(1) ;
//
//				uint8_t sr1 = AT25_Read_Status_Register_1() ;
//				if (0 == (sr1 & SR1_WIP))
//					break ;
//			}
//
//			if (5 == s) {
//				S25_Program_or_Erase_Suspend() ;
//				S25_Software_Reset() ;
//				break ;
//			}
//		}
//		else {
//			abort = true ;
//			for (s=0 ; s<MAX_SECTOR_ERASE_TIME_MS ; ++s) {
//				CyDelay(1) ;
//
//				uint8_t sr1 = AT25_Read_Status_Register_1() ;
//				if (0 == (sr1 & SR1_WIP))
//					break ;
//			}
//
//			if (MAX_SECTOR_ERASE_TIME_MS == s) {
//				S25_Program_or_Erase_Suspend() ;
//				S25_Software_Reset() ;
//				break ;
//			}
//		}
//#else
//		for (s=0 ; s<MAX_SECTOR_ERASE_TIME_MS ; ++s) {
//			CyDelay(1) ;
//
//			uint8_t sr1 = AT25_Read_Status_Register_1() ;
//			if (0 == (sr1 & SR1_WIP))
//				break ;
//		}
//
//		if (MAX_SECTOR_ERASE_TIME_MS == s) {
//            DBG_ERR ;
//			S25_Program_or_Erase_Suspend() ;
//			S25_Software_Reset() ;
//			break ;
//		}
//#endif
//
//		uint8_t sr2 = S25_Read_Status_Register_2() ;
//		if (1 == (sr2 & SR2_E_ERR)) {
//			// Errore
//            DBG_ERR ;
//			S25_Clear_Status_Register() ;
//			break ;
//		}
//
//		esito = is_erased(addr, S25FL064L_SECTOR_SIZE) ;
//
//	} while (false) ;
//
//	return esito ;
//}

bool XMEM_erase_block(uint32_t addr)
{
    bool esito = false ;

    DBG_PRINTF("\r\n%s(%08X)", __func__, addr) ;

    do {
        if (!iniz)
            break ;

        if(XMEM_is_erased(addr, AT25SF041_BLOCK_SIZE)) {
            esito = true;
            break;
        }
        DBG_PRINTF("\r\nErase sector: 0x%04X", addr);
        AT25_Write_Enable() ;

        uint8_t sr1 = AT25_Read_Status_Register_1() ;
        if (0 == (sr1 & SR1_WEL))
            break ;

        AT25_Block_Erase(addr) ;

        int s ;

        for (s=0 ; s<MAX_BLOCK_ERASE_TIME_MS ; ++s) {
            CyDelay(10) ;

            uint8_t sr1 = AT25_Read_Status_Register_1() ;
            if (0 == (sr1 & SR1_WIP))
                break ;
        }

        if (MAX_BLOCK_ERASE_TIME_MS == s) {
//			S25_Program_or_Erase_Suspend() ;
//			S25_Software_Reset() ;
            break ;
        }


//		uint8_t sr2 = S25_Read_Status_Register_2() ;
//		if (1 == (sr2 & SR2_E_ERR)) {
//			// Errore
//			S25_Clear_Status_Register() ;
//			break ;
//		}

        esito = XMEM_is_erased(addr, AT25SF041_BLOCK_SIZE) ;

    } while (false) ;

    return esito ;
}

void XMEM_erase_boot_flash(void)
{
    uint32_t addr = 0;
    XMEM_iniz();
    for(uint8_t i = 0; i < 4; i++) {
        XMEM_erase_block(addr);
        addr += AT25SF041_BLOCK_SIZE;
    }
}


//bool XMEM_erase_all(void)
//{
//	bool esito = false ;
//
//	do {
//		if (!iniz)
//			break ;
//
//		AT25_Write_Enable() ;
//
//		uint8_t sr1 = AT25_Read_Status_Register_1() ;
//		if (0 == (sr1 & SR1_WEL))
//			break ;
//
//		S25_Chip_Erase() ;
//
//		int s ;
//		for (s=0 ; s<MAX_CHIP_ERASE_TIME_S ; ++s) {
//			CyDelay(1000) ;
//
//			uint8_t sr1 = AT25_Read_Status_Register_1() ;
//			if (0 == (sr1 & SR1_WIP))
//				break ;
//		}
//
//		if (MAX_CHIP_ERASE_TIME_S == s) {
//			S25_Program_or_Erase_Suspend() ;
//			S25_Software_Reset() ;
//			break ;
//		}
//
//		uint8_t sr2 = S25_Read_Status_Register_2() ;
//		if (1 == (sr2 & SR2_E_ERR)) {
//			// Errore
//			S25_Clear_Status_Register() ;
//			break ;
//		}
//
//		esito = is_erased(0, AT25SF041_BLOCK_SIZE * S25FL064L_BLOCK_COUNT) ;
//
//	} while (false) ;
//
//	return esito ;
//}

bool XMEM_is_erased(uint32_t pos, size_t dim)
{
    bool esito = true ;

    uint8_t * buf = malloc(AT25SF041_WRITE_PAGE_BUFFER_SIZE) ;
    if (buf) {
        const size_t NUM_BUF = dim / AT25SF041_WRITE_PAGE_BUFFER_SIZE ;
        for (int i=0 ; i<NUM_BUF && esito ; ++i, pos+=AT25SF041_WRITE_PAGE_BUFFER_SIZE) {
            AT25_Read(pos, buf, AT25SF041_WRITE_PAGE_BUFFER_SIZE) ;
            for (int j=0 ; j<AT25SF041_WRITE_PAGE_BUFFER_SIZE ; ++j) {
                if (buf[i] != 0xFF) {
                    esito = false ;
                    break ;
                }
            }
        }
        free(buf) ;
    }

    return esito ;
}

bool XMEM_read(uint32_t addr, void * v, size_t dim)
{
    DBG_PRINTF("\r\n%s(%08X, %d)", __func__, addr, dim) ;

    if (!iniz) {
        DBG_ERR ;
        return false ;
    }

    AT25_Read(addr, v, dim) ;

    DBG_PRINT_HEX("\t", v, MIN(10, dim)) ;

    return true ;
}

bool XMEM_write(uint32_t addr, const void * v, size_t dim)
{
    bool esito = false ;

    DBG_PRINTF("\r\n%s(%08X, %d)", __func__, addr, dim) ;
    DBG_PRINT_HEX("\t", v, MIN(10, dim)) ;

    do {
        if (!iniz) {
            DBG_ERR ;
            break ;
        }

        AT25_Write_Enable() ;

        uint8_t sr1 = AT25_Read_Status_Register_1() ;
        if (0 == (sr1 & SR1_WEL)) {
            DBG_ERR ;
            break ;
        }

        if (dim != AT25_Write(addr, v, dim) ) {
            DBG_ERR ;
            break ;
        }

        int s ;
        for (s=0 ; s<MAX_PAGE_PROGRAMMING_MS ; ++s) {
            CyDelay(1) ;

            uint8_t sr1 = AT25_Read_Status_Register_1() ;
            if (0 == (sr1 & SR1_WIP))
                break ;
        }

        if (MAX_PAGE_PROGRAMMING_MS == s) {
//			S25_Program_or_Erase_Suspend() ;
//			S25_Software_Reset() ;
//
            DBG_ERR ;
            break ;
        }

//		uint8_t sr2 = S25_Read_Status_Register_2() ;
//		if (1 == (sr2 & SR2_P_ERR)) {
//			// Errore
//			S25_Clear_Status_Register() ;
//			DBG_ERR ;
//			break ;
//		}

        esito = true ;

#ifdef PARANOID_ACTIVITY
        // Controllo
        uint8_t * buf = malloc(dim) ;
        if (buf) {
            AT25_Read(addr, buf, dim) ;

            esito = 0 == memcmp(buf, v, dim) ;

            free(buf) ;
        }
        else {
            // se la malloc fallisce
            // la paranoia finisce
        }
#endif

    } while (false) ;

    return esito ;
}

#else

static uint8_t riga[AT25SF041_WRITE_PAGE_BUFFER_SIZE] ;

bool XMEM_iniz(void)
{
    DBG_PUTS(__func__) ;

    return true ;
}

bool XMEM_read(uint32_t a, void * v, size_t d)
{
    UNUSED(a) ;

    DBG_PUTS(__func__) ;

    memcpy(v, riga, d) ;

    return true ;
}

bool XMEM_write(uint32_t a, const void * v, size_t d)
{
    UNUSED(a) ;

    DBG_PUTS(__func__) ;

    memcpy(riga, v, d) ;

    return true ;
}

bool XMEM_erase_block(uint32_t a)
{
    UNUSED(a) ;

    DBG_PUTS(__func__) ;

    memset(riga, 0xFF, AT25SF041_WRITE_PAGE_BUFFER_SIZE) ;

    return true ;
}


#endif

#if 0

void XMEM_test(void)
{
    const size_t NUM_PAG = AT25SF041_SECTOR_COUNT * (AT25SF041_SECTOR_SIZE / AT25SF041_WRITE_PAGE_BUFFER_SIZE) ;
    uint8_t bufs[AT25SF041_WRITE_PAGE_BUFFER_SIZE] ;
    uint8_t bufl[AT25SF041_WRITE_PAGE_BUFFER_SIZE] ;

    {
        const size_t NUM_CAS = AT25SF041_WRITE_PAGE_BUFFER_SIZE / sizeof(int) ;
        uint8_t * tmp = bufs ;
        for (size_t i=0 ; i<NUM_CAS ; ++i, tmp += sizeof(int)) {
            int val = rand() ;
            memcpy(tmp, &val, sizeof(val)) ;
        }
    }

    while (true) {
        size_t pag = rand() % NUM_PAG ;
        uint32_t ind = pag * AT25SF041_WRITE_PAGE_BUFFER_SIZE ;

        DBG_PRINTF("pagina %d = %08X", pag, ind) ;

        DBG_PUTS("\tcancello") ;
        if ( !XMEM_erase_block(ind) ) {
            DBG_ERR ;
            continue ;
        }

        DBG_PUTS("\tscrivo") ;
        if ( !XMEM_write(ind, bufs, AT25SF041_WRITE_PAGE_BUFFER_SIZE) ) {
            DBG_ERR ;
            continue ;
        }

        DBG_PUTS("\tleggo") ;
        if ( !XMEM_read(ind, bufl, AT25SF041_WRITE_PAGE_BUFFER_SIZE) ) {
            DBG_ERR ;
            continue ;
        }

        DBG_PUTS("\tconfronto") ;
        if (0 != memcmp(bufs, bufl, AT25SF041_WRITE_PAGE_BUFFER_SIZE)) {
            DBG_ERR ;
        }

        DBG_PUTS("\taltro giro") ;
        for (size_t i=0 ; i<AT25SF041_WRITE_PAGE_BUFFER_SIZE ; ++i)
            bufs[i] += 1 ;
    }
}

#endif

