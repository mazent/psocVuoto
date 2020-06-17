#ifndef STAMPA_H_
#define STAMPA_H_

/*
 * Usa una seriale sw per le stampe
 *
 * La seriale deve chiamarsi UDI
 * (cfr schematico "diario": disabilitando la pagina si eliminano le stampe)
 */

// La seriale sw non va inizializzata
#define STAMPA_NO_INIZ		1


#ifdef CY_SW_TX_UART_UDI_H
#	define STAMPA_ABIL			1

#	define STAMPA_printf		DBG_printf
#	define STAMPA_puts			DBG_puts
#	define STAMPA_putchar		DBG_putchar
#	define STAMPA_print_hex		DBG_print_hex
#endif

#endif
