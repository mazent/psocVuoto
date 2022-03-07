#ifndef WDTIMER_CFG_H_
#define WDTIMER_CFG_H_

// L'interruzione capita quando commuta il bit
// 15 -> 1 secondo
#define WDOG_TOGGLE_BIT      15

/*
 * Se non c'e' attivita', p.e. bt spento e nessun timer (lento o veloce)
 * attivo, il wdog puo' resettare: per evitare definire WDOG_TICK
 */
//#define WDOG_TICK		1

/*
 * Per diminuire il consumo durante advertising,
 * questi timer possono usare quella interruzione
 *
 * Occorre:
 *      1) definire BLE_ADV_INVIATO
 *      2) definire WDOG_ADVERTISEMENT
 *      3) BLE_CB::adv_inviato invoca wdt_adv_inviato
 *      4) alla connessione, BLE_CB::conn invoca wdt_adv_fine
 */
//#define WDOG_ADVERTISEMENT	1

/*
 * Lista dei timer
 */
#define MAX_WDTIMER_SW      0

/*
 * Watchdog software
 *
 * Se, p.e. dall'esterno, si percepisce che il dispositivo non funziona
 * ma il watchdog non scatta, cosa si fa?
 *
 * Si abilita questo!
 * Un comando esterno azzera il wds e, se si osservano malfunzionamenti,
 * basta non inviare piu' il comando
 *
 * Nel caso, definire:
 *     void WDOG_wds_cb(void)
 * che viene invocata prima del reset
 *
 */
//#define WDOG_SW_ABIL		1

#endif
