#define STAMPA_DBG
#include "app.h"
#include "soc/soc.h"

void HW_iniz(void)
{
}

void HW_sleep(void)
{
}

void HW_wake(void)
{
}

#define TO_LED_MS       200

static void lampeggia(void * v)
{
    uint8_t tmp ;
    UNUSED(v) ;

    tmp = ROSSO_NEG_Read() ;
    tmp ^= 1 ;
    ROSSO_NEG_Write(tmp) ;

    tmp = VERDE_NEG_Read() ;
    tmp ^= 1 ;
    VERDE_NEG_Write(tmp) ;

    tmp = BLU_NEG_Read() ;
    tmp ^= 1 ;
    BLU_NEG_Write(tmp) ;

    timer_start(TIM_LED, TO_LED_MS) ;
}

void app_ini(void)
{
	DBG_PUTS("CIAO!") ;

    timer_setcb(TIM_LED, lampeggia) ;
    timer_start(TIM_LED, TO_LED_MS) ;
}
