//#define STAMPA_DBG
#include "xmem/xmem.h"
#include "soc/soc.h"
#include "at25sf041.h"

//#define PARANOID_ACTIVITY		1

#define MAX_PAGE_PROGRAMMING_MS     (AT25SF041_PAGE_PROGRAMMING_MS + 2)
#define MAX_SECTOR_ERASE_TIME_MS    (AT25SF041_SECTOR_ERASE_TIME_MS + 10)
#define MAX_BLOCK_ERASE_TIME_MS     (AT25SF041_BLOCK_ERASE_TIME_MS + 100)
#define MAX_CHIP_ERASE_TIME_MS      (AT25SF041_CHIP_ERASE_TIME_S * 1000 + 100)

static bool iniz = false ;

#ifdef PARANOID_ACTIVITY
#   define is_erased(a, b)      XMEM_is_erased
#else
#   define is_erased(a, b)      true
#endif

void XMEM_accendi(void)
{
    AT25_accendi() ;
}

void XMEM_spegni(void)
{
    AT25_spegni() ;
}

bool XMEM_iniz(void)
{
    AT25_READ_ID rid = {
        0
    } ;

    DBG_PUTS(__func__) ;

    AT25_Read_Identification(&rid) ;

    if (AT25SF041_MANUF != rid.manuf) {
        return false ;
    }

    iniz = rid.device == AT25SF041_DEVICE ;

    return iniz ;
}

bool XMEM_erase_sector(uint32_t addr)
{
    bool esito = false ;

    DBG_PRINTF("%s(%08X)", __func__, addr) ;

    do {
        if (!iniz) {
            DBG_ERR ;
            break ;
        }

        AT25_Write_Enable() ;

        uint8_t sr1 = AT25_Read_Status_Register_1() ;
        if ( 0 == (sr1 & SR1_WEL) ) {
            DBG_ERR ;
            break ;
        }

        AT25_Sector_Erase(addr) ;

        const int PERIODO = 10 ;
        const int NUM_PERIODI = MAX_SECTOR_ERASE_TIME_MS / PERIODO ;
        int s ;
        for (s = 0 ; s < NUM_PERIODI ; ++s) {
            sr1 = AT25_Read_Status_Register_1() ;
            if ( 0 == (sr1 & SR1_WIP) ) {
                break ;
            }

            CyDelay(PERIODO) ;
        }

        if (0 == s) {
            // Troppo poco
            DBG_ERR ;
            break ;
        }

        if (NUM_PERIODI == s) {
            // Troppo
            DBG_ERR ;
            break ;
        }

        esito = is_erased(addr, AT25SF041_SECTOR_SIZE) ;
    } while (false) ;

    return esito ;
}

bool XMEM_erase_block(uint32_t addr)
{
    bool esito = false ;

    DBG_PRINTF("%s(%08X)", __func__, addr) ;

    do {
        if (!iniz) {
            break ;
        }

        AT25_Write_Enable() ;

        uint8_t sr1 = AT25_Read_Status_Register_1() ;
        if ( 0 == (sr1 & SR1_WEL) ) {
            break ;
        }

        AT25_Block_Erase(addr) ;

        const int PERIODO = 10 ;
        const int NUM_PERIODI = MAX_BLOCK_ERASE_TIME_MS / PERIODO ;
        int s ;
        for (s = 0 ; s < NUM_PERIODI ; ++s) {
            sr1 = AT25_Read_Status_Register_1() ;
            if ( 0 == (sr1 & SR1_WIP) ) {
                break ;
            }

            CyDelay(PERIODO) ;
        }

        if (0 == s) {
            // Troppo poco
            DBG_ERR ;
            break ;
        }

        if (NUM_PERIODI == s) {
            // Troppo
            DBG_ERR ;
            break ;
        }

        esito = is_erased(addr, AT25SF041_BLOCK_SIZE) ;
    } while (false) ;

    return esito ;
}

//bool XMEM_erase_chip(void)
//{
//    bool esito = false ;
//
//    DBG_PUTS(__func__) ;
//
//    do {
//        if (!iniz) {
//            break ;
//        }
//
//        AT25_Write_Enable() ;
//
//        uint8_t sr1 = AT25_Read_Status_Register_1() ;
//        if ( 0 == (sr1 & SR1_WEL) ) {
//            break ;
//        }
//
//        AT25_Chip_Erase();
//
//        const int PERIODO = 100;
//        const int NUM_PERIODI =MAX_CHIP_ERASE_TIME_MS/PERIODO;
//        int s ;
//        for (s = 0 ; s < NUM_PERIODI ; ++s) {
//            sr1 = AT25_Read_Status_Register_1() ;
//            if ( 0 == (sr1 & SR1_WIP) ) {
//                break ;
//            }
//
//            CyDelay(PERIODO) ;
//        }
//
//        if (0==s) {
//          // Troppo poco
//            DBG_ERR ;
//            break ;
//        }
//
//        if (NUM_PERIODI == s) {
//          // Troppo
//            DBG_ERR ;
//            break ;
//        }
//
//        esito = is_erased(addr, AT25SF041_BLOCK_SIZE) ;
//    } while (false) ;
//
//    return esito ;
//}

