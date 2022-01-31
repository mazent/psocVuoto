//#define STAMPA_DBG
#include "vp.h"
#include "crc1021.h"

#define CRC_INI     0xF134

bool VP_ini(S_VP * vp)
{
    if ( vp->crc != CRC_1021_v( CRC_INI, &vp->val, sizeof(uint32_t) ) ) {
        DBG_PRINTF("corrotto %08X[%04X]", vp->val, vp->crc) ;

        vp->val = 0 ;
        vp->crc = CRC_1021_v( CRC_INI, &vp->val, sizeof(uint32_t) ) ;

        return false ;
    }

    return true ;
}

uint32_t VP_leggi(S_VP * vp)
{
    uint32_t val = 0 ;

    if ( vp->crc == CRC_1021_v( CRC_INI, &vp->val, sizeof(uint32_t) ) ) {
        val = vp->val ;
    }

    return val ;
}

void VP_incrm(S_VP * vp)
{
    ++vp->val ;
    vp->crc = CRC_1021_v( CRC_INI, &vp->val, sizeof(uint32_t) ) ;
}

void VP_nuovo(
    S_VP * vp,
    uint32_t val)
{
    vp->val = val ;
    vp->crc = CRC_1021_v( CRC_INI, &vp->val, sizeof(uint32_t) ) ;
}

#if 0
void VP_prova(void)
{
    S_VP vp ;

    VP_ini(&vp) ;

    VP_nuovo(&vp, 0xFFFFFFFF) ;
    ASSERT( VP_ini(&vp) ) ;

    while ( true ) {
        VP_incrm(&vp) ;
        ASSERT( VP_ini(&vp) ) ;
    }
}

#endif
