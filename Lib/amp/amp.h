#ifndef AMP_H_
#define AMP_H_

#include <stdbool.h>
#include <stdint.h>

/*
 * Area di Memoria Persistente (in ram)
 */

void AMP_iniz(void) ;

bool AMP_resta_nel_bl(void) ;
void AMP_forza_nel_bl(bool si) ;

// L'applicazione deve condividere queste info in modo
// che il dispositivo si comporti allo stesso modo

#define PRM_MAX_DIM_COD_PRD		16

void AMP_prd_s(const char *, int8_t) ;
const char * AMP_prd_l(void) ;

void AMP_pkey_s(const uint32_t *) ;
const uint32_t * AMP_pkey_l(void) ;


#else
#	warning amp.h incluso
#endif
