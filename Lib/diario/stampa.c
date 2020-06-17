#include "utili/includimi.h"

#ifdef CY_SW_TX_UART_UDI_H


#if 1

// Se avete ram
static char dbg_buf[200] ;

void STAMPA_printf(const char * fmt, ...)
{
	va_list args;

	va_start(args, fmt) ;

	vsnprintf(dbg_buf, sizeof(dbg_buf), fmt, args) ;

	UDI_PutString(dbg_buf) ;
	UDI_PutCRLF() ;

	va_end(args);
}

#else

// Se non avete ram
void STAMPA_printf(const char * fmt, ...)
{
	char tmp[10] ;
	va_list prm ;

	va_start(prm, fmt) ;

	while (true) {
		const char * perc = strchr(fmt, '%') ;
		if (NULL == perc) {
			UDI_PutString(fmt) ;
			break ;
		}
		else {
			while (fmt != perc) {
				UDI_PutChar(*fmt) ;
				fmt++ ;
			}

			perc++ ;
			fmt = perc + 1 ;

			switch (*perc) {
			case 's': {
					const char * val = va_arg(prm, const char *) ;
					UDI_PutString(val) ;
				}
				break ;
			case 'u': {
					unsigned int val = va_arg(prm, unsigned int) ;
					sprintf(tmp, "%u", val) ;
					UDI_PutString(tmp) ;
				}
				break ;
			case 'd': {
					int val = va_arg(prm, int) ;
					sprintf(tmp, "%d", val) ;
					UDI_PutString(tmp) ;
				}
				break ;
			case 'c': {
					int val = va_arg(prm, int) ;
					UDI_PutChar(val) ;
				}
				break ;
			case 'X':
			case 'x': {
					unsigned int val = va_arg(prm, unsigned int) ;
					sprintf(tmp, "%X", val) ;
					UDI_PutString(tmp) ;
				}
				break ;
			default:
                (void) va_arg(prm, int) ;
				UDI_PutString("?NV?") ;
				break ;
			}
		}
	}

	va_end(prm) ;

	UDI_PutCRLF() ;
}

#endif


void STAMPA_print_hex(const char * titolo, const void * v, const int dim)
{
	const uint8_t * msg = v ;

	if (titolo) {
		UDI_PutString(titolo) ;
	}

	char tmp[8] ;
	(void) sprintf(tmp, "[%d] ", dim) ;
	UDI_PutString(tmp) ;

	for (int i=0 ; i<dim ; i++) {
		(void) sprintf(tmp, "%02X ", msg[i]) ;
		UDI_PutString(tmp) ;
	}

	UDI_PutCRLF() ;
}

void STAMPA_puts(const char * x)
{
	UDI_PutString(x) ;

	UDI_PutCRLF() ;
}

void STAMPA_putchar(char c)
{
	UDI_PutChar(c) ;
}


#else

// Manca la seriale

#endif
