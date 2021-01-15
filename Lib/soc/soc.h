#ifndef SOC_H_
#define SOC_H_

/*
 * Sistema operativo cooperativo
 */

#include "utili/includimi.h"

#include "soc_cfg.h"

// Altri pezzi di soc
#include "timer.h"
#include "wdog.h"

typedef enum {
    E_CR_WDT,
    E_CR_PROTFAULT,
    E_CR_SW,
    E_CR_PON
} E_CAUSA_RESET ;

E_CAUSA_RESET SOC_causa(void) ;

// Utili per debug dei memory leak
void * soc_malloc(size_t) ;
void * soc_calloc(size_t num, size_t size) ;
void soc_free(void *) ;

// Asynchronous Procedure Call: esegue una procedura
// non appena possibile
typedef void (*PF_SOC_APC)(void *) ;

void SOC_apc_arg(int, PF_SOC_APC, void *) ;

static inline void SOC_apc(int a, PF_SOC_APC b)
{
    SOC_apc_arg(a, b, NULL) ;
}

bool SOC_apc_attiva(int) ;

// Power management della cpu
void SOC_min(RICH_CPU) ;

/*
 * Prima di invocarla occorre, se serve:
 *    mettere a riposo le uscite, okkio al pin 2[2]
 *    fermare il ble
 *        BLE_stop() ;
 *        CySysClkEcoStop() ;
 * Per usarla definire SOC_SPEGNI
 */
void SOC_spegni(void) ;

#endif
