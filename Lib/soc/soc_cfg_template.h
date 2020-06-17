#ifndef SOC_CFG_H_
#define SOC_CFG_H_

// Si parte con quella alta
#define SOC_SYSCLK_HIGH		CY_SYS_CLK_SYSCLK_DIV1
#define SOC_SYSCLK_LOW		CY_SYS_CLK_SYSCLK_DIV4

// Come mi spengo
	// CySysPmStop
#define SOC_SPEGNI_STOP		1
	// CySysPmHibernate
#define SOC_SPEGNI_HIB 		2
	// CySysPmfreezeIo + CySysPmHibernate
#define SOC_SPEGNI_FHIB		3
	// Se non definita SOC_spegni e' vuota
#define SOC_SPEGNI			SOC_SPEGNI_FHIB

// lista delle apc
#define MAX_SOC_APC		0


#endif
