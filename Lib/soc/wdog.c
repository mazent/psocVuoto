//#define STAMPA_DBG
#include "utili.h"

#include "wdog.h"

#if defined(CY_ISR_isr_wdog_H)

/*
 * Il ciclo infinito del main deve invocare WDOG_calcia
 * per azzerare i due contatori
 *
 * Se il sistema non gira piu', il secondo resetta
 *
 * Il terzo contatore del gruppo permette di ottenere dei timer lenti
 */

// Watchdog (counter 0 e 1)
// Due secondi - 1
#define WDT_COUNT0_MATCH    65535
// Moltiplicato questo:
#define WDT_COUNT1_MATCH    10

// Timer lenti (counter 2)
#define PERIODO         (1 << WDOG_TOGGLE_BIT)
#define PERIODO_S       (PERIODO / WDOG_UN_SECONDO)

#define UN_SECONDO_MS       1000

#if defined(WDOG_SW_ABIL) || (MAX_WDTIMER_SW > 0)

static void timer_lento_isr(void) ;

// Se si usa almeno uno dei due serve anche il timer 2

static void iniz_timer2(void)
{
    CySysWdtWriteMode(CY_SYS_WDT_COUNTER2, CY_SYS_WDT_MODE_INT) ;
    CySysWdtSetToggleBit(WDOG_TOGGLE_BIT) ;
}

static void abil_timer2(void)
{
    if ( 0 == CySysWdtGetEnabledStatus(2) ) {
        DBG_PUTS("Abilito wdog2") ;

        CySysWdtResetCounters(CY_SYS_WDT_COUNTER2_RESET) ;
        CySysWdtEnable(CY_SYS_WDT_COUNTER2_MASK) ;
    }
}

static void disa_timer2(void)
{
    DBG_PUTS("Disabilito wdog2") ;

    CySysWdtDisable(CY_SYS_WDT_COUNTER2_MASK) ;
}

#ifdef WDOG_ADVERTISEMENT

// Oltre al timer2 si usa anche l'advertisement

// vera se l'adv e' attivo
static volatile bool wa_adv = false ;
// vera se serve il timer lento
static bool wa_abil = false ;

void wdt_adv_inviato(void)
{
    // Solo fast
    static_assert(CYBLE_FAST_ADV_INT_MIN == CYBLE_FAST_ADV_INT_MAX, "OKKIO") ;
    static_assert(0 == CYBLE_FAST_ADV_TIMEOUT, "OKKIO") ;
    // Con lo stesso periodo del timer
    static_assert(ADV_INT_MS == (PERIODO_S * UN_SECONDO_MS), "OKKIO") ;

    wa_adv = true ;

    if ( 1 == CySysWdtGetEnabledStatus(2) ) {
        DBG_QUA ;
        disa_timer2() ;
    }

    if ( wa_abil ) {
        timer_lento_isr() ;
    }
}

void wdt_adv_fine(void)
{
    wa_adv = false ;

    if ( wa_abil ) {
        DBG_QUA ;
        abil_timer2() ;
    }
}

static void abil_lento(void)
{
    wa_abil = true ;

    if ( !wa_adv ) {
        abil_timer2() ;
    }
}

static void disa_lento(void)
{
    wa_abil = false ;

    if ( !wa_adv ) {
        disa_timer2() ;
    }
}

#else

#define abil_lento  abil_timer2
#define disa_lento  disa_timer2

#endif  // WDOG_ADVERTISEMENT

#else

// Il timer 2 non serve
static void iniz_timer2(void)
{}

#endif

#if WDOG_SW_ABIL
/*
 * Watchdog software
 */
static bool wds_run = false ;
static volatile uint16_t wds_s = 0 ;

#endif  // WDOG_SW_ABIL

#if MAX_WDTIMER_SW > 0

