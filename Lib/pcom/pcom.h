#ifndef COD_PCOM_H_
#define COD_PCOM_H_

typedef void (*PCOM_MSG_CALLBACK)(void *, size_t) ;

// Callback invocata ad ogni messaggio
void PCOM_iniz(PCOM_MSG_CALLBACK) ;

// Invocare quando arriva qlc
void PCOM_esamina(void *, size_t) ;

// Restituisce la roba da trasmettere (o NULL)
typedef struct {
    const void * tx ;
    size_t dimTx ;
} S_PCOM_RDT ;
const S_PCOM_RDT * PCOM_rdt(const void *, size_t) ;

#else
#   warning pcom.h incluso
#endif
