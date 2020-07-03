#include "ftl.h"
#include "bit.h"
#include "soc/soc.h"

// LRU indica anche queste cose
#define LRU_LIBERO      ( (LRU_T) NOT(0) )
#define LRU_NONVALE     ( (LRU_T) 0 )

// Ogni blocco contiene settori fisici
#pragma pack(1)

typedef struct {
	// Settore logico associato
	SECT_T lgc ;

	// Dati del settore logico
	// Eventuale crc implememntato a cura delle
	// funzioni di lettura e scrittura in flash (nel campo dati)
	uint8_t dati[FTL_LSECT_DIM] ;

	// Deve essere l'ultimo dato scritto
	LRU_T lru ;

} FTL_SECT_F ;

#pragma pack()

// Numero di settori fisici
#define FTL_FSECT_NUM 		(FTL_BLK_DIM / sizeof(FTL_SECT_F))

// Un bit per ogni settore fisico
#define FTL_FSECT_BYTE      ( (FTL_FSECT_NUM + 7) / 8 )


static const FTL_OP * pOp = NULL ;

static BLK_T blkCorr ;

static uint8_t liberi[FTL_FSECT_BYTE] ;

typedef struct {
    LRU_T lru ;
    BLK_T blk ;
    SECT_T fis ;
} FTL_SECT_L ;

static FTL_SECT_L vLog[FTL_LSECT_NUM] ;

static LRU_T incr_lru(LRU_T lru)
{
    ++lru ;

    if ((LRU_LIBERO == lru) || (LRU_NONVALE == lru)) {
        lru = 1 ;
    }

    return lru ;
}

/*
    un settore e' piu' recente di un altro se la differenza fra i loro
    progressivi e' minore o uguale del numero di settori fisici dei due blocchi

    Aggiungo 2 perche' esiste LRU_LIBERO e LRU_NONVALE
*/

#define LRU_MAX (2 * FTL_FSECT_NUM + 2)

static bool sx_segue_dx(LRU_T sx, LRU_T dx)
{
	LRU_T diff = sx - dx ;

    return diff <= LRU_MAX ;
}


static bool uguale_a(const void * v, uint8_t val, const size_t dim)
{
    const uint8_t * dati = v ;
    size_t i ;

    for (i = 0 ; i < dim ; ++i) {
        if (dati[i] != val) {
            break ;
        }
    }

    return i == dim ;
}

static void esamina_blocco(uint8_t * liberi, BLK_T blk)
{
    FTL_SECT_F * pF = soc_malloc( sizeof(FTL_SECT_F) ) ;
    if (NULL != pF) {
        size_t ofs = 0 ;

//        DBG_PRINTF("blocco %d", blk) ;

        // Tutto usato
        BIT_zeri(liberi, FTL_FSECT_NUM) ;

        for ( size_t fis = 0 ; fis < FTL_FSECT_NUM ;
              ++fis, ofs += sizeof(FTL_SECT_F) ) {
            if ( !pOp->pfRead(blk, ofs, pF, sizeof(FTL_SECT_F)) ) {
                DBG_ERR ;
            }
            else if (LRU_NONVALE == pF->lru ) {
                // Usato
//            	DBG_PRINTF("\t %d: usato", fis) ;
            }
            else if (LRU_LIBERO == pF->lru ) {
            	if ( uguale_a( pF, 0xFF, sizeof(FTL_SECT_F) ) ) {
            		// Libero!
//            		DBG_PRINTF("\t %d: libero", fis) ;
            		BIT_uno(liberi, fis) ;
            	}
            	else {
            		DBG_PRINTF("\t %d: usato", fis) ;
            	}
            }
            else if (pF->lgc >= FTL_LSECT_NUM) {
                DBG_ERR ;
            }
            else {
                // Valido!
                if (LRU_LIBERO == vLog[pF->lgc].lru) {
                    // Prima volta
//                	DBG_PRINTF("\t %d: primo %d", fis, pF->lgc) ;
                    vLog[pF->lgc].lru = pF->lru ;
                    vLog[pF->lgc].blk = blk ;
                    vLog[pF->lgc].fis = fis ;
                }
                else if ( sx_segue_dx(pF->lru, vLog[pF->lgc].lru) ) {
                    // Ho trovato una versione piu' recente
//                	DBG_PRINTF("\t %d: nuovo %d", fis, pF->lgc) ;
                    vLog[pF->lgc].lru = pF->lru ;
                    vLog[pF->lgc].blk = blk ;
                    vLog[pF->lgc].fis = fis ;
                }
                else {
                    // Ho trovato una versione piu' vecchia
//                	DBG_PRINTF("\t %d: vecchio %d", fis, pF->lgc) ;
                }
            }
        }
    }

    soc_free(pF) ;
}

