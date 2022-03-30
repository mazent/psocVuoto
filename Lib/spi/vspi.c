#define STAMPA_DBG
#include "fspi.h"

#ifdef CY_SPIM_SPIM_H

/*
 * Ci deve essere uno "spi master full duplex"
 * chiamato SPIM
 * Lo "slave select" deve essere chiamato SS_1
 * e l'interruzione spim_rx
 */

//#define STAMPA_ROBA     1

static uint8_t * bufRx = NULL ;
static uint16_t dimRx = 0 ;
static bool ignora = true ;

static void rx_isr(void)
{
    while ( true ) {
        uint8_t stt = SPIM_GET_STATUS_RX(0xFF) ;
        //uint8_t stt = SPIM_RX_STATUS_REG;
        if ( 0 != (stt & SPIM_STS_RX_FIFO_NOT_EMPTY) ) {
            uint8_t rx = CY_GET_REG8(SPIM_RXDATA_PTR) ;
            //uint8_t rx = SPIM_RXDATA_REG;
            if ( ignora ) {
                ++dimRx ;
            }
            else if ( dimRx ) {
                *bufRx = rx ;
                bufRx += 1 ;
                dimRx -= 1 ;
            }
        }
        else {
            break ;
        }
    }
}

void FSPI_iniz(void)
{
    static bool iniz = true ;

    if ( iniz ) {
        SPIM_Start() ;
        spim_rx_StartEx(rx_isr) ;
        iniz = false ;
    }
}

void FSPI_sel(void)
{
    SS_1_Write(0) ;
    while ( 1 == SS_1_Read() ) {}
}

void FSPI_les(void)
{
    SS_1_Write(1) ;
    while ( 0 == SS_1_Read() ) {}
}

void FSPI_write(
    const void * v,
    size_t d)
{
#ifdef STAMPA_ROBA
    DBG_PRINT_HEX("SPI -> ", v, d) ;
#endif
    dimRx = 0 ;
    ignora = true ;
    const uint8_t * tx = v ;
    for ( size_t i = 0 ; i < d ; ++i ) {
        SPIM_WriteTxData(tx[i]) ;
    }
    while ( dimRx != d ) {}
}

void FSPI_read(
    void * v,
    size_t d)
{
    dimRx = d ;
    bufRx = v ;
    ignora = false ;
    for ( size_t i = 0 ; i < d ; ++i ) {
        SPIM_WriteTxData(0) ;
    }
    while ( dimRx ) {}
#ifdef STAMPA_ROBA
    DBG_PRINT_HEX("SPI <- ", v, d) ;
#endif
}

void FSPI_dummy_read(size_t d)
{
    dimRx = 0 ;
    ignora = true ;
    for ( size_t i = 0 ; i < d ; ++i ) {
        SPIM_WriteTxData(0) ;
    }
    while ( dimRx != d ) {}
}

typedef bool (*PF_FSPI_READ)(uint8_t) ;
void FSPI_read_cb(PF_FSPI_READ cb) {}

#endif
