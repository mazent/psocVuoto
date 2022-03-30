#ifndef SOC_H_
#define SOC_H_

/*
 * Sistema operativo cooperativo
 *
 * Definendo globalmente:
 *      CY_BOOT_INT_DEFAULT_HANDLER_ENOMEM_EXCEPTION_CALLBACK
 *      CY_BOOT_INT_DEFAULT_HANDLER_EXCEPTION_ENTRY_CALLBACK
 * si sostituisce un ciclo infinito (nella isr predefinita)
 * con una chiamata a WDOG_reset
 */

#include <stdbool.h>
#include <stddef.h>

#include "soc_cfg.h"

// Altri pezzi di soc
#include "timer.h"
#include "wdog.h"

// Reset software
void SOC_reset(void) ;

typedef enum {
    E_CR_WDT,
    E_CR_PROTFAULT,
    E_CR_SW,
    E_CR_PON
} E_CAUSA_RESET ;

E_CAUSA_RESET SOC_causa(void) ;

// Vera se in una isr
bool SOC_in_isr(void) ;

// Memoria
void * SOC_malloc(size_t) ;
void * SOC_calloc(size_t, size_t) ;
void SOC_free(void *) ;

// Utili per debug dei memory leak
// per cui usate queste e non quelle sopra
#if defined(DBG_ABIL) && defined(SOC_DBG_MALLOC)
// Stampe abilitate
#   define soc_malloc(a)    \
    ({                      \
         void * v = SOC_malloc(a) ; \
         DBG_PRINTF("malloc %08X %d %s %d", v, a, __FILE__, __LINE__) ; \
         v ;                 \
     })

#   define soc_calloc(num, size) (          \
        {                                       \
            void * v = SOC_calloc(num, size) ;  \
            DBG_PRINTF("calloc %08X %d %s %d", v, num * size, __FILE__, \
                       __LINE__) ; \
            v ;                                 \
        })

#   define soc_free(a)          DBG_PRINTF("free %08X %s %d",   \
                                           a,                   \
                                           __FILE__,            \
                                           __LINE__) ; SOC_free(a)
#else
#   define soc_malloc        SOC_malloc
#   define soc_calloc        SOC_calloc
#   define soc_free          SOC_free
#endif

// Asynchronous Procedure Call: esegue una procedura
// non appena possibile (non da interruzione!)
typedef void (*PF_SOC_APC)(void *) ;

void SOC_apc_arg(int, PF_SOC_APC, void *) ;

static inline void SOC_apc(
    int a,
    PF_SOC_APC b)
{
    SOC_apc_arg(a, b, NULL) ;
}

bool SOC_apc_attiva(int) ;

// Interrupt Service Routine
typedef void (*PF_SOC_ISR)(void *) ;

void SOC_isr_arg(int, PF_SOC_ISR, void *) ;

static inline void SOC_isr(
    int a,
    PF_SOC_ISR b)
{
    SOC_isr_arg(a, b, NULL) ;
}

// Power management
typedef enum {
    CPU_ATTIVA = 0,
    CPU_PAUSA,
    CPU_FERMA
} RICH_CPU ;

// Power management della cpu (vedi MAX_SOC_CPU)
void SOC_min(int, RICH_CPU) ;

/*
 * Prima di invocarla occorre, se serve:
 *    mettere a riposo le uscite, okkio al pin 2[2]
 *    fermare il ble: BLE_stop()
 * Per usarla definire SOC_SPEGNI
 */
void SOC_spegni(void) ;

#endif
