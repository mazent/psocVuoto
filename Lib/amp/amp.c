#define STAMPA_DBG
#include "amp.h"

#include "../utili/crc1021.h"

#define FIRMA		0xC0DECA05

#define BOOT_BL		0x8099D27C

#define PRD_VALIDO		(1 << 0)
#define PKY_VALIDO		(1 << 1)

#define CRC_INI		0xCAB2

#pragma pack(1)

typedef struct {
	uint32_t firma ;

	uint8_t validi ;

	// Resto?
	uint32_t boot ;

	// p.e. TKQJT001582
	char codice_prodot[PRM_MAX_DIM_COD_PRD + 1] ;

	// Da 0 a 999999
	uint32_t passkey ;

	uint16_t crc ;
} S_AMP ;

#pragma pack()

static CY_NOINIT S_AMP amp ;

static void agg_crc(void)
{
	uint16_t crc = CRC_1021_v(CRC_INI, &amp, sizeof(S_AMP) - sizeof(uint16_t)) ;
	amp.crc = __REV16(crc) ;
}

void AMP_iniz(void)
{
	bool valido = false ;

	if (FIRMA != amp.firma) {
		// Malissimo
		DBG_ERR ;
	}
	else if (0 != CRC_1021_v(CRC_INI, &amp, sizeof(S_AMP)) ) {
		// Male
		DBG_ERR ;
	}
	else {
		// Ottimo
		valido = true ;
		DBG_PUTS("AMP valida") ;
	}

	if (!valido) {
		// Valori predefiniti
		memset(&amp, 0, sizeof(S_AMP)) ;

		amp.firma = FIRMA ;

		agg_crc() ;
	}
}

bool AMP_resta_nel_bl(void)
{
	return amp.boot == BOOT_BL ;
}

void AMP_forza_nel_bl(bool si)
{
	amp.boot = si ? BOOT_BL : 0 ;

	agg_crc() ;
}

void AMP_prd_s(const char * s, int8_t dim)
{
	if (0 == dim)
		s = NULL ;
	else if (dim > PRM_MAX_DIM_COD_PRD) {
		DBG_ERR ;
		return ;
	}

	memset(amp.codice_prodot, 0, PRM_MAX_DIM_COD_PRD + 1) ;
	if (NULL == s) {
		amp.validi &= NOT(PRD_VALIDO) ;
	}
	else {
		memcpy(amp.codice_prodot, s, dim) ;

		amp.validi |= PRD_VALIDO ;
	}

	agg_crc() ;
}

const char * AMP_prd_l(void)
{
	const char * s = NULL ;

	if (amp.validi & PRD_VALIDO) {
		s = amp.codice_prodot ;
	}

	return s ;
}

void AMP_pkey_s(const uint32_t * s)
{
	if (NULL == s) {
		amp.validi &= NOT(PKY_VALIDO) ;
	}
	else {
		amp.passkey = *s ;

		amp.validi |= PKY_VALIDO ;
	}

	agg_crc() ;
}

const uint32_t * AMP_pkey_l(void)
{
	const uint32_t * s = NULL ;

	if (amp.validi & PKY_VALIDO) {
		s = &amp.passkey ;
	}

	return s ;
}