/*
 * Il valore di un timer e' composto da:
 *      1		  servito
 *       1		  scaduto
 *        ....... durata
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
    void * arg ;
} UN_TIMER ;

static UN_TIMER lista_timer[MAX_WDTIMER_SW] ;

uint32_t WDOG_tick_in_s(void)
{
    return PERIODO_S ;
}

void WDOG_setcb(
    int quale,
    PF_WDTIMER_SW cb)
{
    ASSERT(quale < MAX_WDTIMER_SW) ;
    DYN_ASSERT( 0 == __get_IPSR() ) ;

    if ( quale < MAX_WDTIMER_SW ) {
        lista_timer[quale].cb = cb ;
    }
}

void WDOG_start(
    int quale,
    uint32_t secondi)
{
    WDOG_start_arg(quale, secondi, NULL) ;
}

void WDOG_start_arg(
    int quale,
    uint32_t secondi,
    void * v)
{
    ASSERT(quale < MAX_WDTIMER_SW) ;
    ASSERT(secondi) ;
    DYN_ASSERT( 0 == __get_IPSR() ) ;

    if ( quale >= MAX_WDTIMER_SW ) {
        DBG_ERR ;
    }
    else {
        if ( 0 == secondi ) {
            DBG_ERR ;
            secondi = 1 ;
        }

        uint32_t ticks = (secondi + PERIODO_S - 1) / PERIODO_S ;

        if ( ticks > TS_SCADUTO ) {
            ticks = TS_SCADUTO ;
        }

        lista_timer[quale].arg = v ;
        lista_timer[quale].val = TS_SCADUTO - ticks ;

        abil_lento() ;
    }
}

static void wdt_spegni(void)
{
    int conta = 0 ;

    for ( int t = 0 ; t < MAX_WDTIMER_SW ; t++ ) {
        if ( lista_timer[t].val != TS_SERVITO ) {
            conta++ ;
        }
    }
#ifndef WDOG_SW_ABIL
    if ( 0 == conta ) {
#else
    if ( (0 == conta) && (!wds_run) ) {
#endif
        disa_lento() ;
    }
}

void WDOG_stop(int quale)
{
    DYN_ASSERT( 0 == __get_IPSR() ) ;
    ASSERT(quale < MAX_WDTIMER_SW) ;

    if ( quale < MAX_WDTIMER_SW ) {
        lista_timer[quale].val = TS_SERVITO ;
    }

    wdt_spegni() ;
}

bool WDOG_running(int quale)
{
    bool esito = false ;

    ASSERT(quale < MAX_WDTIMER_SW) ;
    DYN_ASSERT( 0 == __get_IPSR() ) ;

    if ( quale < MAX_WDTIMER_SW ) {
        esito = lista_timer[quale].val != TS_SERVITO ;
    }

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

#if defined(WDOG_SW_ABIL) || (MAX_WDTIMER_SW > 0)

__attribute__( (weak) )
void WDOG_wds_cb(void){}

static void timer_lento_isr(void)
{
#if MAX_WDTIMER_SW > 0
    // Avviso WDOG_calcia
    tick = true ;
#endif
#ifdef WDOG_SW_ABIL
    // Il wds viene gestito qui
    if ( !wds_run ) {}
    else if ( wds_s > PERIODO_S ) {
        wds_s -= PERIODO_S ;
    }
    else {
        wds_run = false ;

        // Avviso ...
        WDOG_wds_cb() ;

        // ... e resetto
        WDOG_reset() ;
    }
#endif
}

#endif

static void wdog_irq(void)
{
    uint32 intSource = CySysWdtGetInterruptSource() ;

    if ( (intSource & CY_SYS_WDT_COUNTER0_INT) == CY_SYS_WDT_COUNTER0_INT ) {
        DBG_PUTS("irq wdog0") ;

        CySysWdtClearInterrupt(CY_SYS_WDT_COUNTER0_INT) ;
    }

#if defined(WDOG_SW_ABIL) || (MAX_WDTIMER_SW > 0)
    if ( (intSource & CY_SYS_WDT_COUNTER2_INT) == CY_SYS_WDT_COUNTER2_INT ) {
        CySysWdtClearInterrupt(CY_SYS_WDT_COUNTER2_INT) ;

        timer_lento_isr() ;
    }
#endif

    isr_wdog_ClearPending() ;
}

void wdog_iniz(void)
{
    uint32_t msk = 0 ;

    isr_wdog_StartEx(wdog_irq) ;

#if MAX_WDTIMER_SW > 0
    for ( int t = 0 ; t < MAX_WDTIMER_SW ; t++ ) {
        lista_timer[t].val = TS_SERVITO ;
        lista_timer[t].cb = NULL ;
    }
#endif
    iniz_timer2() ;

#ifdef NDEBUG
    // Uno genera i tick ...
#ifdef WDOG_TICK
    CySysWdtWriteMode(CY_SYS_WDT_COUNTER0, CY_SYS_WDT_MODE_INT) ;
#else
    CySysWdtWriteMode(CY_SYS_WDT_COUNTER0, CY_SYS_WDT_MODE_NONE) ;
#endif
    CySysWdtWriteMatch(CY_SYS_WDT_COUNTER0, WDT_COUNT0_MATCH) ;
    CySysWdtWriteClearOnMatch(CY_SYS_WDT_COUNTER0, 1) ;
    CySysWdtWriteCascade(CY_SYS_WDT_CASCADE_01) ;

    // ... per l'altro che resetta
    CySysWdtWriteMode(CY_SYS_WDT_COUNTER1, CY_SYS_WDT_MODE_RESET) ;
    CySysWdtWriteMatch(CY_SYS_WDT_COUNTER1, WDT_COUNT1_MATCH) ;
    CySysWdtWriteClearOnMatch(CY_SYS_WDT_COUNTER1, 1) ;

    // Andiamo!
    msk |= CY_SYS_WDT_COUNTER0_MASK | CY_SYS_WDT_COUNTER1_MASK ;
#else
    // In debug niente reset
#endif
    if ( msk ) {
        CySysWdtEnable(msk) ;
    }
}

void WDOG_calcia(void)
{
    calcia() ;

#if MAX_WDTIMER_SW > 0
    if ( tick ) {
        tick = false ;

        // Aggiorno
        for ( int t = 0 ; t < MAX_WDTIMER_SW ; t++ ) {
            if ( lista_timer[t].val != TS_SERVITO ) {
                lista_timer[t].val++ ;
            }
        }

        // Eseguo
        for ( int t = 0 ; t < MAX_WDTIMER_SW ; t++ ) {
            if ( TS_SCADUTO == lista_timer[t].val ) {
                lista_timer[t].val = TS_SERVITO ;

                if ( lista_timer[t].cb ) {
                    //DBG_PRINTF("eseguo wdtimer %d\n", t) ;
                    lista_timer[t].cb(lista_timer[t].arg) ;
                }
            }
        }

        wdt_spegni() ;
    }
#endif
}

void WDOG_reset(void)
{
#ifdef NDEBUG
    CySysWdtDisable(CY_SYS_WDT_COUNTER0_MASK | CY_SYS_WDT_COUNTER1_MASK) ;

    DBG_PUTS(__func__) ;

    // [Ri]Programmo: 1 ms
    CySysWdtWriteMode(CY_SYS_WDT_COUNTER0, CY_SYS_WDT_MODE_NONE) ;
    CySysWdtWriteMatch(CY_SYS_WDT_COUNTER0, 32) ;
    CySysWdtWriteClearOnMatch(CY_SYS_WDT_COUNTER0, 1) ;
    CySysWdtWriteCascade(CY_SYS_WDT_CASCADE_01) ;

    CySysWdtWriteMode(CY_SYS_WDT_COUNTER1, CY_SYS_WDT_MODE_RESET) ;
    CySysWdtWriteMatch(CY_SYS_WDT_COUNTER1, 1) ;
    CySysWdtWriteClearOnMatch(CY_SYS_WDT_COUNTER1, 1) ;

    CySysWdtEnable(CY_SYS_WDT_COUNTER0_MASK | CY_SYS_WDT_COUNTER1_MASK) ;
#else
    DBG_PUTS(__func__) ;
    BPOINT ;
#endif
    while ( true ) {}
}

void WDOG_wds(uint16_t secondi)
{
#if WDOG_SW_ABIL
    if ( 0 == secondi ) {
        wds_run = false ;
#   if MAX_WDTIMER_SW > 0
        wdt_spegni() ;
#   else
        disa_lento() ;
#   endif
    }
    else {
        wds_s = secondi ;
        wds_run = true ;
        abil_lento() ;
    }
#else
    UNUSED(secondi) ;
#endif
}

#else

void wdog_iniz(void)
{}

void WDOG_calcia(void)
{}

uint32_t WDOG_tick_in_s(void)
{
    return 0 ;
}

void WDOG_setcb(
    int a,
    PF_WDTIMER_SW b)
{
    UNUSED(a) ;
    UNUSED(b) ;
    DBG_ERR ;
}

void WDOG_start(
    int a,
    uint32_t b)
{
    UNUSED(a) ;
    UNUSED(b) ;
    DBG_ERR ;
}

void WDOG_stop(int a)
{
    UNUSED(a) ;
    DBG_ERR ;
}

bool WDOG_running(int a)
{
    UNUSED(a) ;
    DBG_ERR ;
    return false ;
}

void WDOG_reset(void)
{
    DBG_ERR ;

    BPOINT ;
    while ( 1 ) {}
}

#endif
