#ifndef FSPI_H_
#define FSPI_H_

#include "utili/includimi.h"

void FSPI_write(const void *, size_t) ;
void FSPI_read(void *, size_t) ;
void FSPI_dummy_read(size_t) ;

typedef bool (*PF_FSPI_READ)(uint8_t) ;
void FSPI_read_cb(PF_FSPI_READ) ;

#endif
