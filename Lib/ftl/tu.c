#define STAMPA_DBG
#include "ftl.h"
#include "unity/unity.h"

/*
 * Modificare il progetto:
 *     *) aggiungere unity
 *     *) togliere app.c
 *     *) aggiungere questo file
 *     *) definire UNITY_INCLUDE_CONFIG_H
 */


//#define IN_RAM		1

static bool cancellata = false ;
static bool sbaglia_erase = false ;
static bool esito_erase ;

static uint32_t dove ;


#ifdef IN_RAM

static uint8_t blocco1[FTL_BLK_DIM] ;
static uint8_t blocco2[FTL_BLK_DIM] ;

static void flash_iniz(void) {}

static bool cancella_flash(BLK_T blk)
{
	uint8_t * blocco = blocco1 ;

	if (FTL_BLK_2 == blk)
		blocco = blocco2 ;

	memset(blocco, 0xFF, FTL_BLK_DIM) ;

	return true ;
}

static bool scrivi_flash(BLK_T blk, size_t ofs, const void * v, size_t dim)
{
	uint8_t * blocco = blocco1 ;

	if (FTL_BLK_2 == blk)
		blocco = blocco2 ;

	dove = ofs ;

	memcpy(blocco + ofs, v, dim) ;

	return true ;
}

static bool leggi_flash(BLK_T blk, size_t ofs, void * v, size_t dim)
{
	uint8_t * blocco = blocco1 ;

	if (FTL_BLK_2 == blk)
		blocco = blocco2 ;

	return memcpy(v, blocco + ofs, dim) ;
}

#else

#	include "xmem/xmem.h"

static void flash_iniz(void)
{
	XMEM_iniz() ;
}

static bool cancella_flash(BLK_T blk)
{
	uint32_t ind = blk * XMEM_SECTOR_SIZE ;

	return XMEM_erase_sector(ind) ;
}

static bool scrivi_flash(BLK_T blk, size_t ofs, const void * v, size_t dim)
{
	uint32_t ind = blk * XMEM_SECTOR_SIZE ;

	ind += ofs ;

	dove = ind ;

	return XMEM_write(ind, v, dim) ;
}

static bool leggi_flash(BLK_T blk, size_t ofs, void * v, size_t dim)
{
	uint32_t ind = blk * XMEM_SECTOR_SIZE ;

	ind += ofs ;

	return XMEM_read(ind, v, dim) ;
}

#endif


static bool cancella(BLK_T blk)
{
	cancellata = true ;

	if (sbaglia_erase) {
		sbaglia_erase = false ;

		esito_erase = false ;
	}
	else {
		esito_erase = true ;
		cancella_flash(blk) ;
	}

	return esito_erase ;
}

static bool cancella_falso(BLK_T blk)
{
    UNUSED(blk) ;

	cancellata = true ;

	return false ;
}


static const FTL_OP fop = {
	.pfRead = leggi_flash,
	.pfWrite = scrivi_flash,
	.pfErase = cancella,
} ;

static const FTL_OP fop2 = {
	.pfRead = leggi_flash,
	.pfWrite = scrivi_flash,
	.pfErase = cancella_falso,
} ;

static uint8_t bufl[FTL_LSECT_DIM] ;
static uint8_t dati1[FTL_LSECT_DIM] ;
static uint8_t dati2[FTL_LSECT_DIM] ;

static void crea_dati(uint8_t * dati)
{
	static unsigned int seme = 436676279 ;

	srand(seme) ;

	for (int i=0 ; i<FTL_LSECT_DIM ; i++)
		dati[i] = rand() & 0xFF ;

	seme++ ;
}

static void aggiorna_dati(uint8_t * dati)
{
	for (int i=0 ; i<FTL_LSECT_DIM ; i++)
		dati[i]++ ;
}

static bool uguali(const uint8_t * sx, const uint8_t * dx, size_t dim)
{
	size_t i ;

	for (i=0 ; i<dim ; i++) {
		if (sx[i] != dx[i])
			break ;
	}

	return i == dim ;
}

void setUp(void)
{
}

void tearDown(void)
{
	FTL_fine() ;
}

void t1(void)
{
	FTL_fine() ;

	// Prima di tutto inizializzare
	TEST_ASSERT_FALSE( FTL_write(0, dati1) ) ;
	TEST_ASSERT_FALSE( FTL_read(0, bufl) ) ;
}

void t2(void)
{
	// Se la flash e' vuota
	TEST_ASSERT( cancella_flash(FTL_BLK_1) ) ;
	TEST_ASSERT( cancella_flash(FTL_BLK_2) ) ;

	TEST_ASSERT( FTL_iniz(&fop) ) ;

	// Non trovo elementi
	for (uint16_t virt = 0 ; virt<FTL_LSECT_NUM ; virt++) {
		TEST_ASSERT_FALSE( FTL_read(virt, bufl) ) ;
	}
}

void t3(void)
{
	crea_dati(dati1) ;

	TEST_ASSERT( FTL_iniz(&fop) ) ;

	// Se scrivo qualcosa
	TEST_ASSERT( FTL_write(0, dati1) ) ;

	// E chiudo
	FTL_fine() ;

	// Quando riapro
	TEST_ASSERT( FTL_iniz(&fop) ) ;

	// La ritrovo
	TEST_ASSERT( FTL_read(0, bufl) ) ;
	TEST_ASSERT( uguali(dati1, bufl, FTL_LSECT_DIM) ) ;

	// Ma se la elimino
	TEST_ASSERT( FTL_write(0, NULL) ) ;

	// E chiudo
	FTL_fine() ;

	// Quando riapro
	TEST_ASSERT( FTL_iniz(&fop) ) ;

	// Non c'e' piu'
	TEST_ASSERT_FALSE( FTL_read(0, bufl) ) ;
}

