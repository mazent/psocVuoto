#ifndef LIB_UTILI_VARIE_H_
#define LIB_UTILI_VARIE_H_

// Utile
bool v_strtol(long *, const char *, int) ;

// CWE-120
void copia_str(char * restrict, size_t, const char * restrict) ;
int stampa_str(char * restrict, size_t, const char * restrict, ...) ;

#else
#   warning varie.h incluso
#endif
