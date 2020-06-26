#ifndef FTL_H_
#define FTL_H_

#include "utili/includimi.h"

// Uso due blocchi di flash ...
#define FTL_BLK_1 		64
#define FTL_BLK_2 		65
// ... della stessa dimensione
#define FTL_BLK_DIM 	4096

// Deve poter rappresentare il blocco massimo
typedef uint8_t BLK_T ;

// Salvo un certo numero di settori logici ...
#define FTL_LSECT_NUM 	3
// ... della stessa dimensione
#define FTL_LSECT_DIM 	(257 - 2)

// Deve poter rappresentare il settore massimo
typedef uint8_t SECT_T ;

// Siccome ci sono due blocchi uguali, la distanza massima fra due
// copie dello stesso settore sara' al massimo il doppio del numero
// dei settori fisici piu' due (valori riservati)
typedef uint8_t LRU_T ;


// Flash manipulation
	// Read
typedef bool (* FTL_BUS_READ)(BLK_T blk, size_t ofs, void *, size_t) ;
    // Write
typedef bool (* FTL_BUS_WRITE)(BLK_T blk, size_t ofs, const void *, size_t) ;
    // Erase
typedef bool (* FTL_BUS_ERASE)(BLK_T blk) ;

typedef struct {
	// Operazioni sulla flash
    FTL_BUS_ERASE pfErase ;
    FTL_BUS_WRITE pfWrite ;
    FTL_BUS_READ  pfRead ;
} FTL_OP ;

// The first operation
bool FTL_iniz(const FTL_OP *) ;

// The last operation
void FTL_fine(void) ;

// Retrieve a sector
bool FTL_read(SECT_T logic, void * data) ;

// If data is NULL the sector will not exist anymore
bool FTL_write(SECT_T logic, const void * data) ;

#endif
