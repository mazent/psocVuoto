//#define STAMPA_DBG
#include "wdog.h"

#if defined(CY_ISR_isr_wdog_H)

/*
 * Il ciclo infinito del main deve invocare WDOG_calcia
 * per azzerare i due contatori
 *
 * Quando si passa in sleep/deep-sleep, il primo contatore genera una
 * interruzione e risveglia, per cui il ciclo infinito azzera i conteggi
 *
 * Se il sistema non gira piu', il secondo resetta
 *
 * Il terzo contatore del gruppo permette di ottenere dei timer lenti
 */

// Watchdog (counter 0 e 1)
	// Due secondi - 1
#define WDT_COUNT0_MATCH    65535
	// Dieci secondi
#define WDT_COUNT1_MATCH    5

// Timer lenti (counter 2)
// L'interruzione capita quando commuta il bit
	// 32768 ...
#define TOGGLE_BIT		15
	// ... un secondo
#define PERIODO 		(1 << TOGGLE_BIT)
	// ... 1
#define PERIODO_S		(PERIODO / WDOG_UN_SECONDO)


#if MAX_WDTIMER_SW > 0

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

typedef struct {
	uint32_t val ;
	PF_WDTIMER_SW cb ;
} UN_TIMER ;

static UN_TIMER lista_timer[MAX_WDTIMER_SW] ;

void WDOG_setcb(int quale, PF_WDTIMER_SW cb)
{
	ASSERT(quale < MAX_WDTIMER_SW) ;

	if (quale < MAX_WDTIMER_SW)
		lista_timer[quale].cb = cb ;
}

void WDOG_start(int quale, uint32_t secondi)
{
	ASSERT(quale < MAX_WDTIMER_SW) ;
	ASSERT(secondi) ;

	if (0 == secondi) {
	}
	else if (quale >= MAX_WDTIMER_SW) {
	}
	else {
		uint32_t ttick = (secondi + PERIODO_S - 1) / PERIODO_S ;

		if (ttick > TS_SCADUTO)
			ttick = TS_SCADUTO ;

		lista_timer[quale].val = TS_SCADUTO - ttick ;

		if (0 == CySysWdtGetEnabledStatus(2)) {
			DBG_PUTS("Abilito wdog2") ;

			CySysWdtResetCounters(CY_SYS_WDT_COUNTER2_RESET) ;
			CySysWdtEnable(CY_SYS_WDT_COUNTER2_MASK) ;
		}
	}
}

static void wdt_spegni(void)
{
	int conta = 0 ;

	for (int t = 0 ; t < MAX_WDTIMER_SW ; t++) {
		if (lista_timer[t].val != TS_SERVITO)
			conta++ ;
	}

	if (0 == conta) {
		DBG_PUTS("Disabilito wd2") ;

		CySysWdtDisable(CY_SYS_WDT_COUNTER2_MASK) ;
	}
}

void WDOG_stop(int quale)
{
	ASSERT(quale < MAX_WDTIMER_SW) ;

	if (quale < MAX_WDTIMER_SW)
		lista_timer[quale].val = TS_SERVITO ;

	wdt_spegni() ;
}

bool WDOG_running(int quale)
{
	bool esito = false ;

	ASSERT(quale < MAX_WDTIMER_SW) ;

	if (quale < MAX_WDTIMER_SW)
		esito = lista_timer[quale].val != TS_SERVITO ;

	return esito ;
}

uint32_t WDOG_now(void)
{
	return CySysWdtGetCount(2) ;
}

#endif


static void calcia(void)
{
	CySysWdtResetCounters(CY_SYS_WDT_COUNTER0_RESET | CY_SYS_WDT_COUNTER1_RESET) ;
}

