#ifndef FTL_H_
#define FTL_H_

#include "ftl_cfg.h"


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
