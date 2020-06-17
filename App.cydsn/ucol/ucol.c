#include "ucol.h"

#ifdef CY_SCB_UCO_H

#include "soc/soc.h"
#include "utili/circo.h"

void tp6(bool alto) ;

// Evito di eseguire l'esame dei dati nella isr
// 115200 b/s == 11520 B/s == 1152 B/10ms
#define SW_TO_RX		10

static UCOL_RX_CB cbRx = NULL ;

static void tx_cr_lf(void)
{
	UCO_UartPutChar('\r') ;
	UCO_UartPutChar('\n') ;
}

static void tx_str(const char * riga)
{
	UCO_UartPutString(riga) ;
}

static void tx_car(char c)
{
	UCO_UartPutChar(c) ;
}

static bool dbg_abil = true ;

void DBG_ctrl(bool abil)
{
	dbg_abil = abil ;
}

#if 1

// Se avete ram
static char ucol_buf[300] ;

void DBG_printf_nocrlf(const char * fmt, ...)
{
	if (dbg_abil) {
		va_list args;

		va_start(args, fmt) ;

		vsnprintf(ucol_buf, sizeof(ucol_buf), fmt, args) ;

		tx_str(ucol_buf) ;

		va_end(args);
	}
}

void DBG_printf(const char * fmt, ...)
{
	if (dbg_abil) {
		va_list args;

		va_start(args, fmt) ;

		vsnprintf(ucol_buf, sizeof(ucol_buf), fmt, args) ;

		tx_str(ucol_buf) ;
		tx_cr_lf() ;

		va_end(args);
	}
}

#else

// Se non avete ram
void DBG_printf(const char * fmt, ...)
{
	if (dbg_abil) {
		char tmp[10] ;
		va_list prm ;

		va_start(prm, fmt) ;

		while (true) {
			const char * perc = strchr(fmt, '%') ;
			if (NULL == perc) {
				UCO_UartPutString(fmt) ;
				break ;
			}
			else {
				while (fmt != perc) {
					UCO_UartPutChar(*fmt) ;
					fmt++ ;
				}

				perc++ ;
				fmt = perc + 1 ;

				switch (*perc) {
				case 's': {
						const char * val = va_arg(prm, const char *) ;
						UCO_UartPutString(val) ;
					}
					break ;
				case 'u': {
						unsigned int val = va_arg(prm, unsigned int) ;
						sprintf(tmp, "%u", val) ;
						UCO_UartPutString(tmp) ;
					}
					break ;
				case 'd': {
						int val = va_arg(prm, int) ;
						sprintf(tmp, "%d", val) ;
						UCO_UartPutString(tmp) ;
					}
					break ;
				case 'c': {
						int val = va_arg(prm, int) ;
						UCO_UartPutChar(val) ;
					}
					break ;
				case 'X':
				case 'x': {
						unsigned int val = va_arg(prm, unsigned int) ;
						sprintf(tmp, "%X", val) ;
						UCO_UartPutString(tmp) ;
					}
					break ;
				default:
	                (void) va_arg(prm, int) ;
					UCO_UartPutString("?NV?") ;
					break ;
				}
			}
		}

		va_end(prm) ;

		UCO_UartPutChar('\r') ;
		UCO_UartPutChar('\n') ;
	}
}

#endif

void DBG_print_hex(const char * titolo, const void * v, const int dim)
{
	if (dbg_abil) {
		const uint8_t * msg = v ;

		if (titolo) {
			tx_str(titolo) ;
		}

		char tmp[8] ;
		(void) sprintf(tmp, "[%d] ", dim) ;
		tx_str(tmp) ;

		for (int i=0 ; i<dim ; i++) {
			(void) sprintf(tmp, "%02X ", msg[i]) ;
			tx_str(tmp) ;
		}

		tx_cr_lf() ;
	}
}

void DBG_puts_nocrlf(const char * x)
{
	if (dbg_abil) {
		tx_str(x) ;
	}
}


void DBG_puts(const char * x)
{
	if (dbg_abil) {
		tx_str(x) ;
		tx_cr_lf() ;
	}
}

void DBG_putchar(char c)
{
	if (dbg_abil) {
		tx_car(c) ;
	}
}

void UCOL_iniz(void)
{
    UCO_Start() ;
    UCO_SetRxInterruptMode(0) ;
    UCO_SetTxInterruptMode(0) ;
}

#if UCO_INTERNAL_RX_SW_BUFFER
	// Riceve cypress
#define FINE_DATI_MS    10

