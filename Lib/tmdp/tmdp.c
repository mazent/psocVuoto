#include <project.h>
//#define STAMPA_DBG
#include "soc/utili.h"
#include "soc/soc.h"
#include "tmdp.h"

#ifdef CY_SCB_TBUS_H
#include "tmdp_cfg.h"
#include "comuni/circo.h"

extern uint16 crc1021(
    uint16 crc,
    uint8_t val) ;

// Evito disallineamenti (il protocollo non si resincronizza)
// 19200 b/s == 1920 B/s -> 19 B/10ms
#define SW_TO_RX     200

static enum {
    STT_WAIT,
    STT_WAIT_2,
    STT_ADDR,
    STT_DIM,
    STT_DAT,
    STT_CRC_1,
    STT_CRC_2,
    STT_NULL
} stato = STT_WAIT ;

// 256 + margine
#define MAX_BUFF        (260)

#define CRC_INI     0x00BD

#define STX     0x02

static uint8_t bufRx[MAX_BUFF] ;
static TMD_ADDR curAddr ;
static uint16 crc ;
static uint8_t curDim ;
static uint8_t dimRx = 0 ;

#if 1
static uint32_t rim = 0 ;
static int conta_rim = 0 ;

static void tbus_stop(void)
{
    DBG_PUTS(__func__) ;

    conta_rim += 1 ;
    if ( 1 == conta_rim ) {
        rim = TBUS_GetRxInterruptMode() ;
        TBUS_SetRxInterruptMode(0) ;
    }
    else {
        // Non dovrebbe capitare ...
        DBG_ERR ;
    }
}

static void tbus_restart(void)
{
    bool abil = true ;

    DBG_PUTS(__func__) ;

    conta_rim -= 1 ;
    if ( conta_rim < 0 ) {
        DBG_ERR ;
        abil = false ;
    }
    else if ( 0 == conta_rim ) {}
    else {
        // ... ma se capita non abilito
        DBG_ERR ;
        DBG_PRINTF("\t%d", conta_rim) ;
        abil = false ;
    }

    if ( abil ) {
        // Caso mai fossero arrivati dei byte
        uint32_t value = TBUS_GetRxInterruptSourceMasked() ;
        if ( value ) {
            TBUS_ClearRxInterruptSource(value) ;
        }
        TBUS_SpiUartClearRxBuffer() ;

        stato = STT_WAIT ;

        TBUS_SetRxInterruptMode(rim) ;
    }
}

#else
// Non funziona
static void tbus_stop(void)
{
    SLEEP_TBUS_Write(0) ;
}

static void tbus_restart(void)
{
    SLEEP_TBUS_Write(1) ;
}

#endif

static bool esamina(uint8_t rx)
{
    bool trovato = false ;
    switch ( stato ) {
    case STT_WAIT:
        if ( rx == STX ) {
            stato = STT_WAIT_2 ;
        }
        break ;

    case STT_WAIT_2:
        if ( rx == STX ) {
            stato = STT_ADDR ;
        }
        else {
            stato = STT_WAIT ;
        }
        break ;

    case STT_ADDR:
        curAddr = rx ;
        crc = crc1021(CRC_INI, curAddr) ;
        stato = STT_DIM ;
        break ;

    case STT_DIM:
        curDim = rx ;
        crc = crc1021(crc, curDim) ;
        stato = STT_DAT ;
        dimRx = 0 ;
        break ;

    case STT_DAT:
        bufRx[dimRx] = rx ;
        crc = crc1021(crc, rx) ;
        dimRx++ ;
        if ( dimRx >= curDim ) {
            stato = STT_CRC_1 ;
        }
        break ;

    case STT_CRC_1:
        crc = crc1021(crc, rx) ;
        stato = STT_CRC_2 ;
        break ;

    case STT_CRC_2:
        timer_stop(TIM_TMDP) ;

        crc = crc1021(crc, rx) ;
        if ( crc != 0 ) {     // NOLINT(bugprone-branch-clone)
            stato = STT_WAIT ;
        }
        else if ( curAddr != TMDP_ADDR ) {
            stato = STT_WAIT ;
        }
        else {
            // basta comandi fino alla risposta
            tbus_stop() ;

            trovato = true ;

            stato = STT_NULL ;
        }
        break ;

    case STT_NULL:
        break ;

    default:
        //DBG_ERR ;
        stato = STT_WAIT ;
        break ;
    }

    return trovato ;
}

__attribute__( (weak) )
void tmdp_esegui(
    TMD_CMD tcmd,
    void * v,
    uint16_t dim)
{
    UNUSED(tcmd) ;
    UNUSED(v) ;
    UNUSED(dim) ;
}

static void tmdp_to(void * v)
{
    if ( NULL == v ) {
        // Resincronizzo
        stato = STT_WAIT ;
    }
    else {
        // Eseguo il comando
        tmdp_esegui(bufRx[0], bufRx + 2, dimRx - 2) ;
    }
}

// Interrupts

static const uint32_t IRQ_ERR = TBUS_INTR_RX_OVERFLOW
                                | TBUS_INTR_RX_FRAME_ERROR
                                | TBUS_INTR_RX_PARITY_ERROR ;
