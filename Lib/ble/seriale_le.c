#define STAMPA_DBG
#include "soc/utili.h"
#include "ble.h"

//#define STAMPA_ROBA     1

#if defined(SERIALE_LE_HANDLE_DATI) && defined(SERIALE_LE_HANDLE_CONFIG) && \
    defined(SERIALE_LE_HANDLE_CTRL)

// Se ci sono tutte la seriale LE e' abilitata

#ifdef ABIL_BLE_INDIC
static void cb_ble_indicate(bool bene){}
static bool nuovi_dati(void)
{
    static uint32_t msg = 0x33BF80D9 ;
    static S_BLE_IND ind = {
        .handle = SERIALE_LE_HANDLE_CTRL,
        .v = &msg,
        .dim = sizeof(msg),
        .cb = cb_ble_indicate,
        .to = 100
    } ;
    return BLE_indicate(&ind) ;
}

#endif

#if defined (ABIL_BLE_INDIC) && defined (ABIL_BLE_NOTIF)
static const uint16_t CONTROLLI = BLE_ABIL_NOTIF | BLE_ABIL_INDIC ;
#elif defined (ABIL_BLE_INDIC)
static const uint16_t CONTROLLI = BLE_ABIL_INDIC ;
#elif defined (ABIL_BLE_NOTIF)
static const uint16_t CONTROLLI = BLE_ABIL_NOTIF ;
static bool nuovi_dati(void)
{
    static uint32_t msg = 0x33BF80D9 ;
    BLE_NTF ntf = {
        .handle = SERIALE_LE_HANDLE_CTRL,
        .data = &msg,
        .dim = sizeof(msg),
        .to = 100
    } ;
    return BLE_notify(&ntf) ;
}

#else
#error MANCA NOTIF O INDIC
#endif
static uint16_t controlli = 0 ;

static size_t DIM_DATI = 0 ;
static uint8_t * dati = NULL ;

void sle_iniz(void)
{
    DIM_DATI = cyBle_gattDB[SERIALE_LE_HANDLE_DATI - 1]
               .attValue.attFormatValue.maxAttrLength ;
    dati = cyBle_gattDB[SERIALE_LE_HANDLE_DATI - 1]
           .attValue.attFormatValue.attGenericValLen->attGenericVal ;

    DBG_PRINTF("sle dati: %p[%d]", dati, DIM_DATI) ;
}

void sle_conn(bool si)
{
    if ( si ) {
        controlli = 0 ;
        memset(dati, 0, DIM_DATI) ;
    }
}

bool sle_write_data(
    const void * v,
    uint16_t dim)
{
    DBG_PRINTF("%s(%p, %d)", __func__, v, dim) ;
#ifdef STAMPA_ROBA
    DBG_PRINT_HEX("\t", v, dim) ;
#endif
    sle_ricevi_cb(v, dim) ;

    return true ;
}

bool sle_write_cfg(
    const void * v,
    uint16_t d)
{
    bool esito = false ;
#ifdef STAMPA_ROBA
    DBG_PRINT_HEX(__func__, v, d) ;
#endif
    do {
        if ( NULL == v ) {
            DBG_ERR ;
            break ;
        }
        if ( d != sizeof(uint16_t) ) {
            DBG_ERR ;
            break ;
        }
        memcpy(&controlli, v, d) ;
        esito = true ;
    } while ( false ) ;

    return esito ;
}

bool SLE_trasmetti(
    const void * v,
    size_t dim)
{
    bool esito = false ;

    if ( dim > DIM_DATI ) {
        DBG_ERR ;
    }
    else if ( controlli & CONTROLLI ) {
        // Azzero?
        if ( dim < DIM_DATI ) {
            memset(dati + dim, 0, DIM_DATI - dim) ;
        }

        // Aggiorno ...
        esito = BLE_scrivi_attr(SERIALE_LE_HANDLE_DATI, v, dim) ;

        // ... e avviso central
        if ( esito ) {
            esito = nuovi_dati() ;
        }
    }
    else {
        // central deve abilitare
        DBG_ERR ;
    }

    return esito ;
}

#endif
