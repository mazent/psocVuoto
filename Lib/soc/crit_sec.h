#ifndef SOC_CRIT_SEC_H_
#define SOC_CRIT_SEC_H_

/*
 * Se si opera all'interno di una interruzione
 * non serve disabilitarle: le variabili sono gia' protette
 */

#define ENTER_CRITICAL_SECTION                       \
    uint32_t ipsr = __get_IPSR() ;                   \
    uint8_t interruptStatus = 0 ;                    \
    if (0 == ipsr) {                                 \
        interruptStatus = CyEnterCriticalSection() ; \
    }

#define LEAVE_CRITICAL_SECTION                       \
    if (0 == ipsr) {                                 \
        CyExitCriticalSection(interruptStatus) ;     \
    }

#else
#   warning crit_sec.h incluso
#endif
