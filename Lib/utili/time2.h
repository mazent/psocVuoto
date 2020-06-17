#ifndef TIME2_H_
#define TIME2_H_

#include "includimi.h"
#include <time.h>

/*
 * L'epoca classica parte da
 *               0 = 1 January 1970 00:00:00
 * e arriva a
 *      2147483647 = 19 January 2038 03:14:07
 *
 * Queste funzioni permettono di estendere il periodo
 * (escluso e non oltre il 2100, che non e' bisestile)
 *
 * Per evitare di usare 64 bit, l'epoca che usano e' riferita
 * all'anno TIME2_BASE. Se non ridefinito vale 2020, per cui
 * si aggiungono 50 anni arrivando al 2088 (escluso)
 *
 * cfr https://www.epochconverter.com/
 */

time_t mktime2( struct tm * ) ;

// Torna NULL se errore
struct tm * gmtime2( const time_t * ) ;

#else
#   warning time2.h incluso
#endif