static void esamina_blocco_cancellato(uint8_t * liberi, BLK_T blk)
{
    FTL_SECT_F * pF = soc_malloc( sizeof(FTL_SECT_F) ) ;
    if (NULL != pF) {
        size_t ofs = 0 ;

        // Tutti occupati
        BIT_zeri(liberi, FTL_FSECT_NUM) ;

        for ( size_t fis = 0 ; fis < FTL_FSECT_NUM ;
              ++fis, ofs += sizeof(FTL_SECT_F) ) {
            if ( !pOp->pfRead(blk, ofs, pF, sizeof(FTL_SECT_F)) ) {
                DBG_ERR ;
            }
            else if ( uguale_a( pF, 0xFF, sizeof(FTL_SECT_F) ) ) {
                // Libero!
            	BIT_uno(liberi, fis) ;
            }
            else if (LRU_NONVALE == pF->lru ) {
                // Usato?
                DBG_ERR ;
            }
            else if (pF->lgc >= FTL_LSECT_NUM) {
                DBG_ERR ;
            }
            else {
                // Valido?
                DBG_ERR ;
            }
        }
    }

    soc_free(pF) ;
}

// Copia tutti i settori meno uno

static bool copia_tranne(BLK_T blk, SECT_T lgc)
{
    bool esito = false ;
    FTL_SECT_F * pF = soc_malloc( sizeof(FTL_SECT_F) ) ;
    if (NULL != pF) {
        size_t i ;
        for (i = 0 ; i < FTL_LSECT_NUM ; ++i) {
        	if (i == lgc) {
        		// Il chiamante cancella il blocco
        		vLog[i].lru = LRU_LIBERO ;
        		continue ;
        	}

            if (LRU_LIBERO == vLog[i].lru) {
                continue ;
            }

            // Lo leggo
            size_t ofsl = sizeof(FTL_SECT_F) * vLog[i].fis ;
            if ( !pOp->pfRead(vLog[i].blk, ofsl, pF, sizeof(FTL_SECT_F)) ) {
                DBG_ERR ;
                break ;
            }

            // Mi procuro un settore libero
            size_t fis = BIT_primo_uno(liberi, FTL_FSECT_NUM) ;
            if (FTL_FSECT_NUM == fis) {
                DBG_ERR ;
                break ;
            }

            // Non e' piu' libero
            BIT_zero(liberi, fis) ;

            // Lo aggiorno
            pF->lru = incr_lru(pF->lru) ;

            // Lo scrivo
            size_t ofss = sizeof(FTL_SECT_F) * fis ;
            if ( !pOp->pfWrite(blk, ofss, pF, sizeof(FTL_SECT_F)) ) {
                DBG_ERR ;
                break ;
            }

            // Aggiorno dati in ram
            vLog[i].lru = pF->lru ;
            vLog[i].blk = blk ;
            vLog[i].fis = fis ;
        }

        esito = i == FTL_LSECT_NUM ;
    }

    soc_free(pF) ;
    return esito ;
}

static bool elimina_fis(BLK_T blk, SECT_T fis)
{
	const LRU_T nv = LRU_NONVALE ;
	size_t ofs = sizeof(FTL_SECT_F) * fis ;

	ofs += offsetof(FTL_SECT_F, lru) ;

	if ( !pOp->pfWrite(blk, ofs, &nv, sizeof(LRU_T)) ) {
		DBG_ERR ;
		return false ;
	}
	else
		return true ;
}

// Trasferisce i settori logici nel blocco indicato
// sul blocco corrente

