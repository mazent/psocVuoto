#ifndef SOC_CFG_H_
#define SOC_CFG_H_

// Non so come e' fatto l'hw
// Prima funzione invocata: NON c'e' ancora soc!
extern void HW_iniz(void) ;
// Ingresso in deep-sleep
extern void HW_sleep(void) ;
// Uscita da deep-sleep
extern void HW_wake(void) ;

// Invocata poco prima del ciclo infinito del main
extern void app_ini(void) ;

// Serve la collaborazione del BT
void BLE_run(void) ;
RICH_CPU BLE_cpu(void) ;

// Se definita controlla lo stack stampando lo spazio libero minimo
#define SOC_CTRL_STACK      1

// Come mi spengo (si riparte da reset, ram valida solo in hib)
// CySysPmStop
#define SOC_SPEGNI_STOP     1
// CySysPmHibernate
#define SOC_SPEGNI_HIB      2
// CySysPmfreezeIo + CySysPmHibernate
// make sure that there are no unexpected GPIO transitions during and after
// reset
#define SOC_SPEGNI_FHIB     3
// Se non definita SOC_spegni e' vuota
#define SOC_SPEGNI          SOC_SPEGNI_STOP


// lista delle apc
#define APC_BLE			0
#define APC_AGG_ADV		1
#define APC_BL_EMU		2
#define MAX_SOC_APC		3


#endif
