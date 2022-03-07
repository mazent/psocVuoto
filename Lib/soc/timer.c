//#define STAMPA_DBG
#include "utili.h"
#include "timer.h"
#include "crit_sec.h"

// Evita di svegliare inutilmente da sleep
#define DISAB_TICK          1

/*
 * Il valore di un timer e' composto da:
 *      1		  servito == non attivo
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
static bool tick_attivo ;

typedef struct {
    uint32_t val ;
    PF_TIMER_SW cb ;
    void * arg ;
} UN_TIMER ;

static UN_TIMER lista_timer[MAX_TIMER_SW] ;

/*
 * I timer sono realizzate grazie al systick
 */

static void tick_isr(void)
{
    tick = true ;
}

static void disab_tick(void)
{
    CySysTickDisableInterrupt() ;
    CySysTickStop() ;

    tick_attivo = false ;
}

static void abil_tick(void)
{
    CySysTickClear() ;
    CySysTickEnable() ;

    tick_attivo = true ;
}

static void iniz_tick(void)
{
    // Inizializzo (tick = 1ms)
    CySysTickStart() ;

    // Fermo e modifico
    disab_tick() ;

    // L'interruzione non e' scattata
    tick = false ;

    // La servo io
    (void) CyIntSetSysVector(CY_INT_SYSTICK_IRQN, tick_isr) ;

#ifdef MILLI_PER_TICK
    // Cambio il periodo
    uint32_t rel = CySysTickGetReload() ;
    rel *= MILLI_PER_TICK ;
    CySysTickSetReload(rel) ;
#endif

#ifdef DISAB_TICK
#else
    abil_tick() ;
#endif
}

bool timer_attivi(void)
{
    bool attivi = false ;

    ENTER_CRITICAL_SECTION ;

    for ( int t = 0 ; t < MAX_TIMER_SW ; t++ ) {
        if ( lista_timer[t].val != TS_SERVITO ) {
            attivi = true ;
            break ;
        }
    }

    LEAVE_CRITICAL_SECTION ;

    return attivi ;
}

void timer_setcb(
    int quale,
    PF_TIMER_SW cb)
{
    ASSERT(quale < MAX_TIMER_SW) ;

    if ( quale < MAX_TIMER_SW ) {
        ENTER_CRITICAL_SECTION ;

        lista_timer[quale].cb = cb ;

        LEAVE_CRITICAL_SECTION ;
    }
}

void timer_start(
    int quale,
    uint32_t ms)
{
    timer_start_arg(quale, ms, NULL) ;
}

void timer_start_arg(
    int quale,
    uint32_t ms,
    void * v)
{
    ASSERT(quale < MAX_TIMER_SW) ;

    if ( quale >= MAX_TIMER_SW ) {
        DBG_ERR ;
    }
    else {
#ifdef MILLI_PER_TICK
        // Arrotondo
        uint32_t ttick = (ms + MILLI_PER_TICK - 1) / MILLI_PER_TICK ;
#else
        uint32_t ttick = ms ;
#endif
        if ( ttick > TS_SCADUTO ) {
            ttick = TS_SCADUTO ;
        }
        else if ( 0 == ttick ) {
            ttick = 1 ;
        }

        ENTER_CRITICAL_SECTION ;

        lista_timer[quale].arg = v ;
        lista_timer[quale].val = TS_SCADUTO - ttick ;
#ifdef DISAB_TICK
        if ( !tick_attivo ) {
            abil_tick() ;
        }
#endif
        LEAVE_CRITICAL_SECTION ;
    }
}

void timer_stop(int quale)
{
    ASSERT(quale < MAX_TIMER_SW) ;

    if ( quale < MAX_TIMER_SW ) {
        ENTER_CRITICAL_SECTION ;

        lista_timer[quale].val = TS_SERVITO ;

        LEAVE_CRITICAL_SECTION ;
    }
}

bool timer_running(int quale)
{
    bool esito = false ;

    ASSERT(quale < MAX_TIMER_SW) ;

    if ( quale < MAX_TIMER_SW ) {
        ENTER_CRITICAL_SECTION ;

        esito = lista_timer[quale].val != TS_SERVITO ;

        LEAVE_CRITICAL_SECTION ;
    }

    return esito ;
}

// La prima funzione da invocare

void timer_ini(void)
{
    for ( int t = 0 ; t < MAX_TIMER_SW ; t++ ) {
        lista_timer[t].val = TS_SERVITO ;
        lista_timer[t].cb = NULL ;
    }

    iniz_tick() ;
}

// Invocare periodicamente

void timer_run(void)
{
    if ( tick ) {
        // E' scattata l'interruzione
        PF_TIMER_SW funzioni[MAX_TIMER_SW] = {
            NULL
        } ;
        void * argomenti[MAX_TIMER_SW] = {
            NULL
        } ;

        tick = false ;

        // Aggiorno
        ENTER_CRITICAL_SECTION ;

        for ( int t = 0 ; t < MAX_TIMER_SW ; t++ ) {
            if ( lista_timer[t].val != TS_SERVITO ) {
                lista_timer[t].val++ ;

                if ( TS_SCADUTO == lista_timer[t].val ) {
                    lista_timer[t].val = TS_SERVITO ;

                    if ( lista_timer[t].cb ) {
                        funzioni[t] = lista_timer[t].cb ;
                        argomenti[t] = lista_timer[t].arg ;
                    }
                }
            }
        }

        LEAVE_CRITICAL_SECTION ;

        // Eseguo
        for ( int t = 0 ; t < MAX_TIMER_SW ; t++ ) {
            if ( funzioni[t] ) {
                DBG_PRINTF("eseguo timer %d\n", t) ;
                funzioni[t](argomenti[t]) ;
            }
        }

#ifdef DISAB_TICK
        // E adesso?
        if ( !timer_attivi() ) {
            disab_tick() ;
        }
#endif
    }
}
