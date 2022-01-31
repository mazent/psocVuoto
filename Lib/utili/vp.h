#ifndef LIB_UTILI_VP_H_
#define LIB_UTILI_VP_H_

#include "includimi.h"

/*
 * Associa ad un valore un crc rendendo sicuro il
 * salvataggio con CY_NOINIT
 */

typedef struct {
    uint32_t val ;
    uint16_t crc ;
} S_VP ;

// Se non valido imposta 0 e torna falso
bool VP_ini(S_VP *) ;

// Se non valido torna 0
uint32_t VP_leggi(S_VP *) ;

// Aggiunge 1
void VP_incrm(S_VP *) ;

// Assegna un nuovo valore
void VP_nuovo(S_VP *, uint32_t) ;

#else
#   warning vp.h incluso
#endif