void t4(void)
{
	crea_dati(dati1) ;
	crea_dati(dati2) ;

	TEST_ASSERT( FTL_iniz(&fop) ) ;

	for (uint16_t virt1 = 0 ; virt1<FTL_LSECT_NUM ; virt1++) {
		// Se scrivo una cosa da una parte
		TEST_ASSERT( FTL_write(virt1, dati1) ) ;

		// E una cosa diversa negli altri settori
		for (uint16_t virt2 = 0 ; virt2<FTL_LSECT_NUM ; virt2++) {
			if (virt2 == virt1)
				continue ;
			TEST_ASSERT( FTL_write(virt2, dati2) ) ;
		}

		// La ritrovo
		TEST_ASSERT( FTL_read(virt1, bufl) ) ;
		TEST_ASSERT( uguali(dati1, bufl, FTL_LSECT_DIM) ) ;

		aggiorna_dati(dati1) ;
		aggiorna_dati(dati2) ;
	}
}

void scrivi_settori(void)
{
	int canc = 0 ;
	const int CANC = 3 ;
	uint16_t virt1 = 0 ;

	crea_dati(dati1) ;

	// Scrivo fino ad avere piu' erase dei due blocchi
	while (canc < CANC) {
		bool esito = FTL_write(virt1, dati1) ;
		if (!cancellata)
			TEST_ASSERT(esito) ;
		else if (esito_erase)
			TEST_ASSERT(esito) ;
		else
			TEST_ASSERT_FALSE(esito) ;

		if (cancellata) {
			canc++ ;
			cancellata = false ;
		}

		if (esito) {
			TEST_ASSERT( FTL_read(virt1, bufl) ) ;
			TEST_ASSERT_MESSAGE( uguali(dati1, bufl, FTL_LSECT_DIM), "diversi" ) ;
		}

		virt1++ ;
		virt1 %= FTL_LSECT_NUM ;

		aggiorna_dati(dati1) ;
	}
}

void t5(void)
{
	TEST_ASSERT( FTL_iniz(&fop) ) ;

	scrivi_settori() ;
}

void t6(void)
{
	TEST_ASSERT( FTL_iniz(&fop) ) ;

	// Simulo errore erase
	sbaglia_erase = true ;
	scrivi_settori() ;
}

void t7(void)
{
	uint16_t virt ;
	uint16_t virt1 = 0 ;

	TEST_ASSERT( cancella_flash(FTL_BLK_1) ) ;
	TEST_ASSERT( cancella_flash(FTL_BLK_2) ) ;

	// Lascio dati in giro
	TEST_ASSERT( FTL_iniz(&fop2) ) ;

	crea_dati(dati1) ;

	while (true) {
		bool e = FTL_write(virt1, dati1) ;
		if (e) {
			virt = virt1 ;
			memcpy(dati2, dati1, FTL_LSECT_DIM) ;

			TEST_ASSERT( FTL_read(virt, bufl) ) ;
			TEST_ASSERT( uguali(dati2, bufl, FTL_LSECT_DIM) ) ;
		}
		else
			break ;

		virt1++ ;
		virt1 %= FTL_LSECT_NUM ;
		aggiorna_dati(dati1) ;
	}

	FTL_fine() ;

	// Metto a posto
	TEST_ASSERT( FTL_iniz(&fop) ) ;

	TEST_ASSERT( FTL_read(virt, bufl) ) ;
	TEST_ASSERT( uguali(dati2, bufl, FTL_LSECT_DIM) ) ;
}

#if 1

void app_ini(void)
{
	flash_iniz() ;

	UNITY_BEGIN() ;

	RUN_TEST(t1) ;
	RUN_TEST(t2) ;
	RUN_TEST(t3) ;
	RUN_TEST(t4) ;
	RUN_TEST(t5) ;
	RUN_TEST(t6) ;
	RUN_TEST(t7) ;

	UNITY_END() ;

	DBG_PUTS("fine tu") ;
}

#else

int main(void)
{
	UNITY_BEGIN() ;

	RUN_TEST(t1) ;
	RUN_TEST(t2) ;
	RUN_TEST(t3) ;
	RUN_TEST(t4) ;
	RUN_TEST(t5) ;
	RUN_TEST(t6) ;
	RUN_TEST(t7) ;

	return UNITY_END() ;
}


void STAMPA_printf(const char * fmt, ...)
{
	char dbg_buf[200] ;
	va_list args;

	va_start(args, fmt) ;

	vsnprintf(dbg_buf, sizeof(dbg_buf), fmt, args) ;

	puts(dbg_buf) ;

	va_end(args);
}


void STAMPA_puts(const char * x)
{
	puts(x) ;
}

void STAMPA_putchar(char c)
{
	putchar(c) ;
}

void STAMPA_print_hex(const char * titolo, const void * v, const int dim)
{
	char dbg_buf[2000] = {0} ;
	char * buf = dbg_buf ;
	const uint8_t * msg = v ;

	if (titolo) {
		strcpy(dbg_buf, titolo) ;
		buf += strlen(titolo) ;
	}

	char tmp[8] ;
	(void) sprintf(tmp, "[%d] ", dim) ;
	strcpy(buf, tmp) ;
	buf += strlen(tmp) ;

	for (int i=0 ; i<dim ; i++) {
		(void) sprintf(tmp, "%02X ", msg[i]) ;
		strcpy(buf, tmp) ;
		buf += strlen(tmp) ;
	}

	puts(dbg_buf) ;
}

void * soc_malloc(size_t dim)
{
    return malloc(dim) ;
}

void * soc_calloc(size_t num, size_t size)
{
	return calloc(num, size) ;
}

void soc_free(void * v)
{
	if (v)
		free(v) ;
}
#endif
