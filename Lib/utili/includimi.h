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

#define SILICON_ID    (*(reg32 *) CYREG_SFLASH_SILICON_ID)

// Usare una copia di cm0gcc.ld inserendo questa area fra heap e stack
// (come la .noinit, per cui non si puo' inizializzare)
// Le dimensioni di stack e heap si possono modificare
// solo dentro cm0gcc.ld
// Esempio d'uso
//     #ifdef CY_BOOTLOADABLE_Bootloadable_H
//     static NOMANSLAND tipo variabile ;
//     #else
//     static CY_NOINIT tipo variabile ;
//     #endif

#define NOMANSLAND		CY_SECTION(".nml")

// Utili
#define UNUSED(x)       (void) ( sizeof(x) )
#define NOT(x)          ( ~(unsigned int) (x) )
#define MIN(a, b)       ( (a) < (b) ? (a) : (b) )
#define DIM_VETT(a)     ( sizeof(a) / sizeof(a)[0] )

// Se manca la define
#define SIZEOF_MEMBER(STRUCT, MEMBER) sizeof(((STRUCT *)0)->MEMBER)

// https://en.wikipedia.org/wiki/Offsetof
#ifndef offsetof
#define offsetof(STRUCT, MEMBER)     \
    ( (size_t)& ( ( (STRUCT *) 0 )->MEMBER ) )
#endif
#define container_of(MEMBER_PTR, STRUCT, MEMBER)    \
    ( (STRUCT *) ( (char *) (MEMBER_PTR) - offsetof(STRUCT, MEMBER) ) )

static inline void * CONST_CAST(const void * cv)
{
    union {
        const void * cv ;
        void * v ;
    } u ;
    u.cv = cv ;

    return u.v ;
}

static inline const void * CPOINTER(uint32_t cv)
{
    union {
    	uint32_t cv ;
        const void * v ;
    } u ;
    u.cv = cv ;

    return u.v ;
}

static inline void * POINTER(uint32_t cv)
{
    union {
    	uint32_t cv ;
        void * v ;
    } u ;
    u.cv = cv ;

    return u.v ;
}

static inline uint32_t UINTEGER(const void * v)
{
    union {
    	uint32_t cv ;
        const void * v ;
    } u ;
    u.v = v ;

    return u.cv ;
}


// Consumi
typedef enum {
    CPU_ATTIVA = 0,
    CPU_PAUSA,
    CPU_FERMA
} RICH_CPU ;

// Stampe
#include "dbg.h"

// Utili
#include "varie.h"

// Varie
#ifdef NDEBUG
// In release non ci devono essere
#   define BPOINT
#	define DYN_ASSERT(x)
#else
#   define BPOINT        \
	do {                 \
		__BKPT(0) ;      \
		__NOP() ;        \
		__NOP() ;        \
	}                    \
	while (false) ;
#	define DYN_ASSERT(x)  \
	do {                  \
        if ( !(x) ) {     \
            DBG_ASSERT ;  \
            BPOINT ;      \
        }                 \
	} while (false)
#endif

#define ASSERT(x)		  \
	do {                  \
        if ( !(x) ) {     \
            DBG_ASSERT ;  \
            BPOINT ;      \
        }                 \
	} while (false)

#define CHECK(x)		\
	do {                \
        if ( !(x) ) {   \
            DBG_ERR ;   \
        }               \
	} while (false)

#endif
