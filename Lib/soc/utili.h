#ifndef COD_UTILI_H_
#define COD_UTILI_H_

// Servono sempre
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
// per la static_assert
#include <assert.h>
// Per le PRIu32 ecc
#include <inttypes.h>

// Scheda
#include <project.h>

// .625 =
#define CONV_NUM    5
#define CONV_DEN    8
#define ADV_INT_MS      ( (CYBLE_FAST_ADV_INT_MIN * CONV_NUM) / CONV_DEN )

// Varie
#define UNUSED(x)       (void) ( sizeof(x) )
#define NOT(x)          ( ~(unsigned int) (x) )
#define MIN(a, b)       ( (a) < (b) ? (a) : (b) )
#define DIM_VETT(a)     (sizeof(a) / sizeof(a)[0])
// Conversione a stringa (token stringification)
//      #define foo 4
//      STRINGA(foo)  -> "foo"
//      STRINGA2(foo) -> "4"
#define STRINGA(a)       # a
#define STRINGA2(a)      STRINGA(a)

// Se manca la define
#define SIZEOF_MEMBER(STRUCT, MEMBER) sizeof( ( (STRUCT *) 0 )->MEMBER )

// https://en.wikipedia.org/wiki/Offsetof
#ifndef offsetof
#define offsetof(STRUCT, MEMBER)     \
    ( (size_t)& ( ( (STRUCT *) 0 )->MEMBER ) )
#endif
#define container_of(MEMBER_PTR, STRUCT, MEMBER)    \
    ( (STRUCT *) ( (char *) (MEMBER_PTR) -offsetof(STRUCT, MEMBER) ) )

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

// Stampe (definire STAMPA_DBG prima di includere)
#include "ddb/ddb.h"

// Varie
#ifdef NDEBUG
// In release non ci devono essere
#   define BPOINT
#   define DYN_ASSERT(x)
#else
#   define BPOINT        \
    do {                 \
        __BKPT(0) ;      \
        __NOP() ;        \
        __NOP() ;        \
    }                    \
    while ( false ) ;
#   define DYN_ASSERT(x)  \
    do {                  \
        if ( !(x) ) {     \
            DBG_ASSERT ;  \
            BPOINT ;      \
        }                 \
    } while ( false )
#endif

#define ASSERT(x)         \
    do {                  \
        if ( !(x) ) {     \
            DBG_ASSERT ;  \
            BPOINT ;      \
        }                 \
    } while ( false )

#define CHECK(x)        \
    do {                \
        if ( !(x) ) {   \
            DBG_ERR ;   \
        }               \
    } while ( false )

#else
#   warning utili.h incluso
#endif
