#ifndef XMEM_H_
#define XMEM_H_

#include "utili/includimi.h"

/*
 * I primi quattro blocchi sono usati per l'aggiornamento
 *
 * Gli altri quattro sono disponibili
 */

// Uguale a AT25SF041_BLOCK_SIZE
#define XMEM_BLOCK_SIZE		(64 * 1024)

bool XMEM_iniz(void) ;

//bool XMEM_erase_sector(uint32_t) ;
bool XMEM_erase_block(uint32_t) ;
//bool XMEM_erase_all(void) ;

bool XMEM_read(uint32_t, void *, size_t) ;

bool XMEM_write(uint32_t, const void *, size_t) ;

bool XMEM_is_erased(uint32_t pos, size_t dim) ;

void XMEM_erase_boot_flash(void) ;

#endif
