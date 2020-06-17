#ifndef WDOG_H_
#define WDOG_H_

#include "utili/includimi.h"

// watchdog
// --------------------------

// Libera il cane
void WDOG_iniz(void) ;

// Invocare nel ciclo infinito del main
void WDOG_calcia(void) ;

// Timer "lenti"
// --------------------------
#include "wdtimer_cfg.h"

#define WDOG_UN_SECONDO		32768

// Callback invocata quando scade il tempo impostato
typedef void (* PF_WDTIMER_SW)(void) ;

// Uso dei timer
	// assegna al timer la sua callback (opzionale)
void WDOG_setcb(int, PF_WDTIMER_SW) ;
	// [ri]parte il timer
void WDOG_start(int, uint32_t secondi) ;
	// ferma il timer
void WDOG_stop(int) ;
	// vera se il timer e' attivo
bool WDOG_running(int) ;

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

uint32_t WDOG_now(void) ;

typedef struct {
	int secondi ;
	int milli ;
} WDOG_DURATA ;

static inline void WDOG_durata(WDOG_DURATA * d, uint32_t tick)
{
	float durata = tick ;

	durata /= WDOG_UN_SECONDO ;
	d->secondi = durata ;
	durata -= d->secondi ;
	// arrotondo
	durata += .0005 ;
	d->milli = durata * 1000.0 ;
}

#endif
