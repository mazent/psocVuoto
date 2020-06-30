#define STAMPA_DBG
#include "xmem/xmem.h"
#include "soc/soc.h"
#include "xmem/cod/at25sf041.h"

#ifdef TIM_XMEM_A

#define NUM_PERIODI			10

static uint32_t attesa ;
static int num_periodi ;
static PF_XMA_ESITO pfEsito = NULL ;

static void block_erase(void * v)
{
	UNUSED(v) ;

    uint8_t sr1 = AT25_Read_Status_Register_1() ;
    if ( 0 == (sr1 & SR1_WIP) ) {
        pfEsito(true) ;
        pfEsito = NULL ;
    }
    else {
    	num_periodi += 1 ;
    	if (num_periodi > NUM_PERIODI) {
    		DBG_ERR ;
            pfEsito(false) ;
            pfEsito = NULL ;
    	}
    	else
    		timer_start(TIM_XMEM_A, attesa) ;
    }
}

bool XMA_erase_block(uint32_t ind, PF_XMA_ESITO cb)
{
	bool esito = false ;

	if (NULL == cb) {
		DBG_ERR ;
	}
	else if (NULL == pfEsito) {
		pfEsito = cb ;

        AT25_Write_Enable() ;

        uint8_t sr1 = AT25_Read_Status_Register_1() ;
        if ( 0 == (sr1 & SR1_WEL) ) {
            DBG_ERR ;
            pfEsito = NULL ;
        }
        else {
        	DBG_PRINTF("%s: 0x%08X", __func__, addr) ;

        	AT25_Block_Erase(addr) ;

        	num_periodi = NUM_PERIODI ;
        	attesa = MAX_BLOCK_ERASE_TIME_MS / NUM_PERIODI ;
        	timer_setcb(TIM_XMEM_A, block_erase) ;
        	timer_start(TIM_XMEM_A, attesa) ;

        	esito = true ;
        }
	}
	else {
		DBG_ERR ;
	}

	return esito ;
}


#endif
