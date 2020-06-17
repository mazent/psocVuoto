#ifndef SOC_CFG_H_
#define SOC_CFG_H_

// Come mi spengo
	// CySysPmStop
#define SOC_SPEGNI_STOP		1
	// CySysPmHibernate
#define SOC_SPEGNI_HIB 		2
	// CySysPmfreezeIo + CySysPmHibernate
#define SOC_SPEGNI_FHIB		3
	// Se non definita SOC_spegni e' vuota
//#define SOC_SPEGNI			SOC_SPEGNI_FHIB


// lista delle apc
#define APC_BLE			0
#define APC_AGG_ADV		1
#define APC_BL_EMU		2
#define MAX_SOC_APC		3


#endif
