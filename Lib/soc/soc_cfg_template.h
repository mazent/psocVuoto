#ifndef SOC_CFG_H_
#define SOC_CFG_H_

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

// lista delle apc
#define MAX_SOC_APC     0

// se definito e una apc e' gia' in corso,
// se ne cerca una libera fra quelle in piu'
//#define MAX_NUM_APC     (MAX_SOC_APC + 3)


#endif
