//#define STAMPA_DBG
#include "utili.h"
#include "soc.h"

extern void HW_iniz(void) ;
extern void HW_sleep(void) ;
extern void HW_wake(void) ;

extern void app_ini(void) ;

extern void BLE_run(void) ;
extern RICH_CPU BLE_cpu(void) ;


#ifndef SOC_SPEGNI
#   define SOC_SPEGNI   0
#endif

extern void timer_ini(void) ;
extern void timer_reini(void) ;
extern void timer_run(void) ;
extern bool timer_attivi(void) ;

extern void wdog_iniz(void) ;

// Evito il deep-sleep immediato
#define DURATA_POW      20

// Livello di cpu imposto a compile-time
#ifdef NDEBUG
static const RICH_CPU CPU_CT = CPU_FERMA ;
#else
// Per debug impedisco deep-sleep
static const RICH_CPU CPU_CT = CPU_PAUSA ;
#endif

// Livello di cpu imposto a run-time
static RICH_CPU cpu_rt = CPU_FERMA ;

typedef struct {
    PF_SOC_APC apc ;
    void * arg ;
} UNA_APC ;

#ifdef MAX_NUM_APC
#   define MAX_BUFF        MAX_NUM_APC
#else
#   define MAX_BUFF        MAX_SOC_APC
#endif
static UNA_APC vAPC[MAX_BUFF] ;

static void hard_fault(void)
{
    DBG_PUTS("! HARD FAULT !") ;
    CyDelay(2) ;
#ifdef NDEBUG
    WDOG_reset() ;
#else
    __BKPT(0) ;
#endif
}

void * SOC_malloc(size_t dim)
{
    return malloc(dim) ;
}

void * SOC_calloc(
    size_t num,
    size_t size)
{
    return calloc(num, size) ;
}

void SOC_free(void * v)
{
    if ( v ) {
        free(v) ;
    }
}

static void soc_ini(void)
{
    // sostituisco while(1) con un bel reset
    (void) CyIntSetSysVector(CY_INT_HARD_FAULT_IRQN, hard_fault) ;

    timer_ini() ;
#ifdef MAX_NUM_APC
    static_assert(MAX_NUM_APC > MAX_SOC_APC, "OKKIO") ;
#endif
    for ( int i = 0 ; i < MAX_BUFF ; i++ ) {
        vAPC[i].apc = NULL ;
        vAPC[i].arg = NULL ;
    }
}

void SOC_apc_arg(
    int quale,
    PF_SOC_APC cb,
    void * arg)
{
    DYN_ASSERT( 0 == __get_IPSR() ) ;
    ASSERT(quale < MAX_SOC_APC) ;

    if ( quale < MAX_SOC_APC ) {
#ifndef MAX_NUM_APC
#   ifdef DBG_ABIL
        if ( NULL != vAPC[quale].apc ) {
            DBG_PRINTF("apcocc %d", quale) ;
        }
#   endif
        vAPC[quale].apc = cb ;
        vAPC[quale].arg = arg ;
#else
        if ( NULL == vAPC[quale].apc ) {
            // Ottimo
            vAPC[quale].apc = cb ;
            vAPC[quale].arg = arg ;
        }
        else {
            // Ne cerco una fra quelle in piu'
            int libera ;

            for ( libera = MAX_SOC_APC ; libera < MAX_NUM_APC ; ++libera ) {
                if ( NULL == vAPC[libera].apc ) {
                    break ;
                }
            }

            if ( libera < MAX_NUM_APC ) {
                vAPC[libera].apc = cb ;
                vAPC[libera].arg = arg ;
            }
#   ifdef DBG_ABIL
            else {
                DBG_PRINTF("apcocc %d", quale) ;
            }
#   endif
        }
#endif
    }
}

bool SOC_apc_attiva(int quale)
{
    bool esito = false ;
#ifndef MAX_NUM_APC
    ASSERT(quale < MAX_SOC_APC) ;
    DYN_ASSERT( 0 == __get_IPSR() ) ;

    if ( quale < MAX_SOC_APC ) {
        esito = NULL != vAPC[quale].apc ;
    }
#else
    UNUSED(quale) ;
#endif
    return esito ;
}

static void soc_run(void)
{
    for ( int i = 0 ; i < MAX_BUFF ; i++ ) {
        PF_SOC_APC apc = vAPC[i].apc ;

        if ( apc ) {
            void * arg = vAPC[i].arg ;

            vAPC[i].apc = NULL ;
            vAPC[i].arg = NULL ;

//            DBG_PRINTF("eseguo %d", i) ;
            apc(arg) ;
//            DBG_PRINTF("ougese %d", i) ;
        }
    }

    timer_run() ;
}

static RICH_CPU soc_cpu(void)
{
    RICH_CPU s = MIN(cpu_rt, CPU_CT) ;

    for ( int i = 0 ; i < MAX_BUFF ; i++ ) {
        if ( vAPC[i].apc ) {
            s = CPU_ATTIVA ;
            break ;
        }
    }

    if ( timer_attivi() ) {
        s = MIN(CPU_PAUSA, s) ;
    }

    return s ;
}

