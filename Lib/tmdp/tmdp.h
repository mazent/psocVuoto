#ifndef TMDP_H_
#define TMDP_H_

#include "utili/includimi.h"

#define TMDP_MAX_DAT    248

typedef uint8_t TMD_CMD ;

// Command callback (address ok)
typedef void (*PF_TMD_CMD)(TMD_CMD, void *, uint16_t) ;

void TMDP_ini(void) ;

void TMDP_enter_deep(void) ;
void TMDP_leave_deep(void) ;

void TMDP_irq(bool x) ;

// Callback invoked from here
void TMDP_poll(void) ;

void TMDP_send(TMD_CMD cmd, const void * v, uint16_t dim) ;

// Common errors
#define TBUS_OK         0x00
#define TBUS_ERR        0x01
#define TBUS_ERR_PRM    0x80
#define TBUS_ERR_UNK    0xEE

void TMDP_err(TMD_CMD cmd, uint8_t err) ;

static inline void TMDP_ok(TMD_CMD cmd)
{
    TMDP_err(cmd, TBUS_OK) ;
}

#endif
