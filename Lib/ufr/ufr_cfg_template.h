#ifndef LIB_UFR_UFR_CFG_TEMPLATE_H_
#define LIB_UFR_UFR_CFG_TEMPLATE_H_

#include "ufr/ufr.h"

// Dipende dal dispositivo
#define UFR_DIM     256

// Definire se si scrive in ufr con ble acceso
#define UFR_E_BLE       1

#ifdef UFR_E_BLE

// Questa dovete farla voi
extern bool ufr_writable(void) ;

// Opzionale: si puo' eseguire il flush dentro advertisement
//#define TIM_UFR_MS		100

#endif

#else
#   warning ufr_cfg_template.h incluso
#endif
