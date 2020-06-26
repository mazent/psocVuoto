//#define STAMPA_DBG
#include "soc.h"
#include "timer.h"

// Evita di svegliare inutilmente da sleep
#define DISAB_TICK

/*
 * Il valore di un timer e' composto da:
 * 		1		  servito
 * 		 1		  scaduto
 * 		  ....... durata
 *
 * La durata vale (TS_SCADUTO - numero-tick)
 * Man mano che vengono contati i tick, la durata
 * aumenta fino a TS_SCADUTO
 */

static const uint32_t TS_SERVITO = 1 << 31 ;
static const uint32_t TS_SCADUTO = 1 << 30 ;

static volatile bool tick ;
static bool tick_attivo ;


typedef struct {
	uint32_t val ;
	PF_TIMER_SW cb ;
	void * arg ;
} UN_TIMER ;

static UN_TIMER lista_timer[MAX_TIMER_SW] ;

static void tick_isr(void)
{
	tick = true ;
}

void timer_setcb(int quale, PF_TIMER_SW cb)
{
	ASSERT(quale < MAX_TIMER_SW) ;

	if (quale < MAX_TIMER_SW)
		lista_timer[quale].cb = cb ;
}

void timer_start(int quale, uint32_t ms)
{
	timer_start_arg(quale, ms, NULL) ;
}

void timer_start_arg(int quale, uint32_t ms, void * v)
{
	ASSERT(quale < MAX_TIMER_SW) ;
	ASSERT(ms) ;

	if (quale >= MAX_TIMER_SW) {
		DBG_ERR ;
	}
	else {
		if (0 == ms) {
			DBG_ERR ;
			ms = 1 ;
		}

#ifdef MILLI_PER_TICK
		// Arrotondo
		uint32_t ttick = (ms + MILLI_PER_TICK - 1) / MILLI_PER_TICK ;
#else
		uint32_t ttick = ms ;
#endif
		if (ttick > TS_SCADUTO)
			ttick = TS_SCADUTO ;

		lista_timer[quale].arg = v ;
		lista_timer[quale].val = TS_SCADUTO - ttick ;
#ifdef DISAB_TICK
		if (!tick_attivo) {
			DBG_PUTS("Abilito tick") ;
			CySysTickClear() ;
			CySysTickEnable() ;

			tick_attivo = true ;
		}
#endif
	}
}

void timer_stop(int quale)
{
	ASSERT(quale < MAX_TIMER_SW) ;

	if (quale < MAX_TIMER_SW)
		lista_timer[quale].val = TS_SERVITO ;
}

bool timer_running(int quale)
{
	bool esito = false ;

	ASSERT(quale < MAX_TIMER_SW) ;

	if (quale < MAX_TIMER_SW)
		esito = lista_timer[quale].val != TS_SERVITO ;

	return esito ;
}

// La prima funzione da invocare

void timer_ini(void)
{
    for (int t = 0 ; t < MAX_TIMER_SW ; t++) {
    	lista_timer[t].val = TS_SERVITO ;
    	lista_timer[t].cb = NULL ;
    }

	// Inizializzo (tick = 1ms)
	CySysTickStart() ;

	// Fermo e modifico
	CySysTickStop() ;

	// Comunque uso il mio
	(void) CyIntSetSysVector(CY_INT_SYSTICK_IRQN, tick_isr) ;

	// L'interruzione non e' scattata
	tick = false ;

#ifdef MILLI_PER_TICK
	// Cambio il periodo
	uint32_t rel = CySysTickGetReload() ;
	rel *= MILLI_PER_TICK ;
	CySysTickSetReload(rel) ;
#endif

#ifdef DISAB_TICK
	// Per ora non mi serve il tick
	tick_attivo = false ;
#else
	CySysTickClear() ;
	CySysTickEnable() ;

	tick_attivo = true ;
#endif
}

// Se cambia il clock

void timer_reini(void)
{
#ifdef MILLI_PER_TICK
	// Cambio il periodo
	uint32_t rel = CySysTickGetReload() ;
	rel *= MILLI_PER_TICK ;
	CySysTickSetReload(rel) ;
#endif

	if (tick_attivo) {
		CySysTickClear() ;
		CySysTickEnable() ;
	}
}

// Invocare periodicamente

void timer_run(void)
{
	if (tick) {
		tick = false ;

		// Aggiorno
		for (int t = 0 ; t < MAX_TIMER_SW ; t++) {
			if (lista_timer[t].val != TS_SERVITO) {
				CyGlobalIntDisable ;

				lista_timer[t].val++ ;

				CyGlobalIntEnable ;
			}
		}

		// Eseguo
		for (int t = 0 ; t < MAX_TIMER_SW ; t++) {
			if (TS_SCADUTO == lista_timer[t].val) {
				CyGlobalIntDisable ;

				PF_TIMER_SW cb = lista_timer[t].cb ;
				void * arg = lista_timer[t].arg ;
				lista_timer[t].val = TS_SERVITO ;

				CyGlobalIntEnable ;
				if (cb) {
					DBG_PRINTF("eseguo timer %d\n", t) ;
					cb(arg) ;
				}
			}
		}
#ifdef DISAB_TICK
		// E adesso?
		int conta = 0 ;
		for (int t = 0 ; t < MAX_TIMER_SW ; t++) {
			if (lista_timer[t].val != TS_SERVITO)
				conta++ ;
		}
		if (0 == conta) {
			DBG_PUTS("Disabilito tick") ;
			CySysTickStop() ;
			tick_attivo = false ;
		}
#endif
	}
}

bool timer_attivi(void)
{
#ifdef DISAB_TICK
	return tick_attivo || tick ;
#else
	bool attivi = false ;

	for (int t = 0 ; t < MAX_TIMER_SW ; t++) {
		if (lista_timer[t].val != TS_SERVITO) {
			attivi = true ;
			break ;
		}
	}

	return attivi ;
#endif
}
