#ifndef BLE_CFG_H_
#define BLE_CFG_H_

// Se definita, abilita cambio nome
// La dimensione deve essere quella dei 'gap settings' (altrimenti viene tagliato)
#define BLE_MAX_NDD		11

// Una delle due
//#define ABIL_BLE_NOTIF		1
//#define ABIL_BLE_INDIC		1

//#define BLE_AUTEN			1

//#define BLE_AUTOR			1

//#define BLE_ADV_INVIATO		1

#else
#	warning ble_cfg.h incluso
#endif
