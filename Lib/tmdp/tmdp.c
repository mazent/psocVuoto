#define STAMPA_DBG
#include "tmdp.h"
#include "soc/soc.h"

#ifdef CY_SCB_TBUS_H
#include "tmdp_cfg.h"
#include "circo.h"

extern uint16 crc1021(uint16 crc, uint8 val) ;

// Evito disallineamenti (il protocollo non si resincronizza)
// 19200 b/s == 1920 B/s -> 19 B/10ms
#define SW_TO_RX     200

static enum {
    STT_WAIT,
    STT_ADDR,
    STT_DIM,
    STT_DAT,
    STT_CRC_1,
    STT_CRC_2
} stato = STT_WAIT ;

// Deve poter contenere comando + risposta altrui
#define MAX_BUFF        (2 * 260)

static union {
    S_CIRCO c ;
    uint8_t b[sizeof(S_CIRCO) - 1 + MAX_BUFF] ;
} u ;

#define CRC_INI     0x00BD

#define STX     0x02

static uint8 bufRx[MAX_BUFF] ;
static TMD_ADDR curAddr ;
static uint16 crc ;
static uint8_t curDim ;
static uint8 dimRx = 0 ;

// Fra un comando e la risposta disabilito ricezione
static uint32_t rim = 0 ;
static int conta_rim = 0 ;

