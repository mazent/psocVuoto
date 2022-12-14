#ifndef UCOL_H_
#define UCOL_H_

#include <stdint.h>

/*
 * La seriale di collaudo viene usata anche per debug!
 *
 * Le stampe di debug sono in formato testo e non interferiscono
 * con la comunicazione
 *
 * Il s.o. cooperativo serializza tutto: basta non stampare nelle isr
 *
 */

// Dimensione del massimo pacchetto ricevibile
#ifndef UCOL_RX_BUFF
#	define UCOL_RX_BUFF		300
#endif

// Solo stampe debug
void UCOL_iniz(void) ;

// Power management
void UCOL_enter_deep(void) ;
void UCOL_leave_deep(void) ;

// Le stampe vanno su seriale, per cui possono
// essere abilitate anche in release
#ifdef CY_SCB_UCO_H
	// Voglio le stampe su seriale!
#	define VLSSS		1
void DBG_print_hex(const char *, const void *, int) ;
void DBG_printf(const char *, ...) ;
void DBG_puts(const char *) ;
void DBG_putchar(char) ;
// per cypress
void DBG_printf_nocrlf(const char *, ...) ;
void DBG_puts_nocrlf(const char *) ;

	// Posso [dis]abilitare a run-time
	// Interruttore A in SC681/doc/doc.html
void DBG_ctrl(bool) ;
#else
	// No, grazie
#endif

// Invocata quando si riceve qualcosa
typedef void (*UCOL_RX_CB)(void *, uint16_t) ;

// Se si passa NULL si termina la ricezione
void UCOL_rx_ini(UCOL_RX_CB) ;

void UCOL_tx(const void *, int) ;

#endif
