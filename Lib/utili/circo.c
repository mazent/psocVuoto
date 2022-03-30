//#define STAMPA_DBG
#include "circo.h"
#include "soc/utili.h"

#define RIEMPITIVO_DI_DBG		0xCC

static uint16_t incr(uint16_t x, uint16_t y, uint16_t l)
{
    x += y ;

    return x % l ;
}

bool CIRCO_ins(S_CIRCO * pC, const uint8_t * dati, uint16_t dim)
{
    bool esito = true ;

    do {
#ifdef DEBUG
        if (NULL == pC) {
            break ;
        }

        if (NULL == dati) {
            break ;
        }

        if (0 == dim) {
            break ;
        }
#endif
        if (0 == pC->tot) {
            pC->leggi = 0 ;
        }

        const uint16_t DIM_CIRCO = pC->DIM_CIRCO ;
        if (dim > DIM_CIRCO) {
            // Non ci staranno mai: copio gli ultimi
            DBG_ERR ;

            esito = false ;

            pC->tot = DIM_CIRCO ;
            pC->leggi = 0 ;
            dati += dim - DIM_CIRCO ;
            memcpy(pC->buf, dati, DIM_CIRCO) ;

            // Fatto
            break ;
        }

        const uint16_t LIBERI = DIM_CIRCO - pC->tot ;
        if (dim > LIBERI) {
            // I primi li perdo
            DBG_ERR ;

            esito = false ;

            pC->leggi = incr(pC->leggi, dim - LIBERI, DIM_CIRCO) ;

            // Adesso ho spazio: procedo
        }

        // Primo posto dove inserire i nuovi
        uint16_t scrivi = incr(pC->leggi, pC->tot, DIM_CIRCO) ;

        if (1 == dim) {
            pC->buf[scrivi] = *dati ;
            pC->tot++ ;
        }
        else if (pC->leggi >= scrivi) {
            //     s      l
            // ****.......****
            memcpy(pC->buf + scrivi, dati, dim) ;
            pC->tot += dim ;
        }
        else {
            //     l      s
            // ....*******....
            const uint16_t CODA = MIN(dim, DIM_CIRCO - scrivi) ;
            memcpy(pC->buf + scrivi, dati, CODA) ;
            pC->tot += CODA ;

            dim -= CODA ;
            if (dim) {
                memcpy(pC->buf, dati + CODA, dim) ;
                pC->tot += dim ;
            }
        }
    } while (false) ;

    return esito ;
}

uint16_t CIRCO_est(S_CIRCO * pC, uint8_t * dati, uint16_t dim)
{
    uint16_t letti = 0 ;

    do {
#ifdef DEBUG
        if (NULL == pC) {
            break ;
        }

        if (NULL == dati) {
            break ;
        }

        if (0 == dim) {
            break ;
        }
#endif
        if (0 == pC->tot) {
            break ;
        }

        if (dim > pC->tot) {
            dim = pC->tot ;
        }

        if (1 == pC->tot) {
            *dati = pC->buf[pC->leggi] ;
            pC->leggi = incr(pC->leggi, 1, pC->DIM_CIRCO) ;
            pC->tot-- ;
            letti = 1 ;
            break ;
        }

        while (dim) {
            // In un colpo posso leggere o fino alla fine:
            //                l   U
            //     ****.......****
            // o fino al totale:
            //         l      U
            //     ....*******....
            const uint16_t ULTIMO = MIN(pC->DIM_CIRCO, pC->leggi + pC->tot) ;
            const uint16_t DIM = MIN(dim, ULTIMO - pC->leggi) ;
            if (1 == DIM) {
                *dati = pC->buf[pC->leggi] ;
            }
            else {
                memcpy(dati, pC->buf + pC->leggi, DIM) ;
            }
#ifndef NDEBUG
            memset(pC->buf + pC->leggi, RIEMPITIVO_DI_DBG, DIM) ;
#endif
            dati += DIM ;
            letti += DIM ;
            pC->tot -= DIM ;

            pC->leggi = incr(pC->leggi, DIM, pC->DIM_CIRCO) ;
            dim -= DIM ;
        }
    } while (false) ;

    return letti ;
}

uint16_t CIRCO_est2(S_CIRCO * pC, uint8_t finoa, uint8_t * dati, uint16_t dim)
{
    uint16_t letti = 0 ;

    do {
#ifdef DEBUG
        if (NULL == pC) {
            break ;
        }

        if (NULL == dati) {
            break ;
        }

        if (0 == dim) {
            break ;
        }
#endif
        if (0 == pC->tot) {
            break ;
        }

        if (dim > pC->tot) {
            dim = pC->tot ;
        }

        // L'elemento finale ? si puo' trovare:
        // 1) a destra
        //                l ?  U
        //     ****.......*****		leggo da l a ?
        // 2) a sinistra
        //      ?  S      l    U
        //     ****.......*****     leggo a destra + da 0 a ?
        // 3) in mezzo
        //         l  ?   U
        //     ....*******.....     leggo da l a ?
        // 4) da nessuna parte
        const uint16_t ULTIMO = MIN(pC->DIM_CIRCO, pC->leggi + pC->tot) ;
        const uint16_t DIM = MIN(dim, ULTIMO - pC->leggi) ;
        uint8_t * l = pC->buf + pC->leggi ;
        uint8_t * fa = memchr(l, finoa, DIM) ;
        if (fa) {
            // caso (1) o (3)
            letti = fa - l ;

            memcpy(dati, l, letti) ;
#ifndef NDEBUG
            memset(l, RIEMPITIVO_DI_DBG, letti + 1) ;
#endif
            pC->tot -= letti + 1 ;
            pC->leggi = incr(pC->leggi, letti + 1, pC->DIM_CIRCO) ;
        }
        else if (ULTIMO < pC->DIM_CIRCO) {
            // caso (4)
        }
        else {
            const uint16_t SINISTRA = pC->leggi + pC->tot - pC->DIM_CIRCO ;
            fa = memchr(pC->buf, finoa, SINISTRA) ;
            if (fa) {
                // caso (2)
                // tutta la destra
                memcpy(dati, l, DIM) ;
#ifndef NDEBUG
                memset(l, RIEMPITIVO_DI_DBG, DIM) ;
#endif
                dati += DIM ;
                letti += DIM ;

                // fino a ?
                ptrdiff_t diff = ((uint8_t *) fa) - pC->buf ;

                letti += diff ;
                memcpy(dati, pC->buf, diff) ;
#ifndef NDEBUG
                memset(pC->buf, RIEMPITIVO_DI_DBG, diff + 1) ;
#endif

                pC->tot -= letti + 1 ;
                pC->leggi = incr(pC->leggi, letti + 1, pC->DIM_CIRCO) ;
            }
            else {
                // caso (4)
            }
        }
    } while (false) ;

    return letti ;
}
