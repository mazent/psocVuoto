#ifndef BLE_CFG_H_
#define BLE_CFG_H_

// Definire per Extended Range
// p.e. CYBLE-212006-01
// Nota
// The CYBLE-212006-01 module is certified for FCC, ISED, CE, MIC and KC
// regulations at an output power of +7.5 dBm. To achieve this output power,
// RFO2 (PSoC 4 BLE silicon PA level) must be set to the -12 dBm setting in
// firmware. Settings higher than this will result in higher output power than
// specified in the CYBLE-212006-01 certifications
//#define BLE_EXTENDED_RANGE		1

// Se definita, accende e spegne ECO
//#define BLE_CLOCK		1

// Se definita, abilita il cambio nome
// La dimensione deve essere quella dei 'gap settings' (altrimenti viene tagliato)
// (senza lo 0 finale)
//#define BLE_MAX_NDD		11

// Definire se si usano write long
//#define ABIL_BLE_WR_LONG		1

// Definire se si usano le notifiche
//#define ABIL_BLE_NOTIF	1

// Definire se si usano le indicazioni
//#define ABIL_BLE_INDIC	1

// Definire per abilitare autenticazione con passkey
//#define BLE_AUTEN			1

// Definire se le caratteristiche sono autorizzate in scrittura
//#define BLE_AUTOR			1

// Definire per abilitare l'observer
//#define BLE_OBSERVER		1

// Definire per avere la callback adv_inviato,
// che ha come scopo:
//    1) ottimizzare il consumo evitando timer pari al tempo di adv
//    2) aggiornare advertisement (https://community.cypress.com/message/36753)
//    3) aggiornare user flash row (https://community.cypress.com/thread/52519
//		 e https://community.cypress.com/message/36753)
// 52519 consiglia anche di alzare la priorita' dell'interruzione ble
// (BT_bless_isr = 0)
//#define BLE_ADV_INVIATO		1

// Seriale LE (definirle tutte)
// Occorre anche implementare sle_ricevi_cb
// Usare SLE_trasmetti per trasmettere
//#define SERIALE_LE_HANDLE_DATI		CYBLE_SERIALEBLE_DATI_CHAR_HANDLE
//#define SERIALE_LE_HANDLE_CONFIG	CYBLE_SERIALEBLE_CONTROLLO_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE
//#define SERIALE_LE_HANDLE_CTRL		CYBLE_SERIALEBLE_CONTROLLO_CHAR_HANDLE

#else
#   warning ble_cfg_template.h incluso
#endif