static const uint32_t IRQ_RX = TBUS_INTR_RX_FIFO_LEVEL
                               | TBUS_INTR_RX_NOT_EMPTY | TBUS_INTR_RX_FULL ;

CY_ISR(tbus_irq)
{
    uint32_t irq = TBUS_GetRxInterruptSourceMasked() ;

    if ( irq & IRQ_ERR ) {
        // In caso di errore me ne accorgo dal crc
        TBUS_SpiUartClearRxBuffer() ;
    }
    else if ( irq & IRQ_RX ) {
        bool trovato = false ;

        while ( TBUS_SpiUartGetRxBufferSize() ) {
            union {
                uint32_t temp ;
                uint8_t rx ;
            } u ;
            u.temp = TBUS_SpiUartReadRxData() ;
            if ( esamina(u.rx) ) {
                trovato = true ;
            }
        }

        if ( trovato ) {
#ifdef ISR_TMDP
            SOC_isr_arg( ISR_TMDP, tmdp_to, POINTER(0x5555) ) ;
#else
            // Uso il timer come apc usabile da isr
            timer_start_arg( TIM_TMDP, 0, POINTER(0x5555) ) ;
#endif
        }
        else {
            // Per resincronizzarsi
            timer_start(TIM_TMDP, SW_TO_RX) ;
        }
    }
    else {
        // ???
    }

    TBUS_ClearRxInterruptSource(irq) ;
}

static void waitNrx(void)
{
    // Aspetto fine tx
    uint32 level = TBUS_SpiUartGetTxBufferSize() ;

    while ( level ) {
        CyDelay(1) ;
        level = TBUS_SpiUartGetTxBufferSize() ;
    }

    // shift register
    CyDelay(1) ;

    // Riabilito la ricezione
    tbus_restart() ;
}

static uint16_t preambolo(
    uint8_t dim,
    TMD_CMD cmd,
    uint8_t esito)
{
    uint16_t crcTx = CRC_INI ;
    uint8_t cosa[] = {
        STX, STX, TMDP_ADDR, dim, cmd, esito
    } ;

    DBG_PRINT_HEX( "tbus <-", cosa, sizeof(cosa) ) ;

    for ( size_t i = 2 ; i < sizeof(cosa) ; ++i ) {
        crcTx = crc1021(crcTx, cosa[i]) ;
    }

    for ( size_t i = 0 ; i < sizeof(cosa) ; ++i ) {
        TBUS_SpiUartWriteTxData( (uint32) cosa[i] ) ;
    }

    return crcTx ;
}

static void postambolo(uint16_t crc_)
{
    union {
        uint16_t crc ;
        uint8_t b[sizeof(uint16_t)] ;
    } u ;

    // invio crc big endian
    u.crc = __REV16(crc_) ;
    DBG_PRINT_HEX( "tbus <-", u.b, sizeof(crc_) ) ;

    for ( size_t i = 0 ; i < sizeof(crc_) ; ++i ) {
        TBUS_SpiUartWriteTxData( (uint32) u.b[i] ) ;
    }

    waitNrx() ;
}

/**********************************
        Interfaccia
**********************************/

void TMDP_ini(void)
{
    timer_setcb(TIM_TMDP, tmdp_to) ;

    // tbus
    TBUS_SetCustomInterruptHandler(tbus_irq) ;
    TBUS_Start() ;
}

void TMDP_enter_deep(void)
{
    TBUS_Sleep() ;
}

void TMDP_leave_deep(void)
{
    TBUS_Wakeup() ;
}

void TMDP_irq(bool x)
{
    if ( x ) {
        TBUS_INT_Write(1) ;
    }
    else {
        TBUS_INT_Write(0) ;
    }
}

void TMDP_send(
    TMD_CMD cmd,
    const void * v,
    uint16_t dim)
{
    const uint8_t * dati = (const uint8_t *) v ;
    uint16 crcTx = preambolo(dim + 2, cmd, TBUS_OK) ;

    if ( dim ) {
        DBG_PRINT_HEX("tbus <-", dati, dim) ;

        for ( uint16_t i = 0 ; i < dim ; i++ ) {
            TBUS_SpiUartWriteTxData( (uint32) dati[i] ) ;
            crcTx = crc1021(crcTx, dati[i]) ;
        }
    }

    postambolo(crcTx) ;
}

void TMDP_err(
    TMD_CMD cmd,
    uint8_t err)
{
    uint16 crcTx = preambolo(2, cmd, err) ;

    postambolo(crcTx) ;
}

#else
// Manca la seriale

void TMDP_ini(void)
{}

void TMDP_enter_deep(void)
{}

void TMDP_leave_deep(void)
{}

void TMDP_send(
    TMD_CMD a,
    const void * b,
    uint16_t c)
{
    UNUSED(a) ;
    UNUSED(b) ;
    UNUSED(c) ;
}

void TMDP_err(
    TMD_CMD a,
    uint8_t b)
{
    UNUSED(a) ;
    UNUSED(b) ;
}

void TMDP_irq(bool x)
{
    UNUSED(x) ;
}

#endif
