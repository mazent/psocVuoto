#define STAMPA_DBG
#include "time2.h"

// L'epoca e' riferita all'inizio di quest'anno
#define TIME2_ABASE   2020
#define TIME2_EBASE   ( (time_t) 1577836800 )

// Arrivo fino al: 31 December 2087 23:59:59
#define TIME2_AMAX	 2087
static const uint64_t TIME2_EMAX = 3723753599 ;


/*
 * Ogni 28 anni i giorni si ripetono nella stessa sequenza
 * (esclusi i secolari non bisestili)
 */

#define PERIODO_ANNI 		28

#define UN_GIORNO 			((uint64_t) 24 * 60 * 60)
static const uint64_t PERIODO_S = (365 * 4 + 1) * 7 * UN_GIORNO ;

// Fino a qua uso le classiche
#define MAX_TM_YEAR     (2037 - 1900)
static const time_t MAX_STD_EPOCH = 0x7FFFFFFF ;

time_t mktime2( struct tm * pBT )
{
    if (pBT->tm_year < MAX_TM_YEAR) {
    	// Ottimo ...
    	time_t risul = mktime(pBT) ;

    	// ... ma devo togliere
    	risul -= TIME2_EBASE ;

    	return risul ;
    }
    else {
    	int anni = 0 ;
        union {
            time_t e ;
            uint64_t tmp ;
        } u = {
            .tmp = 0
        } ;

        // Passo ad un anno valido precedente
        while (pBT->tm_year > MAX_TM_YEAR) {
            u.tmp += PERIODO_S ;
            pBT->tm_year -= PERIODO_ANNI ;
            anni += PERIODO_ANNI ;
        }

        // Converto
        time_t risul = mktime(pBT) ;

        // Ripristino
        pBT->tm_year += anni ;

        // Torno all'epoca vera
        u.tmp += risul ;

        // Resto a 32 bit eliminando il riferimento
        u.tmp -= TIME2_EBASE ;

        return u.e ;
    }
}

struct tm * gmtime2( const time_t * pE )
{
    union {
        time_t e ;
        uint64_t tmp ;
    } u = {
        .e = *pE
    } ;

    // Aggiungo il riferimento
    u.tmp += TIME2_EBASE ;

    if (u.tmp < MAX_STD_EPOCH) {
    	// Ottimo
        return gmtime(&u.e) ;
    }
    else if (u.tmp > TIME2_EMAX) {
    	// Troppo in avanti
    	return NULL ;
    }
    else {
        int anni = 0 ;

        // Mi sposto di periodo
        while (u.tmp >= MAX_STD_EPOCH) {
            anni += PERIODO_ANNI ;
            u.tmp -= PERIODO_S ;
        }

        // Converto
        struct tm * pBT = gmtime(&u.e) ;

        // Aggiorno
        pBT->tm_year += anni ;

        return pBT ;
    }
}

#if 0

static void stampa_bt(const char * t, time_t e, struct tm * bt)
{
	const char * giorno = NULL ;

	switch (bt->tm_wday) {
	case 0: giorno = "dom" ; break ;
	case 1: giorno = "lun" ; break ;
	case 2: giorno = "mar" ; break ;
	case 3: giorno = "mer" ; break ;
	case 4: giorno = "gio" ; break ;
	case 5: giorno = "ven" ; break ;
	case 6: giorno = "sab" ; break ;
	default: giorno = "???" ; break ;
	}

	DBG_PRINTF("%s: %08X = %d/%02d/%02d (%03d - %s) %02d:%02d:%02d",
			t, e,
			bt->tm_year + 1900, bt->tm_mon + 1, bt->tm_mday,
			bt->tm_yday + 1, giorno,
			bt->tm_hour, bt->tm_min, bt->tm_sec) ;
}

void test_time2(void)
{
	// epoche (rispetto a TIME2_ABASE)
    time_t vE[] = {
    	// i due estremi
    	0, TIME2_EMAX - TIME2_EBASE,
		// Date casuali
        229912419, 170989625, 420657641, 887311720, 630765354,
        26017356, 207671914, 118435256, 438003565, 955897159,
    } ;
    const int MAX_ANNO = TIME2_AMAX - 1900 ;

    for (size_t i = 0 ; i < DIM_VETT(vE) ; ++i) {
        // Deve essere coerente
        struct tm bt = *( gmtime2(vE + i) ) ;
        stampa_bt("Provo", vE[i], &bt) ;
        time_t e = mktime2(&bt) ;
        ASSERT(e == vE[i])  ;

        // Deve superare il 2038 e arrivare al 2087
        struct tm prox = bt ;
        time_t eprox = e ;
        while (true) {
            // sommo un periodo
            prox.tm_year += PERIODO_ANNI ;
            eprox += PERIODO_S ;
            if (prox.tm_year > MAX_ANNO) {
                break ;
            }

            // passo a epoca
            time_t ep = mktime2(&prox) ;
            // devono essere uguali
            ASSERT(ep == eprox) ;

            // torno a broken time
            struct tm * pBT = gmtime2(&ep) ;
            stampa_bt("\tavanti", ep, pBT) ;

            // devono essere uguali (tranne l'anno)
            ASSERT(pBT->tm_sec  == prox.tm_sec ) ;
            ASSERT(pBT->tm_min  == prox.tm_min ) ;
            ASSERT(pBT->tm_hour == prox.tm_hour) ;
            ASSERT(pBT->tm_mday == prox.tm_mday) ;
            ASSERT(pBT->tm_mon  == prox.tm_mon ) ;
            ASSERT(pBT->tm_wday == prox.tm_wday) ;
            ASSERT(pBT->tm_yday == prox.tm_yday) ;
        }
    }
}

#endif
