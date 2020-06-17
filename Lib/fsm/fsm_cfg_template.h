#ifndef FSM_CFG_H_
#define FSM_CFG_H_

#define STAMPA_DBG
#include "utili/includimi.h"

#ifdef STAMPA_DBG
#	define fsm_printf(f, ...)			DBG_PRINTF(f, ##__VA_ARGS__)
#else
#	define fsm_printf(f, ...)
#endif


// Niente storie
//MZ #define FSM_HISTORY   ( 64 )
#define FSM_HISTORY   ( 0 )

// Anche troppo
#define FSM_NAME_LEN     ( 32 )

// No, grazie
//#define FSM_PROLISSO
//#define USA_FSM_ADESSO

// Requisiti
	// memoria
extern void * soc_malloc(size_t) ;
extern void soc_free(void *) ;
#define FSM_MALLOC		soc_malloc
#define FSM_FREE		soc_free
	// restituisce un riferimento temporale (per stampe debug)
//extern uint32_t fsm_adesso(void) ;


#endif
