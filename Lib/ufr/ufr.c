#define STAMPA_DBG
#include "ufr.h"

static bool iniz = false ;

bool UFR_iniz(void)
{
	static_assert(UFR_DIM == CY_SFLASH_SIZEOF_USERROW, "OKKIO!") ;
	static_assert(UFR_NUMR == CY_SFLASH_NUMBER_USERROWS, "OKKIO!") ;

	ASSERT(!iniz) ;

	iniz = (UFR_DIM == CY_SFLASH_SIZEOF_USERROW) && (UFR_NUMR == CY_SFLASH_NUMBER_USERROWS) ;

	return iniz ;
}

const void * UFR_dati(uint8_t numr)
{
	const void * riga = NULL ;

	ASSERT(iniz) ;

	if (!iniz) {
		DBG_ERR ;
	}
	else if (numr >= UFR_NUMR) {
		DBG_ERR ;
	}
	else
		riga = (const void *) (CY_SFLASH_USERBASE + numr * UFR_DIM) ;

	return riga ;
}

bool UFR_read(uint8_t numr, void * v)
{
	bool esito = false ;

	ASSERT(iniz) ;

	if (!iniz) {
		DBG_ERR ;
	}
	else if (numr >= UFR_NUMR) {
		DBG_ERR ;
	}
	else if (NULL == v) {
		DBG_ERR ;
	}
	else {
		void * r = (void *) (CY_SFLASH_USERBASE + numr * UFR_DIM) ;
		memcpy(v, r, UFR_DIM) ;

		esito = true ;
	}

	return esito ;
}

bool UFR_write(uint8_t numr, const void * v)
{
	bool esito = false ;

	ASSERT(iniz) ;

	if (!iniz) {
		DBG_ERR ;
	}
	else if (numr >= UFR_NUMR) {
		DBG_ERR ;
	}
	else if (NULL == v) {
		DBG_ERR ;
	}
	else {
		esito = CY_SYS_FLASH_SUCCESS == CySysSFlashWriteUserRow(numr, (const uint8 *) v) ;
		if (!esito) {
			DBG_ERR ;
		}
	}

	return esito ;
}


