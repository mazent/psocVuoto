#ifndef UFR_H_
#define UFR_H_

#include "utili/includimi.h"

/******************************************
 * Gestione delle user flash rows
 ******************************************/

#define UFR_NUMR	4

bool UFR_iniz(void) ;

const void * UFR_dati(uint8_t numr) ;

bool UFR_read(uint8_t numr, void *) ;
bool UFR_write(uint8_t numr, const void *) ;

/******************************************
 * Interazione con BLE (definire UFR_E_BLE
 * e opzionalmente il timer TIM_UFR)
 * https://community.cypress.com/thread/52519
 ******************************************/

// Invocare quando si puo' scrivere, p.e. alla
// fine dell'advertisement
void UFR_flush(void);

// Torna vero se i dati sono stati salvati in ufr
bool UFR_flushed(void) ;

/****************************************
	la prima e' riservata al mac-address che,
	assieme ad altri parametri, viene scritto
	durante il collaudo

	Si puo' quindi usare per salvare anche gli altri parametri,
	basta che la struttura inizi come ESEMPIO_RIGA_0

	https://community.cypress.com/message/220592
	https://community.cypress.com/docs/DOC-9287

	typedef struct {
		CYBLE_GAP_BD_ADDR_T mac ;

		uint8_t inutilizzabili[UFR_DIM - sizeof(CYBLE_GAP_BD_ADDR_T)] ;
	} ESEMPIO_RIGA_0 ;

****************************************/


/* test
#define TEST_UFR_NUMR		0

uint8_t rif[UFR_DIM] ;
uint8_t mem[UFR_DIM] ;

static void riempi(uint8_t val)
{
	for (int i=0 ; i<UFR_DIM; ++i, ++val)
		rif[i] = val ;
}

static void ipmeir(uint8_t val)
{
	for (int i=0 ; i<UFR_DIM; ++i, ++val)
		rif[i] = ~val ;
}

static void UFR_test(void)
{
	if ( !UFR_iniz() ) {
		BPOINT ;
	}

	uint8_t val = 7 ;

	while (true) {
		DBG_PRINTF("Provo con %02X", val) ;

		// scrivo un valore diverso in ogni locazione
		riempi(val) ;

		if ( !UFR_write(TEST_UFR_NUMR, rif) ) {
			BPOINT ;
		}

		if ( !UFR_read(TEST_UFR_NUMR, mem) ) {
			BPOINT ;
		}

		if (0 != memcmp(rif, mem, UFR_DIM)) {
			BPOINT ;
		}

		// Ripeto col negato
		ipmeir(val) ;

		if ( !UFR_write(TEST_UFR_NUMR, rif) ) {
			BPOINT ;
		}

		if ( !UFR_read(TEST_UFR_NUMR, mem) ) {
			BPOINT ;
		}

		if (0 != memcmp(rif, mem, UFR_DIM)) {
			BPOINT ;
		}

		// altro giro
		++val ;
	}
}
*/
#endif