static bool copia_da(BLK_T blk)
{
    bool esito = false ;
    FTL_SECT_F * pF = soc_malloc( sizeof(FTL_SECT_F) ) ;
    if (NULL != pF) {
        size_t i ;
        for (i = 0 ; i < FTL_LSECT_NUM ; ++i) {
            if (LRU_LIBERO == vLog[i].lru) {
                continue ;
            }

            if (blk != vLog[i].blk) {
                continue ;
            }

            // Eccolo!

            // Lo leggo
            size_t ofsl = sizeof(FTL_SECT_F) * vLog[i].fis ;
            if ( !pOp->pfRead(blk, ofsl, pF, sizeof(FTL_SECT_F)) ) {
                DBG_ERR ;
                break ;
            }

            // Mi procuro un settore libero
            size_t fis = BIT_primo_uno(liberi, FTL_FSECT_NUM) ;
            if (FTL_FSECT_NUM == fis) {
                DBG_ERR ;
                break ;
            }

            // Lo aggiorno
            pF->lru = incr_lru(pF->lru) ;

            // Lo scrivo
            size_t ofss = sizeof(FTL_SECT_F) * fis ;
            if ( !pOp->pfWrite(blkCorr, ofss, pF, sizeof(FTL_SECT_F)) ) {
                DBG_ERR ;
                break ;
            }
#if 0
            // Elimino l'info dal blocco precedente
            if (!elimina_fis(vLog[i].blk, vLog[i].fis)){
                // Segnalo, ma non e' cosi' grave
                DBG_ERR ;
            }
#endif
            // Aggiorno dati in ram
            vLog[i].lru = pF->lru ;
            vLog[i].blk = blkCorr ;
            vLog[i].fis = fis ;
        }

        esito = i == FTL_LSECT_NUM ;
    }

    soc_free(pF) ;
    return esito ;
}

static bool elimina_log(SECT_T sl)
{
	bool esito = false ;

	if (LRU_LIBERO == vLog[sl].lru) {
		// Il settore non esiste
		esito = true ;
	}
	else {
		vLog[sl].lru = LRU_LIBERO ;

		esito = elimina_fis(vLog[sl].blk, vLog[sl].fis) ;
	}

	return esito ;
}

static size_t alloca_settore(SECT_T tranne)
{
	size_t fis = BIT_primo_uno(liberi, FTL_FSECT_NUM) ;
	if (FTL_FSECT_NUM == fis) {
		BLK_T altro ;

		if (FTL_BLK_1 == blkCorr)
			altro = FTL_BLK_2 ;
		else
			altro = FTL_BLK_1 ;

		do {
			// Creo spazio
			if (!pOp->pfErase(altro)) {
				DBG_ERR ;
				break ;
			}

            esamina_blocco_cancellato(liberi, altro) ;

            if (!copia_tranne(altro, tranne)) {
				DBG_ERR ;
				break ;
			}

			if (!pOp->pfErase(blkCorr)) {
				// Non preoccupiamoci
				DBG_ERR ;
			}

            blkCorr = altro ;

            // Adesso deve risuscire
            fis = BIT_primo_uno(liberi, FTL_FSECT_NUM) ;
		} while (false) ;
	}

	return fis ;
}

bool FTL_iniz(const FTL_OP * op)
{
    bool esito = false ;

    ASSERT(op) ;
    ASSERT(NULL == pOp) ;

    static_assert(FTL_LSECT_NUM < FTL_FSECT_NUM, "AUMENTARE BLOCCO") ;
    static_assert( ((((LRU_T) NOT(0)) >> 1) - 2) > FTL_FSECT_NUM, "AUMENTARE LRU_T") ;
    static_assert( ((SECT_T) NOT(0)) > FTL_FSECT_NUM, "AUMENTARE SECT_T") ;
    static_assert( ((BLK_T) NOT(0)) > FTL_BLK_1, "AUMENTARE BLK_T") ;
    static_assert( ((BLK_T) NOT(0)) > FTL_BLK_2, "AUMENTARE BLK_T") ;

    DBG_PRINTF("%d settori fisici", FTL_FSECT_NUM) ;

    do {
        if (NULL == op) {
        	DBG_ERR ;
        	break ;
        }

        if (pOp) {
        	DBG_ERR ;
        	break ;
        }

        pOp = op ;

        for (size_t i = 0 ; i < FTL_LSECT_NUM ; ++i) {
            vLog[i].lru = LRU_LIBERO ;
        }

        // Esamino i due blocchi
        esamina_blocco(liberi, FTL_BLK_1) ;
        const size_t LIB_1 = BIT_quanti_uni(liberi, FTL_FSECT_NUM) ;

        uint8_t liberi2[FTL_FSECT_BYTE] ;
        esamina_blocco(liberi2, FTL_BLK_2) ;
        const size_t LIB_2 = BIT_quanti_uni(liberi2, FTL_FSECT_NUM) ;

        esito = true ;

        // Come sono messo?
        if ( (FTL_FSECT_NUM == LIB_1) && (FTL_FSECT_NUM == LIB_2) ) {
            // Entrambi vuoti
            blkCorr = FTL_BLK_1 ;
        }
        else if (FTL_FSECT_NUM == LIB_2) {
            blkCorr = FTL_BLK_1 ;
        }
        else if (FTL_FSECT_NUM == LIB_1) {
            blkCorr = FTL_BLK_2 ;
            memcpy(liberi, liberi2, FTL_FSECT_BYTE) ;
        }
        else {
        	BLK_T altro ;

            // Prendo quello con piu' blocchi liberi
            if (LIB_1 > LIB_2) {
                blkCorr = FTL_BLK_1 ;
                altro = FTL_BLK_2 ;
            }
            else {
                blkCorr = FTL_BLK_2 ;
                altro = FTL_BLK_1 ;
            }

            // Trasferisco
            if (!copia_da(altro))
            	break ;

            // Cancello
            esito = pOp->pfErase(altro) ;
        }

        if (!esito)
        	pOp = NULL ;

    } while (false) ;

    return esito ;
}

