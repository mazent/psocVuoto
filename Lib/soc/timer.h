#ifndef TIMER_H_
#define TIMER_H_

/*
 * Timer software
 */

#include <stdint.h>
#include <stdbool.h>

#include "timer_cfg.h"

// Callback invocata quando scade il tempo impostato
typedef void (* PF_TIMER_SW)(void *) ;

// Uso dei timer
//     assegna al timer la sua callback (opzionale)
void timer_setcb(int, PF_TIMER_SW) ;
//     [ri]parte il timer
//     Si puo' passare ms == 0 per avere una apc
//     lanciabile da dentro interruzione
void timer_start(int, uint32_t ms) ;
void timer_start_arg(int, uint32_t ms, void *) ;
//     ferma il timer
void timer_stop(int) ;
//     vera se il timer e' attivo
bool timer_running(int) ;


#endif