//void XMEM_erase_boot_flash(void)
//{
//    uint32_t addr = 0 ;
//    XMEM_iniz() ;
//    for (uint8_t i = 0 ; i < 4 ; i++) {
//        XMEM_erase_block(addr) ;
//        addr += AT25SF041_BLOCK_SIZE ;
//    }
//}

#if 0

bool XMEM_is_erased(uint32_t pos, size_t dim)
{
    bool esito = true ;

    uint8_t * buf = soc_malloc(AT25SF041_WRITE_PAGE_BUFFER_SIZE) ;
    if (buf) {
        const size_t NUM_BUF =
            (dim + AT25SF041_WRITE_PAGE_BUFFER_SIZE - 1)
            / AT25SF041_WRITE_PAGE_BUFFER_SIZE ;
        for (size_t i = 0 ; i < NUM_BUF && esito ;
             ++i, pos += AT25SF041_WRITE_PAGE_BUFFER_SIZE) {
            const size_t DIM = MIN(dim, AT25SF041_WRITE_PAGE_BUFFER_SIZE) ;
            AT25_Read(pos, buf, DIM) ;
            for (size_t j = 0 ; j < DIM ; ++j) {
                if (buf[j] != 0xFF) {
                    esito = false ;
                    break ;
                }
            }
        }
        soc_free(buf) ;
    }

    return esito ;
}

#else

// stesso tempo dell'altra con flash cancellata
bool XMEM_is_erased(uint32_t pos, size_t dim)
{
    return AT25_is_erased(pos, dim) ;
}

#endif

bool XMEM_read(uint32_t addr, void * v, size_t dim)
{
    DBG_PRINTF("%s(%08X, %d)", __func__, addr, dim) ;

    if (!iniz) {
        DBG_ERR ;
        return false ;
    }

    AT25_Read(addr, v, dim) ;

    DBG_PRINT_HEX( "\t", v, MIN(10, dim) ) ;

    return true ;
}

static size_t at25_write(uint32_t addr, const void * v, size_t dim)
{
    size_t scritti = 0 ;

    do {
        AT25_Write_Enable() ;

        uint8_t sr1 = AT25_Read_Status_Register_1() ;
        if ( 0 == (sr1 & SR1_WEL) ) {
            DBG_ERR ;
            break ;
        }

        scritti = AT25_Write(addr, v, dim) ;

        int s ;
        for (s = 0 ; s < MAX_PAGE_PROGRAMMING_MS ; ++s) {
            CyDelay(1) ;

            sr1 = AT25_Read_Status_Register_1() ;
            if ( 0 == (sr1 & SR1_WIP) ) {
                break ;
            }
        }

        if (MAX_PAGE_PROGRAMMING_MS == s) {
            DBG_ERR ;
            break ;
        }

#ifdef PARANOID_ACTIVITY
        // Controllo
        uint8_t * buf = soc_malloc(scritti) ;
        if (buf) {
            AT25_Read(addr, buf, scritti) ;

            esito = 0 == memcmp(buf, v, scritti) ;

            soc_free(buf) ;
        }
        else {
            // se la soc_malloc fallisce
            // la paranoia finisce
        }
#endif
    } while (false) ;

    return scritti ;
}

bool XMEM_write(uint32_t addr, const void * v, size_t dim)
{
    bool esito = false ;

    DBG_PRINTF("%s(%08X, %d)", __func__, addr, dim) ;
    DBG_PRINT_HEX( "\t", v, MIN(10, dim) ) ;

    do {
        if (!iniz) {
            DBG_ERR ;
            break ;
        }

        union {
            const void * v ;
            const uint8_t * u ;
        } u = {
            .v = v
        } ;

        while (dim) {
            size_t scritti = at25_write(addr, u.v, dim) ;
            if (0 == scritti) {
                break ;
            }
            dim -= scritti ;
            u.u += scritti ;
            addr += scritti ;
        }

        esito = 0 == dim ;
    } while (false) ;

    return esito ;
}

#if 0

void XMEM_test(void)
{
    const size_t NUM_PAG = AT25SF041_SECTOR_COUNT *
                           (AT25SF041_SECTOR_SIZE /
                            AT25SF041_WRITE_PAGE_BUFFER_SIZE) ;
    uint8_t bufs[AT25SF041_WRITE_PAGE_BUFFER_SIZE] ;
    uint8_t bufl[AT25SF041_WRITE_PAGE_BUFFER_SIZE] ;

    {
        const size_t NUM_CAS = AT25SF041_WRITE_PAGE_BUFFER_SIZE / sizeof(int) ;
        uint8_t * tmp = bufs ;
        for ( size_t i = 0 ; i < NUM_CAS ; ++i, tmp += sizeof(int) ) {
            int val = rand() ;
            memcpy( tmp, &val, sizeof(val) ) ;
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
        if ( 0 != memcmp(bufs, bufl, AT25SF041_WRITE_PAGE_BUFFER_SIZE) ) {
            DBG_ERR ;
        }

        DBG_PUTS("\taltro giro") ;
        for (size_t i = 0 ; i < AT25SF041_WRITE_PAGE_BUFFER_SIZE ; ++i) {
            bufs[i] += 1 ;
        }
    }
}

#endif
