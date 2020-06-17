#define STAMPA_DBG
#include "soc/soc.h"
#include "ble/ble.h"

extern void app_ini(void) ;

// Evito il deep-sleep immediato
#define DURATA_POW      20


#ifndef NDEBUG		/* utilizzo stack vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */

static const uint32_t STACK_NON_USATO = 0xDEADBABE ;

extern char __cy_heap_end ;
extern char __cy_stack ;
static size_t dimStack ;

static uint32_t * valid_cast(char * x)
{
	union {
		char * c ;
		uint32_t * dw ;
	} cast ;

	cast.c = x ;

	return cast.dw ;
}

static void fill_stack(void)
{
	// Riempio lo stack con un valore noto
	dimStack = (&__cy_stack - &__cy_heap_end) / sizeof(uint32_t) ;
	uint32_t * stack = valid_cast(&__cy_heap_end) ;
	for (size_t i=0 ; i< dimStack - 100 ; i++)
		stack[i] = STACK_NON_USATO ;
}

static void iniz_stack(void)
{
	uint32_t * stack = valid_cast(&__cy_heap_end) ;

	// Inizializzo
	for (size_t i=0 ; i<dimStack ; ++i) {
		if (stack[i] != STACK_NON_USATO) {
			dimStack = i - 1 ;
			break ;
		}
	}

	DBG_PRINTF("stack unused = %d", dimStack * sizeof(uint32_t)) ;
}

static void runt_stack(void)
{
	// Rispetto allo stack precedente, cerco il limite attuale
	bool modif = false ;
	uint32_t * stack = valid_cast(&__cy_heap_end) ;

	while (stack[dimStack] != STACK_NON_USATO) {
		--dimStack ;
		modif = true ;
	}

	if (modif) {
		DBG_PRINTF("stack unused = %d", dimStack * sizeof(uint32_t)) ;
	}
}

#else

static void fill_stack(void) {}
static void iniz_stack(void) {}
static void runt_stack(void) {}

#endif			    /* utilizzo stack ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

//void resetta(void)
//{
//	DBG_PUTS("CySoftwareReset") ;
//	CyDelay(2) ;
//
//	CySoftwareReset() ;
//}
//
//uint32_t get_ver(void)
//{
//	uint32_t ver = VER_MAG ;
//	ver <<= 24 ;
//	ver |= VERSIONE ;
//
//	return ver ;
//}

int main(void)
{
	fill_stack() ;

	SOC_accendi() ;

	// Controllo la ram non inizializzata
    //AMP_iniz() ;
    
	CyGlobalIntEnable ;

	// Prima debug e s.o.
	DBG_INIZ ;
	SOC_ini() ;
    WDOG_iniz() ;
#ifdef NDEBUG
    DBG_PRINTF("APP %s %s", __DATE__, __TIME__) ;
#else
    DBG_PRINTF("APP (dbg) %s %s", __DATE__, __TIME__) ;
#endif

	// Per ultima l'applicazione
    app_ini() ;

#if 1
    // Timer per evitare il deep-sleep per un poco
    // (senza callback)
	timer_start(TIM_POW, DURATA_POW) ;
#else
	// Evito deep-sleep
	SOC_min(CPU_PAUSA) ;
#endif

	iniz_stack() ;

	while (true) {
		runt_stack() ;

		// Avviso che va tutto bene
		WDOG_calcia() ;

		// Un giro di giostra
        SOC_run() ;
        BLE_run() ;

        uint8_t interruptStatus = CyEnterCriticalSection() ;

        RICH_CPU scpu = SOC_cpu() ;
        RICH_CPU bcpu = BLE_cpu() ;

        switch ( MIN(scpu, bcpu) ) {
    	case CPU_ATTIVA:
    		break ;
    	case CPU_PAUSA: {
				bool eco =
#if 1
							false ;
#else
							// non funziona
							BLE_clock() ;
#endif
				if (eco) {
					/* change HF clock source from IMO to ECO, as IMO is not required and can be stopped to save power */
					CySysClkWriteHfclkDirect(CY_SYS_CLK_HFCLK_ECO);
					/* stop IMO for reducing power consumption */
					CySysClkImoStop();
				}

				/* put the CPU to sleep */
				CySysPmSleep();

				if (eco) {
					/* starts execution after waking up, start IMO */
					CySysClkImoStart();
					/* change HF clock source back to IMO */
					CySysClkWriteHfclkDirect(CY_SYS_CLK_HFCLK_IMO);
				}
			}
    		break ;
    	case CPU_FERMA: {
				// Disabilito
				BLE_enter_deep() ;
				DBG_ENTER_DEEP ;

				// Dormo
				CySysPmDeepSleep() ;

				// Riabilito
				DBG_LEAVE_DEEP ;
				BLE_leave_deep() ;
#if 0
				timer_start(TIM_POW, DURATA_POW) ;
#endif
    		}
    		break ;
        }

        CyExitCriticalSection(interruptStatus) ;
	}
}