static void wdog_irq(void)
{
	uint32 intSource = CySysWdtGetInterruptSource() ;

	if ((intSource & CY_SYS_WDT_COUNTER0_INT) == CY_SYS_WDT_COUNTER0_INT) {
		CySysWdtClearInterrupt(CY_SYS_WDT_COUNTER0_INT);
	}
#if MAX_WDTIMER_SW > 0
	if ((intSource & CY_SYS_WDT_COUNTER2_INT) == CY_SYS_WDT_COUNTER2_INT) {
		CySysWdtClearInterrupt(CY_SYS_WDT_COUNTER2_INT) ;
		tick = true ;
	}
#endif

	isr_wdog_ClearPending();
}

void WDOG_iniz(void)
{
	uint32_t msk = 0 ;

	isr_wdog_StartEx(wdog_irq) ;

#if MAX_WDTIMER_SW > 0
    for (int t = 0 ; t < MAX_WDTIMER_SW ; t++) {
    	lista_timer[t].val = TS_SERVITO ;
    	lista_timer[t].cb = NULL ;
    }

    // Se si usano i timer lenti serve l'interruzione ...
    msk = CY_SYS_WDT_COUNTER2_MASK ;
	CySysWdtWriteMode(CY_SYS_WDT_COUNTER2, CY_SYS_WDT_MODE_INT) ;
	CySysWdtSetToggleBit(TOGGLE_BIT) ;
#endif

	// Uno genera i tick ...
	// Non funziona: CySysWdtWriteMode(CY_SYS_WDT_COUNTER0, CY_SYS_WDT_MODE_NONE) ;
	// ... ma per farla scattare occorre abilitare anche questa
	CySysWdtWriteMode(CY_SYS_WDT_COUNTER0, CY_SYS_WDT_MODE_INT) ;
	CySysWdtWriteMatch(CY_SYS_WDT_COUNTER0, WDT_COUNT0_MATCH) ;
	CySysWdtWriteClearOnMatch(CY_SYS_WDT_COUNTER0, 1u) ;
	msk |= CY_SYS_WDT_COUNTER0_MASK ;
#ifdef NDEBUG
	CySysWdtWriteCascade(CY_SYS_WDT_CASCADE_01) ;

	// ... per l'altro che resetta
	CySysWdtWriteMode(CY_SYS_WDT_COUNTER1, CY_SYS_WDT_MODE_RESET) ;
	CySysWdtWriteMatch(CY_SYS_WDT_COUNTER1, WDT_COUNT1_MATCH);
    CySysWdtWriteClearOnMatch(CY_SYS_WDT_COUNTER1, 1u);

    // Andiamo!
    msk |= CY_SYS_WDT_COUNTER1_MASK ;
#else
	// In debug niente reset
#endif
    if (msk)
    	CySysWdtEnable(msk);
}


void WDOG_calcia(void)
{
	calcia() ;

#if MAX_WDTIMER_SW > 0
	if (tick) {
		tick = false ;

		// Aggiorno
		for (int t = 0 ; t < MAX_WDTIMER_SW ; t++) {
			if (lista_timer[t].val != TS_SERVITO)
				lista_timer[t].val++ ;
		}

		// Eseguo
		for (int t = 0 ; t < MAX_WDTIMER_SW ; t++) {
			if (TS_SCADUTO == lista_timer[t].val) {
				lista_timer[t].val = TS_SERVITO ;

				if (lista_timer[t].cb) {
					DBG_PRINTF("eseguo wdtimer %d\n", t) ;
					lista_timer[t].cb() ;
				}
			}
		}

		wdt_spegni() ;
	}
#endif
}

#else

void WDOG_iniz(void) {}
void WDOG_calcia(void) {}
void WDOG_setcb(int a, PF_WDTIMER_SW b)
{
	UNUSED(a) ;
	UNUSED(b) ;
}
void WDOG_start(int a, uint32_t b)
{
	UNUSED(a) ;
	UNUSED(b) ;
}
void WDOG_stop(int a)
{
	UNUSED(a) ;
}
bool WDOG_running(int a)
{
	UNUSED(a) ;
	return false ;
}

#endif
