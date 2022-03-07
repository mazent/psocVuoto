#ifndef WDOG_H_
#define WDOG_H_

#include <stdbool.h>
#include <stdint.h>

#include "wdtimer_cfg.h"

// Watchdog
// --------------------------

// Invocata nel ciclo infinito del main ma,
// se serve, invocarla anche in altri loop
void WDOG_calcia(void) ;

// Causa un wdog reset
void WDOG_reset(void) ;

// Watchdog software (definire WDOG_SW_ABIL)
// --------------------------

// 0 ferma il wds
void WDOG_wds(uint16_t secondi) ;

// Timer "lenti"
// --------------------------

// Restituisce la risoluzione in secondi
uint32_t WDOG_tick_in_s(void) ;

// Callback invocata quando scade il tempo impostato
typedef void (*PF_WDTIMER_SW)(void *) ;

// assegna al timer la sua callback (opzionale)
void WDOG_setcb(int, PF_WDTIMER_SW) ;
// [ri]parte il timer
void WDOG_start(
    int,
    uint32_t secondi) ;
void WDOG_start_arg(int, uint32_t, void *) ;
// ferma il timer
void WDOG_stop(int) ;
// vera se il timer e' attivo
bool WDOG_running(int) ;

// Se WDOG_ADVERTISEMENT e' definito
// Invocare in BLE_CB::adv_inviato
void wdt_adv_inviato(void) ;
// Invocare alla fine dell'advertising
void wdt_adv_fine(void) ;

/*
    conteggio attuale globale

    usabile per calcolare la durata di una operazione

    esempio:
        WDOG_start(WDT_CRONO, molti secondi [p.e. 60 * 60]) ;

        uint32_t inizio = WDOG_now() ;

        operazione

        uint32_t fine = WDOG_now() ;
        WDOG_stop(WDT_CRONO) ;

        WDOG_DURATA d ;
        WDOG_durata(&d, fine - inizio) ;

        DBG_PRINTF("\tdurata = %d.%03d s\n", d.secondi, d.milli) ;
*/

#define WDOG_UN_SECONDO     32768

uint32_t WDOG_now(void) ;

typedef struct {
    int secondi ;
    int milli ;
} WDOG_DURATA ;

static inline void WDOG_durata(
    WDOG_DURATA * d,
    uint32_t tick)
{
    double durata = tick ;

    durata /= WDOG_UN_SECONDO ;
    d->secondi = (int) durata ;
    durata -= d->secondi ;
    // arrotondo
    durata += .0005 ;
    d->milli = (int) (durata * 1000.0) ;
}

#endif
