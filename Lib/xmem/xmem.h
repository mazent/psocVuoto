#ifndef XMEM_H_
#define XMEM_H_

#include <stdbool.h>
#include <stdint.h>

#define XMEM_SECTOR_SIZE    (4 * 1024)
#define XMEM_BLOCK_SIZE     (64 * 1024)

void XMEM_accendi(void) ;
void XMEM_spegni(void) ;

bool XMEM_iniz(void) ;

bool XMEM_erase_sector(uint32_t) ;
bool XMEM_erase_block(uint32_t) ;

bool XMEM_read(uint32_t, void *, size_t) ;

bool XMEM_write(uint32_t, const void *, size_t) ;

bool XMEM_is_erased(uint32_t, size_t) ;

#endif