void SOC_min(RICH_CPU cpu)
{
#ifdef DBG_ABIL
    if ( cpu_rt != cpu ) {
        const char * p = NULL ;
        switch ( cpu_rt ) {
        case CPU_ATTIVA:
            p = "CPU_ATTIVA" ;
            break ;
        case CPU_PAUSA:
            p = "CPU_PAUSA" ;
            break ;
        case CPU_FERMA:
            p = "CPU_FERMA" ;
            break ;
        }

        const char * d = NULL ;
        switch ( cpu ) {
        case CPU_ATTIVA:
            d = "CPU_ATTIVA" ;
            break ;
        case CPU_PAUSA:
            d = "CPU_PAUSA" ;
            break ;
        case CPU_FERMA:
            d = "CPU_FERMA" ;
            break ;
        }
        DBG_PRINTF("%s -> %s", p, d) ;
    }
#endif
    cpu_rt = cpu ;
}

void SOC_spegni(void)
{
#if SOC_SPEGNI == 0
    // Perfetto
#elif SOC_SPEGNI == SOC_SPEGNI_STOP
    DBG_PUTS("CySysPmStop") ;
    CyDelay(1) ;
    // https://community.cypress.com/docs/DOC-10476
    if ( 0 == CY_SYS_PINS_READ_PIN(CYREG_PRT2_PS, 2) ) {
        CySysPmSetWakeupPolarity(CY_PM_STOP_WAKEUP_ACTIVE_HIGH) ;
    }
    else {
        CySysPmSetWakeupPolarity(CY_PM_STOP_WAKEUP_ACTIVE_LOW) ;
    }
    CySysPmStop() ;
#elif SOC_SPEGNI == SOC_SPEGNI_HIB
    DBG_PUTS("CySysPmHibernate") ;
    CyDelay(1) ;
    CySysPmHibernate() ;
#elif SOC_SPEGNI == SOC_SPEGNI_FHIB
    DBG_PUTS("CySysPmfreezeIo") ;
    CyDelay(1) ;
    CySysPmfreezeIo() ;
    CySysPmHibernate() ;
#endif
}

static void soc_accendi(void)
{
#if SOC_SPEGNI == 0
    // Perfetto
#elif SOC_SPEGNI == SOC_SPEGNI_STOP
    // CySysPmStop() function freezes IO-Cells implicitly.
    if ( CY_PM_RESET_REASON_WAKEUP_STOP == CySysPmGetResetReason() ) {
        CySysPmUnfreezeIo() ;
    }
#elif SOC_SPEGNI == SOC_SPEGNI_HIB
    // Grande
#elif SOC_SPEGNI == SOC_SPEGNI_FHIB
    if ( CY_PM_RESET_REASON_WAKEUP_HIB == CySysPmGetResetReason() ) {
        CySysPmUnfreezeIo() ;
    }
#endif
}

#ifdef CTRL_STACK
#   warning OKKIO
static const uint32_t STACK_NON_USATO = 0xDEADBABE ;

extern char __cy_heap_end ;
extern char __cy_stack ;
static size_t dimStack ;

static uint32_t * valid_cast(char * x)
{
    union {
        char * c ;
        uint32_t * dw ;
    } cast ;

    cast.c = x ;

    return cast.dw ;
}

static void fill_stack(void)
{
    // Riempio lo stack con un valore noto
    dimStack = (&__cy_stack - &__cy_heap_end) / sizeof(uint32_t) ;
    uint32_t * stack = valid_cast(&__cy_heap_end) ;
    for ( size_t i = 0 ; i < dimStack - 100 ; i++ ) {
        stack[i] = STACK_NON_USATO ;
    }
}

static void iniz_stack(void)
{
    uint32_t * stack = valid_cast(&__cy_heap_end) ;

    // Inizializzo
    for ( size_t i = 0 ; i < dimStack ; ++i ) {
        if ( stack[i] != STACK_NON_USATO ) {
            dimStack = i - 1 ;
            break ;
        }
    }

    DBG_PRINTF( "stack unused = %d", dimStack * sizeof(uint32_t) ) ;
}

static void runt_stack(void)
{
    // Rispetto allo stack precedente, cerco il limite attuale
    bool modif = false ;
    uint32_t * stack = valid_cast(&__cy_heap_end) ;

    while ( stack[dimStack] != STACK_NON_USATO ) {
        --dimStack ;
        modif = true ;
    }

    if ( modif ) {
        DBG_PRINTF( "stack unused = %d", dimStack * sizeof(uint32_t) ) ;
    }
}

#else

static void fill_stack(void)
{}

static void iniz_stack(void)
{}

static void runt_stack(void)
{}

#endif

// Condivisa con BL
CY_NOINIT uint32_t causa ;