void FTL_fine(void)
{
	pOp = NULL ;
}

bool FTL_read(SECT_T logic, void * v)
{
	bool esito = false ;
	FTL_SECT_F * pF = NULL ;

//	DBG_PRINTF("%s(%d, %p)", __func__, logic, v) ;

	do {
		if (NULL == pOp) {
			// Mai inizializzata
			DBG_ERR ;
			break ;
		}

		if (NULL == v) {
			DBG_ERR ;
			break ;
		}

		if (LRU_LIBERO == vLog[logic].lru) {
			// Il settore non esiste
			DBG_ERR ;
			break ;
		}

		pF = soc_malloc( sizeof(FTL_SECT_F) ) ;
		if (NULL == pF) {
			DBG_ERR ;
			break ;
		}

        size_t ofs = sizeof(FTL_SECT_F) * vLog[logic].fis ;
        if ( !pOp->pfRead(blkCorr, ofs, pF, sizeof(FTL_SECT_F)) ) {
            DBG_ERR ;
            break ;
        }

        memcpy(v, pF->dati, FTL_LSECT_DIM) ;
        esito = true ;

	} while (false) ;

	soc_free(pF) ;

	return esito ;
}

bool FTL_write(SECT_T logic, const void * v)
{
	bool esito = false ;
	FTL_SECT_F * pF = NULL ;

//	DBG_PRINTF("%s(%d, %p)", __func__, logic, v) ;

	do {
		if (NULL == pOp) {
			// Mai inizializzata
			DBG_ERR ;
			break ;
		}

		if (NULL == v) {
			esito = elimina_log(logic) ;
			break ;
		}

		pF = soc_malloc( sizeof(FTL_SECT_F) ) ;
		if (NULL == pF) {
			DBG_ERR ;
			break ;
		}

		memcpy(pF->dati, v, FTL_LSECT_DIM) ;
		pF->lgc = logic ;
		pF->lru = incr_lru(vLog[logic].lru) ;

        // Mi procuro un settore libero
        size_t fis = alloca_settore(logic) ;
        if (FTL_FSECT_NUM == fis) {
            break ;
        }

        // Non e' piu' libero
		BIT_zero(liberi, fis) ;

        // Scrivo
        size_t ofs = sizeof(FTL_SECT_F) * fis ;
        if ( !pOp->pfWrite(blkCorr, ofs, pF, sizeof(FTL_SECT_F)) ) {
            DBG_ERR ;
            break ;
        }

        // Annullo il precedente
        if (LRU_LIBERO == vLog[logic].lru) {
        	// L'allocazione del settore ha causato il trasferimento
        	// del blocco e il suo erase
        }
        else {
        	// lru rende poco importante il fallimento
        	CHECK(elimina_fis(blkCorr, vLog[logic].fis)) ;
        }

        // Aggiorno dati in ram
        vLog[logic].lru = pF->lru ;
        vLog[logic].blk = blkCorr ;
        vLog[logic].fis = fis ;

        esito = true ;

	} while (false) ;

	soc_free(pF) ;

	return esito ;
}


