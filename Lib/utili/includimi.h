#ifndef INCLUDIMI_H_
#define INCLUDIMI_H_

// Servono sempre
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdbool.h>
	// per la static_assert
#include <assert.h>
	// Per le PRIu32 ecc
#include <inttypes.h>

// Scheda
#include <project.h>

// Utili
#define UNUSED(x)     	(void)(sizeof(x))
#define NOT(x)          (~(unsigned int) (x))
#define MIN(a, b)		((a) < (b) ? (a) : (b))
#define DIM_VETT(a)		(sizeof(a) / sizeof(a[0]))

static inline void * CONST_CAST(const void * cv)
{
	union {
		const void * cv ;
		void * v ;
	} u ;
	u.cv = cv ;

	return u.v ;
}

// Consumi
typedef enum {
	CPU_ATTIVA = 0,
	CPU_PAUSA,
	CPU_FERMA
} RICH_CPU ;

// Stampe
#include "dbg.h"

// Varie
#ifdef NDEBUG
	// In release non ci devono essere
#	define BPOINT
#	define ASSERT(x)		(void) (x)
#	define CHECK(x)			(void) (x)
#else
#	define BPOINT			__BKPT(0)
#	define ASSERT(x)	    do {				  \
						    	if (!(x)) {       \
						    		DBG_ASSERT ;  \
						    		__BKPT(0)  ;  \
						    	}				  \
						    } while (false)
#	define CHECK(x)	    	do {				  \
						    	if (!(x)) {       \
						    		DBG_ERR ;	  \
						    	}				  \
						    } while (false)
#endif

#endif
