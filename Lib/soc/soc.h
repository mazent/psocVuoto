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

// Utile?
#define SILICON_ID    (*(reg32 *) CYREG_SFLASH_SILICON_ID)


void SOC_ini(void) ;

void * soc_malloc(size_t) ;
void * soc_calloc(size_t num, size_t size) ;
void soc_free(void *) ;

typedef void (* PF_SOC_APC)(void *) ;

void SOC_apc_arg(int, PF_SOC_APC, void *) ;

static inline void SOC_apc(int a, PF_SOC_APC b)
{
	SOC_apc_arg(a, b, NULL) ;
}

bool SOC_apc_attiva(int) ;

void SOC_run(void) ;

// Si parte veloci
void SOC_sysclk(bool high) ;

RICH_CPU SOC_cpu(void) ;

void SOC_min(RICH_CPU) ;

/*
 * Prima di invocarla occorre, se serve:
 *    impostare la polarita' del wakeup
 *        CySysPmSetWakeupPolarity(CY_PM_STOP_WAKEUP_ACTIVE_HIGH) ;
 *    fermare il ble
 *        BLE_stop() ;
 *        CySysClkEcoStop() ;
 * Per usarla definire SOC_SPEGNI
 */
void SOC_spegni(void) ;

// Invocare prima di tutto il resto
void SOC_accendi(void) ;

#endif
