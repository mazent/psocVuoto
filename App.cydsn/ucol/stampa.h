#ifndef STAMPA_H_
#define STAMPA_H_

#include "ucol.h"


#define STAMPA_iniz					UCOL_iniz
#define STAMPA_enter_deep			UCOL_enter_deep
#define STAMPA_leave_deep			UCOL_leave_deep


#ifdef CY_SCB_UCO_H
#	define STAMPA_ABIL			1

#	define STAMPA_printf		DBG_printf
#	define STAMPA_puts			DBG_puts
#	define STAMPA_putchar		DBG_putchar
#	define STAMPA_print_hex		DBG_print_hex
#endif

#endif
