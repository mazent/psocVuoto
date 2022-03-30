#ifndef SOC_CFG_H_
#define SOC_CFG_H_

// Se definita, il BL non condivide la causa del reset
#define SOC_CAUSA_NC		1

// Reset immediato (per wdog) nelle eccezioni
// Definire globalmente
// CY_BOOT_INT_DEFAULT_HANDLER_ENOMEM_EXCEPTION_CALLBACK
// CY_BOOT_INT_DEFAULT_HANDLER_EXCEPTION_ENTRY_CALLBACK

// Se definita controlla lo stack stampando lo spazio libero minimo
#define SOC_CTRL_STACK      1

// Debug malloc/free con diario.py
#define SOC_DBG_MALLOC		1

// Come mi spengo (si riparte da reset, ram valida solo in hib)
#define SOC_SPEGNI_STOP     1	// CySysPmStop (risveglio da 2[2])
#define SOC_SPEGNI_HIB      2	// CySysPmHibernate
// make sure that there are no unexpected GPIO transitions during and after
// reset
#define SOC_SPEGNI_FHIB     3	// CySysPmfreezeIo + CySysPmHibernate
// Se non definita SOC_spegni e' vuota
#define SOC_SPEGNI          SOC_SPEGNI_STOP

// Lista delle apc
#define MAX_SOC_APC     0

// se definito e una apc e' gia' in corso,
// se ne cerca una libera fra quelle in piu'
//#define MAX_NUM_APC     (MAX_SOC_APC + 3)

// Lista delle isr
#define MAX_SOC_ISR     0

// Lista di chi vuole gestire cpu
#define MAX_SOC_CPU		0

#endif