static void tbus_stop(void)
{
    DBG_PUTS(__func__) ;

    conta_rim += 1 ;
    if (1 == conta_rim) {
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
    bool abil = false ;

    DBG_PUTS(__func__) ;

    conta_rim -= 1 ;
    if (conta_rim < 0) {
        DBG_ERR ;
    }
    else if (0 == conta_rim) {
        abil = true ;
    }
    else {
        // ... ma se capita non abilito
        DBG_ERR ;
        DBG_PRINTF("\t%d", conta_rim) ;
    }

    if (abil) {
        // Caso mai fossero arrivati dei byte
        uint32_t value = TBUS_GetRxInterruptSourceMasked() ;
        if (value) {
            TBUS_ClearRxInterruptSource(value) ;
        }
        TBUS_SpiUartClearRxBuffer() ;

        CIRCO_svuota(&u.c) ;

        TBUS_SetRxInterruptMode(rim) ;
    }
}

// Interrupts

static const uint32_t IRQ_ERR = TBUS_INTR_RX_OVERFLOW |
                                TBUS_INTR_RX_FRAME_ERROR |
                                TBUS_INTR_RX_PARITY_ERROR ;
static const uint32_t IRQ_RX = TBUS_INTR_RX_FIFO_LEVEL |
                               TBUS_INTR_RX_NOT_EMPTY | TBUS_INTR_RX_FULL ;

CY_ISR(tbus_irq)
{
    uint32_t irq = TBUS_GetRxInterruptSourceMasked() ;

    if (irq & IRQ_ERR) {
        // In caso di errore me ne accorgo dal crc
        TBUS_SpiUartClearRxBuffer() ;
    }
    else if (irq & IRQ_RX) {
        uint32_t level = TBUS_SpiUartGetRxBufferSize() ;

        while (level) {
            uint32_t temp = TBUS_SpiUartReadRxData() ;
            (void) CIRCO_ins(&u.c, (uint8_t *) &temp, 1) ;
            level = TBUS_SpiUartGetRxBufferSize() ;
        }

        // La periferica sveglia il processore da deep-sleep ma l'interruzione
        // scatta dopo: il timer del main serve a non tornare subito in
        // deep-sleep
        // Questo ha un altro scopo (vedi sopra)
        timer_start(TIM_TMDP, SW_TO_RX) ;
    }
    else {
        // ???
    }

    TBUS_ClearRxInterruptSource(irq) ;
}

static void purge_rx(void)
{
    uint32 level = TBUS_SpiUartGetTxBufferSize() ;

    while (level) {
        CyDelay(1) ;
        level = TBUS_SpiUartGetTxBufferSize() ;
    }

    // shift register
    CyDelay(1) ;

    // riabilito la ricezione
    tbus_restart() ;
}

static void esamina(const uint8 * d, uint16_t dim)
{
    for (uint16_t j = 0 ; j < dim ; j++) {
        uint8 rx = d[j] ;

        switch (stato) {
        case STT_WAIT:
            if (rx == STX) {
                stato = STT_ADDR ;
            }
            break ;
        case STT_ADDR:
            if (rx != STX) {
                curAddr = rx ;
                crc = crc1021(CRC_INI, curAddr) ;
                stato = STT_DIM ;
            }
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
            if (dimRx >= curDim) {
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
            if (crc != 0) {     // NOLINT(bugprone-branch-clone)
            }
            else if (curAddr != TMDP_ADDR) {
            }
            else {
                // basta comandi fino alla risposta
                tbus_stop() ;

                // esco
                j = dim ;

                // Eseguo
                TMDP_CMD(bufRx[0], bufRx + 2, dimRx - 2) ;
            }

            stato = STT_WAIT ;
            break ;

        default:
            //DBG_ERR ;
            stato = STT_WAIT ;
            break ;
        }
    }
}

static void tmdp_to(void)
{
    // Riparto
    stato = STT_WAIT ;
}

/**********************************
        Interfaccia
**********************************/

void TMDP_ini(void)
{
    timer_setcb(TIM_TMDP, tmdp_to) ;

    CIRCO_iniz(&u.c, MAX_BUFF) ;

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
    if (x) {
        TBUS_INT_Write(1) ;
    }
    else {
        TBUS_INT_Write(0) ;
    }
}

void TMDP_poll(void)
{
    static uint8_t rx[MAX_BUFF] ;

    if ( CIRCO_dim(&u.c) ) {
        while (true) {
            uint16_t dim ;

            // Maschero interruzioni
            uint32_t x = TBUS_GetRxInterruptMode() ;
            TBUS_SetRxInterruptMode(0) ;

            // Estraggo i byte ricevuti
            dim = CIRCO_est( &u.c, rx, sizeof(rx) ) ;

            // Ripristino
            TBUS_SetRxInterruptMode(x) ;

            // Esamino
            if (dim) {
                DBG_PRINT_HEX("tbus ->", rx, dim) ;
                esamina(rx, dim) ;
            }
            else {
                break ;
            }
        }
    }
}

static uint16_t preambolo(uint8_t dim, TMD_CMD cmd, uint8_t esito)
{
    uint16_t crcTx = CRC_INI ;
    uint8_t cosa[] = {
        STX, STX, TMDP_ADDR, dim, cmd, esito
    } ;

    DBG_PRINT_HEX( "tbus <-", cosa, sizeof(cosa) ) ;

    for (size_t i = 2 ; i < sizeof(cosa) ; ++i) {
        crcTx = crc1021(crcTx, cosa[i]) ;
    }

    for (size_t i = 0 ; i < sizeof(cosa) ; ++i) {
        TBUS_SpiUartWriteTxData( (uint32) cosa[i] ) ;
    }

    return crcTx ;
}

static void postambolo(uint16_t crc)
{
    union {
        uint16_t crc ;
        uint8_t b[sizeof(uint16_t)] ;
    } u ;

    // invio crc big endian
    u.crc = __REV16(crc) ;
    DBG_PRINT_HEX( "tbus <-", u.b, sizeof(crc) ) ;

    for (size_t i = 0 ; i < sizeof(crc) ; ++i) {
        TBUS_SpiUartWriteTxData( (uint32) u.b[i] ) ;
    }

    purge_rx() ;
}

void TMDP_send(TMD_CMD cmd, const void * v, uint16_t dim)
{
    const uint8_t * dati = (const uint8_t *) v ;
    uint16 crcTx = preambolo(dim + 2, cmd, TBUS_OK) ;

    if (dim) {
        DBG_PRINT_HEX("tbus <-", dati, dim) ;

        for (uint16_t i = 0 ; i < dim ; i++) {
            TBUS_SpiUartWriteTxData( (uint32) dati[i] ) ;
            crcTx = crc1021(crcTx, dati[i]) ;
        }
    }

    postambolo(crcTx) ;
}

void TMDP_err(TMD_CMD cmd, uint8_t err)
{
    uint16 crcTx = preambolo(2, cmd, err) ;

    postambolo(crcTx) ;
}

#else
// Manca la seriale

void TMDP_ini(TMD_ADDR a, PF_TMD_CMD b)
{
}

void TMDP_enter_deep(void)
{
}

void TMDP_leave_deep(void)
{
}

void TMDP_poll(void)
{
}

void TMDP_send(TMD_CMD a, const void * b, uint16_t c)
{
}

void TMDP_err(TMD_CMD a, uint8_t b)
{
}

#endif
