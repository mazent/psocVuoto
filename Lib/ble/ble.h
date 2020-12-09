#ifndef BLE_H_
#define BLE_H_

#include "utili/includimi.h"
#include "ble_cfg.h"

#define DIM_MAC		6
#define BLE_MAX_PASSKEY		999999

// Restituire falso se qualcosa non torna
typedef bool (*PF_BLE_WRITE)(const void *, const uint16_t) ;

typedef struct {
	uint16_t handle ;
	PF_BLE_WRITE pfWrite ;
} BLE_WRITE_CFG ;

#define BLE_ABIL_NOTIF		(1 << 0)
#define BLE_ABIL_INDIC		(1 << 1)

// Invocare prima della BLE_start (opzionale)
// Passare il vettore che associa alla caratteristica (CYBLE_XXX_CHAR_HANDLE)
// la funzione da invocare quando viene scritta
// Nota: con le CYBLE_XXX_CONFIGURATION_DESC_HANDLE il byte 0
//       indica se le notifiche sono abilitate
// Nota: BLE_stop => invocare nuovamente
void BLE_config(const BLE_WRITE_CFG *, const size_t) ;

// Invocata quando lo stack e' pronto
// Gli advertisement NON sono attivi
typedef void (*CB_BLE_ATTIVO)(void) ;

// Invocata alla [dis]connessione
// Alla sconnessione gli advertisement NON sono attivi
typedef void (*CB_BLE_CONNECT)(bool) ;

// Invocata quando qualcuno ha sbagliato la passkey
// BLE disconnette, per cui arriva anche BLE_CB.conn(false)
typedef void (*CB_BLE_AUTH_FAIL)(void) ;

// Invocata alla fine di ogni advertisement
typedef void (*CB_BLE_ADV_INVIATO)(void) ;

// Invocata ad ogni advertisement
typedef struct {
	// BLE_ADV_TYPE_xxx
	uint8_t type ;

	// BLE_MAC_TYPE_xxx
	uint8_t mtype ;
	const uint8_t * mac ;
	int8_t rssi ;

	uint8_t dim ;
	const void * dati ;
} BLE_OBS_INFO ;
typedef void (*CB_OBS_ADV)(BLE_OBS_INFO *) ;

typedef struct {
	CB_BLE_ATTIVO stack_on ;
	CB_BLE_CONNECT conn ;
	// Opzionale
	CB_BLE_AUTH_FAIL authfail ;
#ifdef BLE_OBSERVER
	CB_OBS_ADV obs_adv ;
#endif
#ifdef BLE_MAX_NDD
	char nome[BLE_MAX_NDD + 1] ;
#endif
	// Autenticazione
		// se non definita
		//	   impostare: Mode 1 - No Security
		// altrimenti
		//	   impostare: Mode 1 - Authenticated pairing with encryption
		//				  strict pairing
		//				  display
		// 0 <= passkey <= BLE_MAX_PASSKEY < disabilitata
#ifdef BLE_AUTEN
	uint32_t passkey ;
#endif
#ifdef BLE_ADV_INVIATO
	CB_BLE_ADV_INVIATO adv_inviato ;
#endif
} BLE_CB ;

void BLE_start(const BLE_CB *) ;
void BLE_stop(void) ;

// Genera 8 byte casuali
bool BLE_rand(uint8_t *) ;

// L'rng genera gli stessi numeri ad ogni accensione
// perche' il seme e' un clock interno
bool BLE_seed(uint32_t) ;

// Invocata quando BLE_IC.bda e' stato creato
typedef void (*CB_IC_OK)(void) ;

typedef struct {
	// Indirizzo Casuale
	uint8_t bda[DIM_MAC] ;

	CB_IC_OK cb ;
} BLE_IC ;

// Quando lo stack si attiva o dopo la sconnessione ...
void BLE_presentati(BLE_IC * /* anche null */) ;
void BLE_nasconditi(void) ;

// Aggiornamento caratteristiche
	// Invocare nella PF_BLE_WRITE
	// p.e. se il peer legge la risposta sulla stesso attributo
bool BLE_scrivi_attr_corr(const void *, const uint16_t) ;
	// Invocare al di fuori della PF_BLE_WRITE
	// Se invocata nella PF_BLE_WRITE fallisce
bool BLE_scrivi_attr(uint16_t, const void *, const uint16_t) ;
	// Generica
bool BLE_agg_char(uint16_t charh, bool locale, const void * dati, uint16_t dim) ;

// Abilitazione e disabilitazione servizi
// Conviene mettere per primi quelli da disabilitare
typedef struct {
	CYBLE_GATT_DB_ATTR_HANDLE_T h ;
	bool abil ;
} BLE_SRV ;
bool BLE_servizi(const BLE_SRV *, size_t) ;

// Varie
void BLE_sconnetti(void) ;
	// Autorizza l'accesso alle caratteristiche (definire BLE_AUTOR)
bool BLE_autorizza(void) ;
	// Restituisce il mac del central connesso
const void * BLE_central(void) ;
	// Legge il proprio l'indirizzo
bool BLE_mac(void *, bool public) ;

	// Notifica la caratteristica (definire ABIL_BLE_NOTIF e TIM_BLE_NTF)
	// Il messaggio deve vivere fino alla callback
	// Se si passa NULL viene annullata la notifica in corso
	// cfr spec 4.2 [Vol 3, Part F] 3.4.7.1 Handle Value Notification pag 2199
	// Nota: ble la trasmette comunque, intercettare la scrittura della
	//       CYBLE_XXX_CONFIGURATION_DESC_HANDLE per sapere se e' abilitata
typedef void (*CB_BLE_NOTIFY)(uint16_t, bool) ;
typedef struct {
	// caratteristica da notificare
	uint16_t handle ;
	// dati
	void * data ;
	uint16_t dim ;
	// timeout (ms)
	uint16_t to ;
	// opzionale
	CB_BLE_NOTIFY cb ;
} BLE_NTF ;
bool BLE_notify(BLE_NTF *) ;

	// Indica la caratteristica (definire ABIL_BLE_INDIC)
	// Il messaggio deve vivere fino alla callback
	// Definire TIM_BLE_IND
	// cfr spec 4.2 [Vol 3, Part F] 3.4.7.2 Handle Value Indication pag 2199
	// Nota: ble la trasmette comunque, la scrittura della
	//       CYBLE_XXX_CONFIGURATION_DESC_HANDLE per sapere se e' abilitata
typedef void (*CB_BLE_INDICATE)(bool) ;
typedef struct {
	uint16_t handle ;
	void * v ;
	uint16_t dim ;
	CB_BLE_INDICATE cb ;
	uint32_t timeout ;
} S_BLE_IND ;
bool BLE_indicate(const S_BLE_IND *) ;

// Observer (definire BLE_OBSERVER)
bool BLE_obs_start(void) ;
bool BLE_obs_stop(void) ;

	// Power management
void BLE_enter_deep(void) ;
void BLE_leave_deep(void) ;
bool BLE_clock(void) ;
	// Invocare periodicamente
void BLE_run(void) ;
RICH_CPU BLE_cpu(void) ;



#endif
