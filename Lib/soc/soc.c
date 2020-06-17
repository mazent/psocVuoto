//#define STAMPA_DBG
#include <project.h>
#include "soc.h"

#ifndef SOC_SPEGNI
#	define SOC_SPEGNI	0
#endif

extern void timer_ini(void) ;
extern void timer_reini(void) ;
extern void timer_run(void) ;
extern bool timer_attivi(void) ;

// Livello di cpu imposto a compile-time
#ifdef NDEBUG
static const RICH_CPU CPU_CT = CPU_FERMA ;
#else
// Per debug impedisco deep-sleep
static const RICH_CPU CPU_CT = CPU_PAUSA ;
#endif

// Livello di cpu imposto a run-time
static RICH_CPU cpu_rt = CPU_FERMA ;

typedef struct {
	PF_SOC_APC apc ;
	void * arg ;
} UNA_APC ;

static UNA_APC vAPC[MAX_SOC_APC] ;


static void hard_fault(void)
{
#ifdef NDEBUG
	CySoftwareReset() ;
#else
	__BKPT(0) ;
#endif
}

#ifdef DEBUG

static size_t tot = 0 ;
typedef struct {
	void * v ;
	size_t d ;
} UN_MALLOC ;
#define NUM_MALLOC		10
static UN_MALLOC vM[NUM_MALLOC] ;

static void iniz_malloc(void)
{
	for (int i=0 ; i<NUM_MALLOC ; ++i) {
		vM[i].v = NULL ;
	}
}

void * soc_malloc(size_t dim)
{
	void * v = malloc(dim) ;

	if (v) {
		tot += dim ;

		int i=0 ;
		for (; i<NUM_MALLOC ; ++i) {
			if (NULL == vM[i].v) {
				vM[i].v = v ;
				vM[i].d = dim ;
				break ;
			}
		}
		if (NUM_MALLOC == i) {
			DBG_ERR ;
		}

		DBG_PRINTF("%s(%d) -> %d", __func__, dim, tot) ;
	}
	else {
		DBG_ERR ;
	}

    return v ;
}

void * soc_calloc(size_t num, size_t size)
{
	void * v = calloc(num, size) ;

	if (v) {
		size_t dim = num * size ;
		tot += dim ;

		int i=0 ;
		for (; i<NUM_MALLOC ; ++i) {
			if (NULL == vM[i].v) {
				vM[i].v = v ;
				vM[i].d = dim ;
				break ;
			}
		}
		if (NUM_MALLOC == i) {
			DBG_ERR ;
		}

		DBG_PRINTF("%s(%d) -> %d", __func__, dim, tot) ;
	}
	else {
		DBG_ERR ;
	}

    return v ;
}


void soc_free(void * v)
{
	if (v) {
		int i=0 ;
		for ( ; i<NUM_MALLOC ; ++i) {
			if (v == vM[i].v) {
				tot -= vM[i].d ;
				vM[i].v = NULL ;
				free(v) ;
				DBG_PRINTF("%s(%d) -> %d", __func__, vM[i].d, tot) ;
				break ;
			}
		}

		if (NUM_MALLOC == i) {
			DBG_ERR ;
		}
	}
	else {
		DBG_ERR ;
	}
}

#else

static void iniz_malloc(void) {}

void * soc_malloc(size_t dim)
{
    return malloc(dim) ;
}

void * soc_calloc(size_t num, size_t size)
{
	return calloc(num, size) ;
}

void soc_free(void * v)
{
	if (v)
		free(v) ;
}

#endif

void SOC_ini(void)
{
	// sostituisco while(1) con un bel reset
    (void) CyIntSetSysVector(CY_INT_HARD_FAULT_IRQN, hard_fault) ;

    iniz_malloc() ;

	timer_ini() ;

	for (int i=0 ; i<MAX_SOC_APC ; i++) {
		vAPC[i].apc = NULL ;
		vAPC[i].arg = NULL ;
	}
}

void SOC_apc_arg(int quale, PF_SOC_APC cb, void * arg)
{
	ASSERT(quale < MAX_SOC_APC) ;
//	ASSERT(NULL == vAPC[quale].apc) ;

	if (quale < MAX_SOC_APC) {
		vAPC[quale].apc = cb ;
		vAPC[quale].arg = arg ;
	}
}

bool SOC_apc_attiva(int quale)
{
	ASSERT(quale < MAX_SOC_APC) ;

	if (quale < MAX_SOC_APC) {
		return NULL != vAPC[quale].apc ;
	}
	else
		return false ;
}

void SOC_run(void)
{
	for (int i=0 ; i<MAX_SOC_APC ; i++) {
		CyGlobalIntDisable ;

		PF_SOC_APC apc = vAPC[i].apc ;
		void * arg = vAPC[i].arg ;
		vAPC[i].apc = NULL ;
		vAPC[i].arg = NULL ;

		CyGlobalIntEnable ;

		if (apc) {
			DBG_PRINTF("eseguo %p(%p)", apc, arg) ;
			apc(arg) ;
			DBG_PRINTF("fine exec %p(%p)", apc, arg) ;
		}
	}

	timer_run() ;
}

RICH_CPU SOC_cpu(void)
{
	RICH_CPU s = MIN(cpu_rt, CPU_CT) ;

	for (int i=0 ; i<MAX_SOC_APC ; i++) {
		if ( vAPC[i].apc ) {
			s = CPU_ATTIVA ;
			break ;
		}
	}

	if ( timer_attivi() )
		s = MIN(CPU_PAUSA, s) ;

	return s ;
}

void SOC_min(RICH_CPU cpu)
{
	cpu_rt = cpu ;
}

void SOC_sysclk(bool high)
{
#if defined(SOC_SYSCLK_HIGH) && defined(SOC_SYSCLK_LOW)
	static bool alta = true ;

	if (high != alta) {
		uint32_t div = SOC_SYSCLK_LOW ;
		if (alta)
			div = SOC_SYSCLK_HIGH ;

		CySysClkWriteSysclkDiv(div) ;

		timer_reini() ;

		alta = high ;
	}
#endif
}

void SOC_spegni(void)
{
#if SOC_SPEGNI == 0
	// Perfetto
#elif SOC_SPEGNI == SOC_SPEGNI_STOP
	DBG_PUTS("CySysPmStop") ;
	CyDelay(1) ;
	CySysPmStop() ;
#elif SOC_SPEGNI == SOC_SPEGNI_HIB
	DBG_PUTS("CySysPmHibernate") ;
	CyDelay(1) ;
	CySysPmHibernate() ;
#elif SOC_SPEGNI == SOC_SPEGNI_FHIB
	DBG_PUTS("CySysPmfreezeIo") ;
	CyDelay(1) ;
	CySysPmfreezeIo() ;
	CySysPmHibernate() ;
#endif
}

void SOC_accendi(void)
{
#if SOC_SPEGNI == 0
	// Perfetto
#elif SOC_SPEGNI == SOC_SPEGNI_STOP
	// CySysPmStop() function freezes IO-Cells implicitly.
	if (CY_PM_RESET_REASON_WAKEUP_STOP == CySysPmGetResetReason())
		CySysPmUnfreezeIo() ;
#elif SOC_SPEGNI == SOC_SPEGNI_HIB
	// Grande
#elif SOC_SPEGNI == SOC_SPEGNI_FHIB
	if (CY_PM_RESET_REASON_WAKEUP_HIB == CySysPmGetResetReason())
		CySysPmUnfreezeIo() ;
#endif
}
