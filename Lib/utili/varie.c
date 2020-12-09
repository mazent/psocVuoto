#define STAMPA_DBG
#include "includimi.h"

bool v_strtol(long * p, const char * s, int base)
{
    bool esito = false ;

    do {
        if (NULL == p) {
            DBG_ERR ;
            break ;
        }

        if (NULL == s) {
            DBG_ERR ;
            break ;
        }

        char * str_end = NULL ;
        long prog = strtol(s, &str_end, base) ;
        if (str_end == s) {
            DBG_ERR ;
            break ;
        }

        *p = prog ;
        esito = true ;
    } while (false) ;

    return esito ;
}

// https://cwe.mitre.org/data/definitions/120.html

void copia_str(char * restrict dst, size_t bufsz, const char * restrict srg)
{
    (void) snprintf(dst, bufsz, "%s", srg) ;
}

int stampa_str(char * restrict buffer,
               size_t bufsz,
               const char * restrict fmt,
               ...)
{
    va_list args ;

    va_start(args, fmt) ;

    int dim = vsnprintf(buffer, bufsz, fmt, args) ;

    va_end(args) ;

    return dim ;
}
