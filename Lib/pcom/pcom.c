/*
 * Protocollo di comunicazione
 */

//#define STAMPA_DBG
#include "soc/utili.h"
#include "utili/crc1021.h"
#include "pcom.h"
#include "pcom_cfg.h"

static PCOM_MSG_CALLBACK cb_msg = NULL ;


static uint8_t msgTX[1 + 2 * (PCOM_DIM_MAX + 2) + 1] ;
static size_t dimTx = 0 ;

static uint8_t msgRx[PCOM_DIM_MAX + 2] ;
static size_t dimRx ;
static bool nega = false ;


static S_PCOM_RDT rdt = {
    .tx = msgTX
} ;

static void appendi(uint8_t x)
{
    if ( (PCOM_INIZIO_MSG == x) ||
         (PCOM_FINE_MSG == x) ||
         (PCOM_FUGA == x) ) {
        msgTX[dimTx++] = PCOM_FUGA ;
        msgTX[dimTx++] = NOT( x) ;
    }
    else {
        msgTX[dimTx++] = x ;
    }
}

static void da_capo(void)
{
    dimRx = 0 ;
    nega = false ;
}

void PCOM_esamina(
    void * v,
	size_t LETTI)
{
    uint8_t * dati = v ;

    for ( size_t i = 0 ; i < LETTI ; i++ ) {
        uint8_t rx = dati[i] ;
        if ( nega ) {
            rx = NOT(rx) ;

            switch ( rx ) {
            case PCOM_INIZIO_MSG:
            case PCOM_FINE_MSG:
            case PCOM_FUGA:
                // Solo questi sono ammessi
                if ( PCOM_DIM_MAX == dimRx ) {
                    // Non ci stanno
                    da_capo() ;
                }
                else {
                    msgRx[dimRx++] = rx ;
                    nega = false ;
                }
                break ;
            default:
                da_capo() ;
                break ;
            }
        }
        else if ( PCOM_INIZIO_MSG == rx ) {
            da_capo() ;
        }
        else if ( PCOM_FUGA == rx ) {
            nega = true ;
        }
        else if ( PCOM_FINE_MSG == rx ) {
            if ( dimRx < PCOM_DIM_MIN ) {}
            else if ( 0 == crc1021V(PCOM_CRC_INI, msgRx, dimRx) ) {
                // OK
                cb_msg(msgRx, dimRx - 2) ;
            }
            else {}

            // In ogni caso ricomincio
            da_capo() ;
        }
        else if ( PCOM_DIM_MAX == dimRx ) {
            // Non ci stanno
            da_capo() ;
        }
        else {
            msgRx[dimRx++] = rx ;
        }
    }
}

const S_PCOM_RDT * PCOM_rdt(
    const void * p,
	size_t dim)
{
    if ( NULL == p ) {
        return NULL ;
    }
    if ( 0 == dim ) {
        return NULL ;
    }
    if ( dim > PCOM_DIM_MAX ) {
        return NULL ;
    }

    uint16_t crc = crc1021V(PCOM_CRC_INI, p, dim) ;

    dimTx = 0 ;
    msgTX[dimTx++] = PCOM_INIZIO_MSG ;

    const uint8_t * roba = p ;
    for ( size_t j = 0 ; j < dim;j++ ) {
        appendi(roba[j]) ;
    }

    // crc
    roba = (uint8_t *) &crc ;
    appendi(roba[1]) ;
    appendi(roba[0]) ;

    msgTX[dimTx++] = PCOM_FINE_MSG ;

    // Finito
    rdt.dimTx = dimTx ;

    return &rdt ;
}

void PCOM_iniz(PCOM_MSG_CALLBACK cb)
{
    cb_msg = cb ;
    nega = false ;
}