static void passa_dati(void * v)
{
	UNUSED(v) ;

	while (true) {
	    const uint16 DIM = UCO_SpiUartGetRxBufferSize() ;

        if (0 == DIM) {
            break ;
        }

	    for (uint16_t i = 0 ; i < DIM ; ++i) {
	        char rx = UCO_SpiUartReadRxData() ;
	        cbRx(&rx, 1) ;
	    }
	}
}

static void ucol_isr(void)
{
	timer_start(TIM_UCOL, FINE_DATI_MS) ;
}

void UCOL_rx_ini(UCOL_RX_CB cb)
{
	if (cb) {
		timer_setcb(TIM_UCOL, passa_dati) ;

		UCO_SetCustomInterruptHandler(ucol_isr) ;
		UCO_SetRxInterruptMode(IRQ_RX) ;
	}
	else {
		timer_setcb(TIM_UCOL, NULL) ;

		UCO_SetRxInterruptMode(0) ;
		UCO_SetCustomInterruptHandler(NULL) ;
	}

	cbRx = cb ;
}

#else
	// Ricevo io
#define MAX_BUFF        UCOL_RX_BUFF

static union {
    S_CIRCO c ;
    uint8_t b[sizeof(S_CIRCO) - 1 + MAX_BUFF] ;
} u ;

// dimensione del buffer per passaggio dati
#define DIM_TMP_RX		100


static const uint32_t IRQ_ERR = UCO_INTR_RX_OVERFLOW | UCO_INTR_RX_FRAME_ERROR | UCO_INTR_RX_PARITY_ERROR ;
static const uint32_t IRQ_RX = UCO_INTR_RX_FIFO_LEVEL | UCO_INTR_RX_NOT_EMPTY | UCO_INTR_RX_FULL ;

static void ucol_isr(void)
{
	uint32_t irq = UCO_GetRxInterruptSourceMasked() ;

	tp6(true) ;

	// Ricezione
	if (irq & IRQ_ERR) {
		// In caso di errore me ne accorgo dal crc
		UCO_SpiUartClearRxBuffer() ;
	}
	else if (irq & IRQ_RX) {
	    uint32_t level = UCO_SpiUartGetRxBufferSize() ;

	    while (level) {
	        uint32_t temp = UCO_SpiUartReadRxData() ;
	        (void) CIRCO_ins(&u.c, (uint8_t *) &temp, 1) ;
	        level = UCO_SpiUartGetRxBufferSize() ;
	    }

	    timer_start(TIM_UCOL, SW_TO_RX) ;
	}

	UCO_ClearRxInterruptSource(irq) ;

	tp6(false) ;
}

// Quando scatta il t.o. recupero i dati e li passo

static void passa_dati(void * v)
{
	static uint8_t rx[DIM_TMP_RX] ;

	UNUSED(v) ;

	do {
		if ( 0 == CIRCO_dim(&u.c) ) {
			break ;
		}

		while (true) {
			uint16_t dim ;

			// Maschero interruzioni
			uint32_t x = UCO_GetRxInterruptMode() ;
			UCO_SetRxInterruptMode(0) ;

			// Estraggo i byte ricevuti
			dim = CIRCO_est(&u.c, rx, DIM_TMP_RX) ;

			// Ripristino
			UCO_SetRxInterruptMode(x) ;

			// Esamino
			if (dim)
				cbRx(rx, dim) ;
			else
				break ;
		}
	} while (false) ;
}

void UCOL_rx_ini(UCOL_RX_CB cb)
{
	if (cb) {
		CIRCO_iniz(&u.c, MAX_BUFF) ;

		timer_setcb(TIM_UCOL, passa_dati) ;

		uint32_t rx = UCO_GetRxInterruptMode() ;

		UCO_SetCustomInterruptHandler(ucol_isr) ;
		UCO_SetRxInterruptMode(IRQ_RX) ;

		rx = UCO_GetRxInterruptMode() ;

		ASSERT(IRQ_RX == rx) ;
	}
	else {
		timer_setcb(TIM_UCOL, NULL) ;

		UCO_SetRxInterruptMode(0) ;
		UCO_SetCustomInterruptHandler(NULL) ;
	}

	cbRx = cb ;
}

#endif

void UCOL_enter_deep(void)
{
	UCO_Sleep() ;
}

void UCOL_leave_deep(void)
{
	UCO_Wakeup() ;
}

void UCOL_tx(const void * v, int d)
{
	UCO_SpiUartPutArray(v, d) ;
}

#else

// Manca la seriale

void UCOL_iniz(void) {}

void UCOL_enter_deep(void) {}
void UCOL_leave_deep(void) {}

void UCOL_tx(const void * v, int d)
{
	UNUSED(v) ;
	UNUSED(d) ;
}

void UCOL_rx_ini(UCOL_RX_CB cb)
{
	UNUSED(cb) ;
}

#endif
