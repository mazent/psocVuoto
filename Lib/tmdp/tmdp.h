#ifndef TMDP_H_
#define TMDP_H_

#include <stdbool.h>
#include <stdint.h>

#define TMDP_MAX_DAT    248

typedef uint8_t TMD_ADDR ;
typedef uint8_t TMD_CMD ;

// Command callback (address ok)
void tmdp_esegui(
    TMD_CMD tcmd,
    void * v,
    uint16_t dim) ;

void TMDP_ini(void) ;

void TMDP_enter_deep(void) ;
void TMDP_leave_deep(void) ;

void TMDP_irq(bool x) ;

void TMDP_send(
    TMD_CMD cmd,
    const void * v,
    uint16_t dim) ;

// Common errors
#define TBUS_OK         0x00
#define TBUS_ERR        0x01
#define TBUS_ERR_PRM    0x80
#define TBUS_ERR_UNK    0xEE

void TMDP_err(
    TMD_CMD cmd,
    uint8_t err) ;

static inline void TMDP_ok(TMD_CMD cmd)
{
    TMDP_err(cmd, TBUS_OK) ;
}

#endif
