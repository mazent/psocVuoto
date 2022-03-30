#define STAMPA_DBG
#include "soc/utili.h"
#include "ufr/ufr.h"

static bool iniz = false ;

#ifdef UFR_E_BLE
// Mi basta scrivere dopo advertising ...
#   include "soc/soc.h"

#ifdef TIM_UFR_MS
// ... o voglio anche salvare periodicamente
#   define USA_TIMER
#endif

static void * vUFR[UFR_NUMR] = {
    NULL
} ;

static void ufr_flush(void * v)
{
    size_t i ;

    UNUSED(v) ;

    for ( i = 0 ; i < UFR_NUMR ; ++i ) {
        void * riga = vUFR[i] ;

        if ( NULL == riga ) {
            continue ;
        }

        if ( !ufr_writable() ) {
            break ;
        }

        if ( CY_SYS_FLASH_SUCCESS ==
             CySysSFlashWriteUserRow(i, riga) ) {
            soc_free(riga) ;
            vUFR[i] = NULL ;
        }
        else {
            DBG_ERR ;
        }
    }
#ifdef USA_TIMER
    if ( i < UFR_NUMR ) {
        timer_start(TIM_UFR, TIM_UFR_MS) ;
    }
#endif
}

static void prova_dopo(void)
{
#ifdef USA_TIMER
    if ( !timer_running(TIM_UFR) ) {
        timer_start(TIM_UFR, TIM_UFR_MS) ;
    }
#endif
}

#endif

bool UFR_iniz(void)
{
    static_assert(UFR_DIM == CY_SFLASH_SIZEOF_USERROW, "OKKIO!") ;
    static_assert(UFR_NUMR == CY_SFLASH_NUMBER_USERROWS, "OKKIO!") ;

    ASSERT(!iniz) ;

    iniz = (UFR_DIM == CY_SFLASH_SIZEOF_USERROW) &&
           (UFR_NUMR == CY_SFLASH_NUMBER_USERROWS) ;

#ifdef USA_TIMER
    timer_setcb(TIM_UFR, ufr_flush) ;
#endif
    return iniz ;
}

const void * UFR_dati(uint8_t numr)
{
    const void * riga = NULL ;

    ASSERT(iniz) ;

    if ( !iniz ) {    // NOLINT(bugprone-branch-clone)
        DBG_ERR ;
    }
    else if ( numr >= UFR_NUMR ) {
        DBG_ERR ;
    }
#ifdef UFR_E_BLE
    else if ( vUFR[numr] ) {
        riga = vUFR[numr] ;
    }
#endif
    else {
        riga = CPOINTER(CY_SFLASH_USERBASE + numr * UFR_DIM) ;
    }

    return riga ;
}

bool UFR_read(
    uint8_t numr,
    void * v)
{
    bool esito = false ;

    ASSERT(iniz) ;

    if ( !iniz ) {    // NOLINT(bugprone-branch-clone)
        DBG_ERR ;
    }
    else if ( numr >= UFR_NUMR ) {
        DBG_ERR ;
    }
    else if ( NULL == v ) {
        DBG_ERR ;
    }
#ifdef UFR_E_BLE
    else if ( vUFR[numr] ) {
        memcpy(v, vUFR[numr], UFR_DIM) ;
        esito = true ;
    }
#endif
    else {
        void * r = POINTER(CY_SFLASH_USERBASE + numr * UFR_DIM) ;
        memcpy(v, r, UFR_DIM) ;

        esito = true ;
    }

    return esito ;
}

bool UFR_write(
    uint8_t numr,
    const void * v)
{
    bool esito = false ;

    ASSERT(iniz) ;

    if ( !iniz ) {    // NOLINT(bugprone-branch-clone)
        DBG_ERR ;
    }
    else if ( numr >= UFR_NUMR ) {
        DBG_ERR ;
    }
    else if ( NULL == v ) {
        DBG_ERR ;
    }
#ifdef UFR_E_BLE
    else {
        if ( vUFR[numr] ) {
            memcpy(vUFR[numr], v, UFR_DIM) ;
            esito = true ;
        }
        else {
            void * riga = soc_malloc(UFR_DIM) ;
            if ( NULL == riga ) {
                DBG_ERR ;
            }
            else {
                memcpy(riga, v, UFR_DIM) ;
                vUFR[numr] = riga ;
                esito = true ;
            }
        }

        if ( esito && ufr_writable() ) {
            if ( CY_SYS_FLASH_SUCCESS ==
                 CySysSFlashWriteUserRow(numr, vUFR[numr]) ) {
                soc_free(vUFR[numr]) ;
                vUFR[numr] = NULL ;
            }
            else {
                DBG_ERR ;

                // Riprovo
                prova_dopo() ;
            }
        }
        else {
            // Aspetto
            prova_dopo() ;
        }
    }
#else
    else {
        esito = CY_SYS_FLASH_SUCCESS ==
                CySysSFlashWriteUserRow(numr, (const uint8 *) v) ;
        if ( !esito ) {
            DBG_ERR ;
        }
    }
#endif
    return esito ;
}

#ifdef UFR_E_BLE

void UFR_flush(void)
{
#ifdef USA_TIMER
    if ( timer_running(TIM_UFR) ) {
        // C'e' almeno una scrittura in sospeso
        ufr_flush(NULL) ;

        /*
         * se c'e' ancora qlc in sospeso
         *     ufr_flush ha fatto ripartire il timer
         * altrimenti
         *     quando scatta il timer la ufr_flush non lo attiva
         */
    }
#else
    ufr_flush(NULL) ;
#endif
}

bool UFR_flushed(void)
{
    bool esito = true ;

    for ( size_t i = 0 ; i < UFR_NUMR ; ++i ) {
        if ( NULL != vUFR[i] ) {
            esito = false ;
            break ;
        }
    }

    return esito ;
}

#endif
