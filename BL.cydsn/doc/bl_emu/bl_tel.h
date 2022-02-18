#ifndef BL_EMU_BL_TEL_H_
#define BL_EMU_BL_TEL_H_

#include <stdbool.h>
#include <stdint.h>

/*
 * Bootloader telefonico
 */

bool BLT_iniz(void) ;

bool BLT_scrivi(
    uint8_t aid,
    uint16_t rn,
    const uint8_t * dati) ;

typedef bool (*PF_VALIDA_RIGA)(void *) ;

// La cb viene invocata ad ogni riga e, con NULL,
// alla fine: serve a calcolare e verificare l'hash
// della roba scritta in flash
bool BLT_esci(PF_VALIDA_RIGA /*opzionale*/) ;

#else
#   warning bl_tel.h incluso
#endif
