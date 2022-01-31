#ifndef DBG_H_
#define DBG_H_

#include "stampa.h"

#ifdef STAMPA_NO_INIZ

#define DBG_INIZ
#define DBG_ENTER_DEEP
#define DBG_LEAVE_DEEP

#else

#define DBG_INIZ			STAMPA_iniz()
#define DBG_ENTER_DEEP		STAMPA_enter_deep()
#define DBG_LEAVE_DEEP		STAMPA_leave_deep()

#endif

// Inoltre abilito sul singolo file
#if defined( STAMPA_DBG ) && defined( STAMPA_ABIL )
#	define DBG_ABIL			1

extern void STAMPA_printf(const char * fmt, ...) ;
extern void STAMPA_puts(const char * x) ;
extern void STAMPA_putchar(char c) ;
extern void STAMPA_print_hex(const char * titolo, const void * v, const int dim) ;

#	define DBG_FUN						STAMPA_puts(__func__)
#	define DBG_QUA						STAMPA_printf("QUA %s %d", __FILE__, __LINE__)
#	define DBG_ERR						STAMPA_printf("ERR %s %d", __FILE__, __LINE__)
#	define DBG_ASSERT					STAMPA_printf("ASSERT %s %d", __FILE__, __LINE__)
#	define DBG_PRINTF(f, ...)			STAMPA_printf(f, ##__VA_ARGS__)
#	define DBG_PUTS(a)					STAMPA_puts(a)
#	define DBG_PUTCHAR(a)				STAMPA_putchar(a)
#	define DBG_PRINT_HEX(t, x, d)		STAMPA_print_hex(t, x, d)
#else
#	define DBG_FUN
#	define DBG_QUA
#	define DBG_ERR
#	define DBG_ASSERT
#	define DBG_PRINTF(f, ...)
#	define DBG_PUTS(a)
#	define DBG_PUTCHAR(a)
#	define DBG_PRINT_HEX(t, x, d)
#endif


#endif