E_CAUSA_RESET SOC_causa(void)
{
    return causa ;
}

bool SOC_isr(void)
{
    uint32_t ipsr = __get_IPSR() ;

    return ipsr != 0 ;
}

void SOC_reset(void)
{
    CySysWdtDisable(
        CY_SYS_WDT_COUNTER0_MASK | CY_SYS_WDT_COUNTER1_MASK
        | CY_SYS_WDT_COUNTER2_MASK) ;
    CyGlobalIntDisable ;

    DBG_PUTS("CySoftwareReset") ;
    CyDelay(2) ;

    CySoftwareReset() ;
}

int main(void)
{
    fill_stack() ;

#if defined(SOC_DBG_MALLOC) && defined(NDEBUG)
#   warning OKKIO
#endif
    soc_accendi() ;

    // Eventuale hw
    HW_iniz() ;

    // Perche' mi sono resettato?
#ifdef CY_BOOTLOADABLE_Bootloadable_H
    // Ci ha pensato il bootloader
#else
    // Devo farlo io
    causa = CySysGetResetReason(
        CY_SYS_RESET_WDT | CY_SYS_RESET_PROTFAULT | CY_SYS_RESET_SW) ;
#endif
    // Trasformo la causa prioritizzando
    if ( causa & CY_SYS_RESET_WDT ) {
        causa = E_CR_WDT ;
    }
    else if ( causa & CY_SYS_RESET_PROTFAULT ) {
        causa = E_CR_PROTFAULT ;
    }
    else if ( causa & CY_SYS_RESET_SW ) {
        causa = E_CR_SW ;
    }
    else {
        causa = E_CR_PON ;
    }

    CyGlobalIntEnable ;

    // Prima debug e s.o.
    DDB_INIZ ;

    wdog_iniz() ;
    soc_ini() ;

    // Per ultima l'applicazione
    app_ini() ;

    // Timer per evitare il deep-sleep per un poco
    timer_start(TIM_POW, DURATA_POW) ;

    iniz_stack() ;

    while ( true ) {
        runt_stack() ;

        // Avviso che va tutto bene
        WDOG_calcia() ;

        // Un giro di giostra
        soc_run() ;
        BLE_run() ;

        uint8_t interruptStatus = CyEnterCriticalSection() ;

        RICH_CPU scpu = soc_cpu() ;
        RICH_CPU bcpu = BLE_cpu() ;

        switch ( MIN(scpu, bcpu) ) {
        case CPU_ATTIVA:
            break ;
        case CPU_PAUSA: {
                bool eco =
#if 1
                    false ;
#else
                    // non funziona
                    BLE_clock() ;
#endif
                if ( eco ) {
                    /* change HF clock source from IMO to ECO, as IMO is not
                      required and can be stopped to save power */
                    CySysClkWriteHfclkDirect(CY_SYS_CLK_HFCLK_ECO) ;
                    /* stop IMO for reducing power consumption */
                    CySysClkImoStop() ;
                }

                /* put the CPU to sleep */
#ifdef CY_PINS_DBG_VRD_H
                DBG_VRD_Write(1) ;
#endif
                CySysPmSleep() ;
#ifdef CY_PINS_DBG_VRD_H
                DBG_VRD_Write(0) ;
#endif
                if ( eco ) {
                    /* starts execution after waking up, start IMO */
                    CySysClkImoStart() ;
                    /* change HF clock source back to IMO */
                    CySysClkWriteHfclkDirect(CY_SYS_CLK_HFCLK_IMO) ;
                }
            }
            break ;
        case CPU_FERMA:
            HW_sleep() ;

            DDB_ENTER_DEEP ;

            // Dormo
#ifdef CY_PINS_DBG_BLU_H
            DBG_BLU_Write(1) ;
            DBG_BLU_Sleep() ;
#endif
            CySysPmDeepSleep() ;
#ifdef CY_PINS_DBG_BLU_H
            DBG_BLU_Wakeup() ;
            DBG_BLU_Write(0) ;
#endif

            // Riabilito
            DDB_LEAVE_DEEP ;

            HW_wake() ;

            timer_start(TIM_POW, DURATA_POW) ;
            break ;
        }

        CyExitCriticalSection(interruptStatus) ;
    }

    return 0 ;
}

#ifdef CY_BOOT_INT_DEFAULT_HANDLER_ENOMEM_EXCEPTION_CALLBACK
void CyBoot_IntDefaultHandler_Enomem_Exception_Callback(void)
{
    DBG_PUTS(__func__) ;
    WDOG_reset() ;
}

#else
#   warning OKKIO AL WHILE 1
#endif

#ifdef CY_BOOT_INT_DEFAULT_HANDLER_EXCEPTION_ENTRY_CALLBACK
void CyBoot_IntDefaultHandler_Exception_EntryCallback(void)
{
    DBG_PUTS(__func__) ;
    WDOG_reset() ;
}

#else
#   warning OKKIO AL WHILE 1
#endif
