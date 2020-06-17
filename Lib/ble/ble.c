#define STAMPA_DBG
#include "ble.h"
#include "soc/soc.h"

/* CYBLE-212006-01: define the test register to switch the PA/LNA hardware control pins */
//#define CYREG_SRSS_TST_DDFT_CTRL      0x40030008

/*
    Equivale a clocks->edit clock->ECO->configure

    Bit 7 = 8.1 pF
    Bit 6/0 = 3.69 + 0.1011 * N
    Con N = 0x15 = 21 -> 13.9131 pF
*/
//#define CAPACITOR_TRIM_VALUE       0x9595

#ifdef CY_BLE_CYBLE_H

extern void stampa_CYBLE_STATE_T(CYBLE_STATE_T s) ;
extern void stampa_evento(const char * titolo, uint32_t evn) ;
extern void stampa_CYBLE_GATT_ERR_CODE_T(CYBLE_GATT_ERR_CODE_T err) ;
extern void stampa_CYBLE_API_RESULT_T(CYBLE_API_RESULT_T err) ;
extern void stampa_au(const char * titolo, const void * v) ;
extern void stampa_bonded(CYBLE_GAP_BONDED_DEV_ADDR_LIST_T * bdal) ;

#define MTU_MIN     23

#define HANDLE_NON_VALIDO       0

static const BLE_WRITE_CFG * pCfg = NULL ;
static size_t numCfg = 0 ;
static uint16_t attrCorr = HANDLE_NON_VALIDO ;

static const BLE_CB * pCB = NULL ;

static bool bt_on = false ;
static uint16_t mtu = 0 ;
static bool connesso = false ;
static bool advertising = false ;

static CYBLE_GAP_BD_ADDR_T mac ;

#ifdef ABIL_BLE_INDIC
static CB_BLE_INDICATE cbInd = NULL ;
static CYBLE_GATTS_HANDLE_VALUE_IND_T hvi ;
static uint16_t indDim ;
#endif

#ifdef BLE_AUTEN
	// Volete la passkey
#	if CYBLE_BONDING_REQUIREMENT == CYBLE_BONDING_YES
		// Avete scelto bonding
#		define USA_BONDING		1
#	endif
#endif

static void ble_attivo(void)
{
    if (pCB) {
        pCB->stack_on() ;
    }
}

static void ble_connect(bool conn)
{
    if (pCB) {
        pCB->conn(conn) ;
    }
}

#ifdef BLE_AUTEN

static void ble_authfail(void)
{
    if (NULL == pCB) {
    }
    else if (NULL == pCB->authfail) {
    }
    else {
        pCB->authfail() ;
    }
}

#endif

#ifdef USA_BONDING

static void salva_bonding(void * v)
{
    UNUSED(v) ;

    // The cyBle_pendingFlashWrite variable is used to detect status of pending write to flash operation for stack
    // data and CCCD. This function automatically clears pending bits after write operation complete.
    if (cyBle_pendingFlashWrite != 0u) {
        bool continua = true ;
        while (continua) {
            // Application should keep calling this function till it return CYBLE_ERROR_OK
            CYBLE_API_RESULT_T car = CyBle_StoreBondingData(0u) ;
            switch (car) {
            case CYBLE_ERROR_OK:
                continua = false ;
                DBG_PUTS("CyBle_StoreBondingData OK") ;
                break ;
            case CYBLE_ERROR_FLASH_WRITE_NOT_PERMITED:
                // Riprovo
                break ;
            default:
                DBG_ERR ;
                stampa_CYBLE_API_RESULT_T(car) ;
                continua = false ;
                break ;
            }
        }
    }
    else {
        DBG_PUTS("? cyBle_pendingFlashWrite == 0 ?") ;
    }
}

static void gestione_bonded(void)
{
	CYBLE_GAP_BONDED_DEV_ADDR_LIST_T * bdal = soc_malloc(sizeof(CYBLE_GAP_BONDED_DEV_ADDR_LIST_T)) ;

	do {
		if (NULL == bdal) {
			DBG_ERR ;
			break ;
		}

		if (CYBLE_ERROR_OK !=
			CyBle_GapGetBondedDevicesList(bdal)) {
			DBG_ERR ;
			break ;
		}

		stampa_bonded(bdal) ;

		if (bdal->count < CYBLE_GAP_MAX_BONDED_DEVICE) {
			// Ottimo
			break ;
		}

		DBG_PUTS("CyBle_GapRemoveOldestDeviceFromBondedList") ;
		if (CYBLE_ERROR_OK !=
			CyBle_GapRemoveOldestDeviceFromBondedList() ) {
			DBG_ERR ;
			break ;
		}

		// Day015_Bonding: non serve perche' viene generato
		// CYBLE_EVT_PENDING_FLASH_WRITE
		//while(CYBLE_ERROR_OK != CyBle_StoreBondingData(1));

	} while (false) ;

	soc_free(bdal) ;
}

#endif

#ifdef BLE_ADV_INVIATO

static void bless_cb(uint32_t event, void * v)
{
	UNUSED(v) ;

	if (event & CYBLE_ISR_BLESS_ADV_CLOSE) {
		pCB->adv_inviato() ;
	}
}

#endif

static void avvia(void)
{
    CYBLE_BLESS_CLK_CFG_PARAMS_T clockConfig ;
#ifdef CAPACITOR_TRIM_VALUE
    /* load capacitors on the ECO should be tuned and the tuned value
    ** must be set in the CY_SYS_XTAL_BLERD_BB_XO_CAPTRIM_REG  */
    CY_SYS_XTAL_BLERD_BB_XO_CAPTRIM_REG = CAPACITOR_TRIM_VALUE ;
#endif
    /* Get the configured clock parameters for BLE sub-system */
    CyBle_GetBleClockCfgParam(&clockConfig) ;

    /* Update the sleep clock inaccuracy PPM based on WCO crystal used */
    /* If you see frequent link disconnection, tune your WCO or update the sleep clock accuracy here */
    clockConfig.bleLlSca = CYBLE_LL_SCA_000_TO_020_PPM ;

    /* set the clock configuration parameter of BLE sub-system with updated values*/
    CyBle_SetBleClockCfgParam(&clockConfig) ;

//    /* Put the device into discoverable mode so that a central device can connect to it */
//	(void) CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
}

#if 0
// Meglio impostare 23 alla connessione e poi
// aspettare l'eventuale evento di cambio mtu

static void leggi_mtu(void)
{
    if (0 == mtu) {
        CyBle_GattGetMtuSize(&mtu) ;
        DBG_PRINTF("mtu %d", mtu) ;
    }
}
#endif


static void evn_conn(void)
{
    if ( CYBLE_ERROR_OK == CyBle_GapGetPeerBdAddr(cyBle_connHandle.bdHandle, &mac) ) {
        DBG_PRINTF("Connesso a (%s): %02X:%02X:%02X:%02X:%02X:%02X",
                   0 == mac.type ? "PUB" : "PRV",
                   mac.bdAddr[5], mac.bdAddr[4], mac.bdAddr[3],
                   mac.bdAddr[2], mac.bdAddr[1], mac.bdAddr[0]) ;
    }
    else {
        DBG_ERR ;
    }

    connesso = true ;

    // Finche' non arriva CYBLE_EVT_GATTS_XCNHG_MTU_REQ
    mtu = MTU_MIN ;

#ifdef BLE_AUTEN
    if (pCB->passkey <= BLE_MAX_PASSKEY) {
        CyBle_GapAuthReq(cyBle_connHandle.bdHandle, &cyBle_authInfo) ;
    }
    else {
        ble_connect(true) ;
    }
#else
    ble_connect(true) ;
#endif
}

#ifdef BLE_OBSERVER

static enum {
	OBS_STOPPED,
	OBS_STARTING,
	OBS_RUNNING,
	OBS_STOPPING
} obs_state = OBS_STOPPED ;

bool BLE_obs_start(void)
{
	bool esito = false ;

	if (!bt_on) {
		DBG_ERR ;
	}
	else if (OBS_STOPPED == obs_state) {
		cyBle_discoveryInfo.scanType = CYBLE_GAPC_PASSIVE_SCANNING ;
		obs_state = OBS_STARTING ;
		if (CYBLE_ERROR_OK ==
			CyBle_GapcStartDiscovery(&cyBle_discoveryInfo)) {
			esito = true ;
		}
		else {
			DBG_ERR ;
			obs_state = OBS_STOPPED ;
		}
	}
	else {
		DBG_ERR ;
	}

    return esito ;
}

bool BLE_obs_stop(void)
{
	bool esito = false ;

	if (!bt_on) {
		DBG_ERR ;
	}
	else if (OBS_RUNNING == obs_state) {
		obs_state = OBS_STOPPING ;
		CyBle_GapcStopDiscovery() ;
		esito = true ;
	}
	else {
		DBG_ERR ;
	}

	return esito ;
}
#endif

static PF_BLE_WRITE trova_wh(CYBLE_GATT_DB_ATTR_HANDLE_T attrCorr)
{
	PF_BLE_WRITE pfWrite = NULL ;

	if (pCfg) {
	    const BLE_WRITE_CFG * cfg = pCfg ;
	    for (size_t i = 0 ; i < numCfg ; ++i, ++cfg) {
	        if (attrCorr == cfg->handle) {
	            pfWrite = cfg->pfWrite ;
	            break ;
	        }
	    }
	}

	return pfWrite ;
}


#ifdef ABIL_BLE_WR_LONG

static CYBLE_GATT_DB_ATTR_HANDLE_T wlh = HANDLE_NON_VALIDO ;

static void prep_write(CYBLE_GATTS_PREP_WRITE_REQ_PARAM_T * prm)
{
	// the CyBle_GattsPrepWriteReqSupport() function is called each time the device
	// receives the first CYBLE_EVT_GATTS_PREP_WRITE_REQ event of Long Write Value
	// procedure. For a Reliable Write Procedure, the CYBLE_EVT_GATTS_PREP_WRITE_REQ
	// event is generated for each unique attribute handle, and therefore it requires calling the
	// CyBle_GattsPrepWriteReqSupport() function.

	do {
		DBG_PRINTF("\t currentPrepWriteReqCount = %d", prm->currentPrepWriteReqCount) ;

		if (HANDLE_NON_VALIDO != wlh) {
			// Richieste successive
			break ;
		}

		// Prima richiesta ?
		PF_BLE_WRITE pf = trova_wh(prm->baseAddr[0].handleValuePair.attrHandle) ;
		if (NULL == pf) {
			// ??
			break ;
		}

		if(prm->currentPrepWriteReqCount == 1u) {
			// Salvo handle
			wlh = prm->baseAddr[0].handleValuePair.attrHandle ;

			// rispondo
			CyBle_GattsPrepWriteReqSupport(CYBLE_GATTS_PREP_WRITE_SUPPORT);
		}

	} while (false) ;
}

static void exec_write(CYBLE_GATTS_EXEC_WRITE_REQ_T * prm)
{
	// the event is generated once for each Long Write Value procedure,
	// and the event parameter provides a pointer to the start of the buffer where data is temporarily
	// stored. The data will be written to the GATT database only if there is successful indication from
	// the user, or if gattErrorCode equals to CYBLE_GATT_ERR_NONE

	do {
		DBG_PRINTF("\t prepWriteReqCount = %d", prm->prepWriteReqCount) ;

		if (HANDLE_NON_VALIDO == wlh) {
			break ;
		}

		if(prm->baseAddr[0u].handleValuePair.attrHandle != wlh) {
			break ;
		}

		if(prm->execWriteFlag != CYBLE_GATT_EXECUTE_WRITE_EXEC_FLAG) {
			// CYBLE_GATT_EXECUTE_WRITE_CANCEL_FLAG
			wlh = HANDLE_NON_VALIDO ;
			break ;
		}

		PF_BLE_WRITE pf = trova_wh(wlh) ;
		if (NULL == pf) {
			// ??
			break ;
		}

		uint8_t * msg = prm->baseAddr[0].handleValuePair.value.val;
		uint16_t dim = prm->baseAddr[0].handleValuePair.value.len ;
		for (uint8_t i=1 ; i<prm->prepWriteReqCount ; ++i) {
			dim += prm->baseAddr[i].handleValuePair.value.len ;
		}

		DBG_PRINT_HEX("exec_write", msg, dim) ;

        if ( !pf(msg, dim) ) {
            // Errore
            DBG_ERR ;

            prm->gattErrorCode = CYBLE_GATT_ERR_UNLIKELY_ERROR ;
        }

	} while (false) ;
}

#endif

static void bt_evn(uint32_t event, void * eventParam)
{
    switch (event) {
#ifdef BLE_OBSERVER
    case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
    	DBG_PUTS("CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT") ;
    	if (NULL == pCB->obs_adv) {
    		DBG_ERR ;
    	}
    	else {
    		CYBLE_GAPC_ADV_REPORT_T *advReport = (CYBLE_GAPC_ADV_REPORT_T*)eventParam ;
    		BLE_OBS_INFO oi = {
    				.type = advReport->eventType,
					.mtype = advReport->peerAddrType,
    				.mac = advReport->peerBdAddr,
					.rssi = advReport->rssi,
					.dim = advReport->dataLen,
					.dati = advReport->data
    		} ;

    		if (0 == oi.dim)
    			oi.dati = NULL ;

            pCB->obs_adv(&oi) ;
    	}
        break ;

    case CYBLE_EVT_GAPC_SCAN_START_STOP:
    	DBG_PUTS("CYBLE_EVT_GAPC_SCAN_START_STOP") ;
    	switch (obs_state) {
    	case OBS_STOPPED: DBG_ERR ; break ;
    	case OBS_STARTING:
    		obs_state = OBS_RUNNING ;
    		DBG_PUTS("\trun") ;
    		break ;
    	case OBS_RUNNING: DBG_ERR ; break ;
    	case OBS_STOPPING:
    		obs_state = OBS_STOPPED ;
    		DBG_PUTS("\tstop") ;
    		break ;
    	default: DBG_ERR ; break ;
    	}
        break ;

#endif
#ifdef ABIL_BLE_INDIC
    case CYBLE_EVT_GATTS_HANDLE_VALUE_CNF:
        DBG_PUTS("CYBLE_EVT_GATTS_HANDLE_VALUE_CNF") ;
        if (cbInd) {
            if (indDim == hvi.value.len) {
                // Finito
                cbInd(true) ;
                cbInd = NULL ;
                timer_stop(TIM_BLE_IND) ;
            }
            else {
                indDim -= hvi.value.len ;
                hvi.value.val += hvi.value.len ;

                uint16_t dim = indDim ;
                if (dim > mtu - 3) {
                    dim = mtu - 3 ;
                }

                hvi.value.len = dim ;

                if ( CYBLE_ERROR_OK !=
                     CyBle_GattsIndication(cyBle_connHandle, &hvi) ) {
                    DBG_ERR ;
                    cbInd(false) ;
                    cbInd = NULL ;
                    timer_stop(TIM_BLE_IND) ;
                }
            }
        }
        else {
            // Dovevo aspettare di piu'
            DBG_ERR ;
        }
        break ;
#endif

//    case CYBLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ: {
//          CYBLE_GATTS_CHAR_VAL_READ_REQ_T * prm = (CYBLE_GATTS_CHAR_VAL_READ_REQ_T *) eventParam ;
//      }
//        break ;

#ifdef ABIL_BLE_WR_LONG
    // https://community.cypress.com/message/41810
    case CYBLE_EVT_GATTS_PREP_WRITE_REQ:
    	DBG_PUTS("CYBLE_EVT_GATTS_PREP_WRITE_REQ") ;
    	prep_write((CYBLE_GATTS_PREP_WRITE_REQ_PARAM_T *) eventParam) ;
    	break ;
    case CYBLE_EVT_GATTS_EXEC_WRITE_REQ:
    	DBG_PUTS("CYBLE_EVT_GATTS_EXEC_WRITE_REQ") ;
    	exec_write((CYBLE_GATTS_EXEC_WRITE_REQ_T *) eventParam) ;
    	break ;
#endif

    case CYBLE_EVT_GATTS_WRITE_REQ: {
            /* This event is generated when the connected Central device sends a
             * Write request. The parameter contains the data written */
            DBG_PUTS("CYBLE_EVT_GATTS_WRITE_REQ") ;

            /* Extract the Write data sent by Client */
            CYBLE_GATTS_WRITE_REQ_PARAM_T * wrReqParam = (CYBLE_GATTS_WRITE_REQ_PARAM_T *) eventParam ;

            attrCorr = wrReqParam->handleValPair.attrHandle ;

            // Cerco l'handle
            PF_BLE_WRITE pfWrite = trova_wh(attrCorr) ;

            do {
                if (NULL == pfWrite) {
                    // Non interessa
                    (void) CyBle_GattsWriteRsp(cyBle_connHandle) ;
                    break ;
                }
#ifdef BLE_AUTOR
                // https://community.cypress.com/thread/52034
                CYBLE_GATT_ERR_CODE_T gattErr = CyBle_GattsWriteAttributeValue(&wrReqParam->handleValPair,
                                                                               0u,
                                                                               &wrReqParam->connHandle,
                                                                               CYBLE_GATT_DB_PEER_INITIATED) ;
                if (CYBLE_GATT_ERR_NONE != gattErr) {
                    CYBLE_GATTS_ERR_PARAM_T err = {
                        .attrHandle = attrCorr,
                        .opcode = CYBLE_GATT_WRITE_REQ,
                        .errorCode = gattErr
                    } ;

                    DBG_ERR ;

                    (void) CyBle_GattsErrorRsp(cyBle_connHandle, &err) ;
                    break ;
                }
#endif
                if ( !pfWrite(wrReqParam->handleValPair.value.val,
                              wrReqParam->handleValPair.value.len) ) {
                    // Errore
                    DBG_ERR ;

                    CYBLE_GATTS_ERR_PARAM_T err = {
                        .attrHandle = attrCorr,
                        .opcode = CYBLE_GATT_WRITE_REQ,
                        .errorCode = CYBLE_GATT_ERR_UNLIKELY_ERROR
                    } ;
                    (void) CyBle_GattsErrorRsp(cyBle_connHandle, &err) ;
                }

                // OK
                (void) CyBle_GattsWriteRsp(cyBle_connHandle) ;
            } while (false) ;

            attrCorr = HANDLE_NON_VALIDO ;
        }
        break ;

    case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
        /* This event is generated whenever Advertisement starts or stops command is invoked.
         * The exact state of advertisement is obtained by CyBle_State() */
        DBG_PUTS("CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP") ;
        stampa_CYBLE_STATE_T( CyBle_GetState() ) ;

        //advertising = CYBLE_STATE_ADVERTISING == CyBle_GetState() ;
        advertising = !advertising ;
//      DBG_PRINTF("advertising %s", advertising ? "SI" : "NO") ;
        break ;

//    case CYBLE_EVT_GAP_ENHANCE_CONN_COMPLETE:
//      // Generato se si abilita Link Layer Security (cfr AN99209)
//      DBG_PUTS("CYBLE_EVT_GAP_ENHANCE_CONN_COMPLETE") ;
//      evn_conn() ;
//        break ;

    case CYBLE_EVT_GAP_DEVICE_CONNECTED:
        DBG_PUTS("CYBLE_EVT_GAP_DEVICE_CONNECTED") ;
        advertising = false ;
        {
#ifdef DBG_ABIL
        	CYBLE_GAP_CONN_PARAM_UPDATED_IN_CONTROLLER_T * prm = (CYBLE_GAP_CONN_PARAM_UPDATED_IN_CONTROLLER_T *) eventParam ;
            DBG_PRINTF("\t status        0x%02X", prm->status) ;
            DBG_PRINTF("\t connIntv      0x%04X", prm->connIntv) ;
            DBG_PRINTF("\t connLatency   0x%04X", prm->connLatency) ;
        	DBG_PRINTF("\t supervisionTO 0x%04X", prm->supervisionTO) ;
#endif
        }
        evn_conn() ;
        break ;

    case CYBLE_EVT_GATTS_XCNHG_MTU_REQ:
        mtu = ( ( (CYBLE_GATT_XCHG_MTU_PARAM_T *)eventParam )->mtu < CYBLE_GATT_MTU ) ?
              ( (CYBLE_GATT_XCHG_MTU_PARAM_T *)eventParam )->mtu : CYBLE_GATT_MTU ;
        DBG_PRINTF("CYBLE_EVT_GATTS_XCNHG_MTU_REQ %d", mtu) ;
        break ;

#ifdef USA_BONDING
    case CYBLE_EVT_PENDING_FLASH_WRITE:
        /* Inform application that flash write is pending. Stack internal data
        * structures are modified and require to be stored in Flash using
        * CyBle_StoreBondingData() */
        DBG_PRINTF("CYBLE_EVT_PENDING_FLASH_WRITE") ;
        SOC_apc(APC_BLE, salva_bonding) ;
        break ;
#endif

#ifdef BLE_AUTEN

//    case CYBLE_EVT_GAP_SMP_NEGOTIATED_AUTH_INFO:
//      // ? mie impostazioni ?
//      stampa_au("CYBLE_EVT_GAP_SMP_NEGOTIATED_AUTH_INFO", eventParam) ;
//      break ;

//    case CYBLE_EVT_GAP_AUTH_REQ:
//		// quelle del central
//		stampa_au("CYBLE_EVT_GAP_AUTH_REQ", eventParam) ;
//        break ;

    case CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
        DBG_PRINTF("CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST. Passkey is: %06ld",
                   *(uint32_t*)eventParam) ;
        break ;

    case CYBLE_EVT_GAP_AUTH_COMPLETE:
        // quelle contrattate
#if 1
    	DBG_PUTS("CYBLE_EVT_GAP_AUTH_COMPLETE") ;
#else
        stampa_au("CYBLE_EVT_GAP_AUTH_COMPLETE", eventParam) ;
#endif
        ble_connect(true) ;
        break ;

    case CYBLE_EVT_GAP_AUTH_FAILED:
        DBG_PUTS("CYBLE_EVT_GAP_AUTH_FAILED") ;
        if ( CYBLE_ERROR_OK !=
             CyBle_GapDisconnect(cyBle_connHandle.bdHandle) ) {
            DBG_ERR ;
        }
        ble_authfail() ;
        break ;

//    case CYBLE_EVT_GAP_ENCRYPT_CHANGE:
//        DBG_PUTS("CYBLE_EVT_GAP_ENCRYPT_CHANGE") ;
//
//        /*Get the current status of Encryption*/
//        if (*(uint8 *)eventParam == 0x00) {
//            DBG_PUTS("\tEncrytpion OFF") ;
//        }
//        else if (*(uint8 *)eventParam == 0x01) {
//            DBG_PUTS("\tEncryption ON") ;
//        }
//        else {
//            DBG_PRINTF("\tERROR %02X", *(uint8 *)eventParam) ;
//        }
//        break ;
#endif

    case CYBLE_EVT_GATT_DISCONNECT_IND:
        DBG_PUTS("disc. ind.") ;
        break ;

    case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
        DBG_PUTS("disconnesso") ;
        connesso = false ;
        mtu = 0 ;
#ifdef USA_BONDING
        gestione_bonded() ;
#endif
        avvia() ;

        ble_connect(false) ;
        break ;

    case CYBLE_EVT_STACK_ON:
        DBG_PUTS("stack on") ;
#ifdef CYREG_SRSS_TST_DDFT_CTRL
        /* Configure the Link Layer to automatically switch PA control pin P3[2] and LNA control pin P3[3] */
        CY_SET_XTND_REG32( (void CYFAR *)(CYREG_BLE_BLESS_RF_CONFIG), 0x0331 ) ;
        CY_SET_XTND_REG32( (void CYFAR *)(CYREG_SRSS_TST_DDFT_CTRL), 0x80000302 ) ;
#endif

#ifdef CAPACITOR_TRIM_VALUE
        /* load capacitors on the ECO should be tuned and the tuned value
        ** must be set in the CY_SYS_XTAL_BLERD_BB_XO_CAPTRIM_REG  */
        //CY_SYS_XTAL_BLERD_BB_XO_CAPTRIM_REG = CAPACITOR_TRIM_VALUE;
        // Da https://community.cypress.com/docs/DOC-10498
        CY_SET_XTND_REG32( (void CYFAR *)(CYREG_BLE_BLERD_BB_XO_CAPTRIM), CAPACITOR_TRIM_VALUE ) ;
#endif
        avvia() ;

#ifdef BLE_MAX_NDD
        CHECK( CYBLE_ERROR_OK ==
               CyBle_GapSetLocalName(pCB->nome) ) ;
#endif

#ifdef DBG_ABIL
        {
            CYBLE_GAP_BD_ADDR_T chiSono = {
                .type = 0
            } ;
            if ( CYBLE_ERROR_OK != CyBle_GetDeviceAddress(&chiSono) ) {
                // Cacchio!
                DBG_ERR ;
            }
            else {
                DBG_PRINTF("Io sono %02X:%02X:%02X:%02X:%02X:%02X",
                           chiSono.bdAddr[5], chiSono.bdAddr[4], chiSono.bdAddr[3],
                           chiSono.bdAddr[2], chiSono.bdAddr[1], chiSono.bdAddr[0]) ;
            }
        }
#endif
#ifdef USA_BONDING
        gestione_bonded() ;
#endif
#ifdef BLE_ADV_INVIATO
        {
        	CYBLE_BLESS_EVENT_PARAM_T be = {
				.BlessStateMask = CYBLE_ISR_BLESS_ADV_CLOSE,
				.bless_evt_app_cb = bless_cb
        	};
        	CYBLE_API_RESULT_T err = CyBle_RegisterBlessInterruptCallback(&be) ;
            if (CYBLE_ERROR_OK != err) {
            	stampa_CYBLE_API_RESULT_T(err) ;
            }
        }
#endif
        // Avviso
        ble_attivo() ;
        break ;

    default:
        stampa_evento("? non gestito ?", event) ;
        break ;
    }
}

void BLE_config(const BLE_WRITE_CFG * a, const size_t b)
{
    pCfg = a ;
    numCfg = b ;
}

void BLE_start(const BLE_CB * cb)
{
    ASSERT(cb) ;

    if (!bt_on) {
        DBG_PUTS("ble start") ;

        pCB = cb ;

//		// Attivo il clock
//		CySysClkEcoStart(2000) ;
//		while (0 == CySysClkEcoReadStatus())
//			CyDelayUs(100) ;

        bt_on = true ;
        connesso = false ;
        advertising = false ;
        mtu = 0 ;

        // Il ciclo infinito viene dagli esempi cypress
        // Complessivamente impiega un tempo inferiore ai 70 ms
        CyBle_Start(bt_evn) ;

        while (CyBle_GetState() == CYBLE_STATE_INITIALIZING)
            CyBle_ProcessEvents() ;
    }
}

bool BLE_seed(uint32_t seed)
{
    if (!bt_on) {
        DBG_ERR ;
        return false ;
    }
    else {
        CyBle_SetSeedForRandomGenerator(seed) ;
        return true ;
    }
}

bool BLE_rand(uint8_t * nc)
{
    if (!bt_on) {
        DBG_ERR ;
        return false ;
    }
    else if ( CYBLE_ERROR_OK !=
              CyBle_GenerateRandomNumber(nc) ) {
        DBG_ERR ;
        return false ;
    }
    else {
        DBG_PRINTF("BLE_rand %02X %02X %02X %02X %02X %02X %02X %02X",
                   nc[0], nc[1], nc[2], nc[3],
                   nc[4], nc[5], nc[6], nc[7], nc[8]) ;
        return true ;
    }
}

void BLE_stop(void)
{
    if (bt_on) {
        DBG_PUTS("ble stop") ;

        if (connesso) {
            if ( CYBLE_ERROR_OK == CyBle_GapDisconnect(cyBle_connHandle.bdHandle) ) {
                /* Wait for disconnection event */
                while (CyBle_GetState() == CYBLE_STATE_CONNECTED) {
                    /* Process BLE events */
                    CyBle_ProcessEvents() ;
                }
            }
            else {
                DBG_ERR ;
            }
        }

        // La funzione impiega meno di 15 ms
        // Certe volte si blocca e scatta il watchdog
        CyBle_Stop() ;

        bt_on = false ;
        connesso = false ;
        advertising = false ;
        mtu = 0 ;

        pCfg = NULL ;
        numCfg = 0 ;

        // Non serve piu'
        //CySysClkEcoStop() ;
    }
}

bool BLE_agg_char(uint16_t charh, bool locale, const void * dati, uint16_t dim)
{
	bool esito = false ;

	if (NULL == dati) {
		DBG_ERR ;
	}
	else if (0 == dim) {
		DBG_ERR ;
	}
	else if (!bt_on) {
		DBG_ERR ;
	}
	else {
		union {
			const void * v ;
			uint8_t * u ;
		} u ;
		u.v = dati ;

		CYBLE_GATT_HANDLE_VALUE_PAIR_T hvp = {
			.attrHandle = charh,
			.value.val = u.u,
			.value.len = dim
		} ;

		CYBLE_GATT_ERR_CODE_T ris = CyBle_GattsWriteAttributeValue(
				&hvp,
				0,
				&cyBle_connHandle,
				locale ? CYBLE_GATT_DB_LOCALLY_INITIATED : CYBLE_GATT_DB_PEER_INITIATED) ;
		esito = CYBLE_GATT_ERR_NONE == ris ;
		if (!esito) {
			stampa_CYBLE_GATT_ERR_CODE_T(ris) ;
		}
	}

	return esito ;
}

static bool scrivi_attr(CYBLE_GATT_DB_ATTR_HANDLE_T h,
                        const void * v, const uint16_t dim,
                        uint8 flag)
{
    union
    {
        const void * v ;
        uint8 * p ;
    } u ;

    u.v = v ;

    CYBLE_GATT_HANDLE_VALUE_PAIR_T hvp = {
        .attrHandle = h,
        .value = {
            .val = u.p,
            .len = dim
        }
    } ;

    CYBLE_GATT_ERR_CODE_T err =
        CyBle_GattsWriteAttributeValue(
            &hvp,
            0,
            &cyBle_connHandle,
            flag) ;
    stampa_CYBLE_GATT_ERR_CODE_T(err) ;
    return CYBLE_GATT_ERR_NONE == err ;
}

bool BLE_scrivi_attr_corr(const void * v, const uint16_t dim)
{
    bool esito = false ;

    if (attrCorr != HANDLE_NON_VALIDO) {
        esito = scrivi_attr(attrCorr, v, dim, CYBLE_GATT_DB_PEER_INITIATED) ;

        if (!esito) {
            DBG_ERR ;
        }
    }
    else {
        DBG_ERR ;
    }

    return esito ;
}

bool BLE_scrivi_attr(uint16_t h, const void * v, const uint16_t dim)
{
    bool esito = false ;

    if (attrCorr == HANDLE_NON_VALIDO) {
        esito = scrivi_attr(h, v, dim, CYBLE_GATT_DB_LOCALLY_INITIATED) ;

        if (!esito) {
            DBG_ERR ;
            DBG_PRINTF("\th = %04X", h) ;
        }
    }
    else {
        DBG_ERR ;
    }

    return esito ;
}

bool BLE_servizi(const BLE_SRV * vSrv, size_t dim)
{
	size_t i=0 ;
	CYBLE_GATT_DB_ATTR_HANDLE_T hmin = vSrv[0].h ;

	for ( ; i<dim ; ++i) {
		CYBLE_GATT_DB_ATTR_HANDLE_T h = vSrv[i].h ;
		CYBLE_GATT_ERR_CODE_T err ;

		if (vSrv[i].abil) {
			DBG_PRINTF("abilito srv %04X", h) ;
			err = CyBle_GattsEnableAttribute(h) ;
		}
		else {
			DBG_PRINTF("DISabilito srv %04X", h) ;
			err = CyBle_GattsDisableAttribute(h) ;
		}

		if (CYBLE_GATT_ERR_NONE != err) {
			DBG_ERR ;
			stampa_CYBLE_GATT_ERR_CODE_T(err) ;
			break ;
		}
		else if (h < hmin)
			hmin = h ;
	}

	if (i == dim) {
		CYBLE_GATT_HANDLE_VALUE_PAIR_T    handleValuePair;
#if 0
		uint32 value;

		/* Force client to rediscover services in range of bootloader service */
		value = ((uint32)(((uint32) cyBle_btss.btServiceHandle) << 16u)) |
				((uint32) (cyBle_btss.btServiceInfo[0u].btServiceCharDescriptors[0u]));


		handleValuePair.value.val = (uint8 *)&value;
		handleValuePair.value.len = sizeof(value);

		//handleValuePair.attrHandle = cyBle_gatts.serviceChangedHandle;
#else
		struct {
			CYBLE_GATT_DB_ATTR_HANDLE_T minh ;
			CYBLE_GATT_DB_ATTR_HANDLE_T maxh ;
		} val ;

		/* Force client to rediscover services dal minimo in su */
		val.minh = hmin ;
		val.maxh = 0xFFFF ;

		handleValuePair.value.val = (uint8 *)&val;
		handleValuePair.value.len = sizeof(val);
#endif
		handleValuePair.attrHandle = cyBle_gatts.serviceChangedHandle;

		DBG_PRINTF("scrivo %04X", handleValuePair.attrHandle) ;
		DBG_PRINT_HEX("\t", handleValuePair.value.val, handleValuePair.value.len) ;

		CYBLE_GATT_ERR_CODE_T err = CyBle_GattsWriteAttributeValue(&handleValuePair, 0u, NULL,CYBLE_GATT_DB_LOCALLY_INITIATED);
		if (CYBLE_GATT_ERR_NONE != err) { 
			DBG_ERR ;
			stampa_CYBLE_GATT_ERR_CODE_T(err) ;
		}
		else if (!connesso) {
			// Ottimo
		}
		else {
			// https://community.cypress.com/message/238346
			DBG_PUTS("indico") ;
            CYBLE_API_RESULT_T res = CyBle_GattsIndication(cyBle_connHandle, &handleValuePair);
	        if (CYBLE_ERROR_OK != res) {
                DBG_ERR ;
                stampa_CYBLE_API_RESULT_T(res) ;
	        }
		}
	}

	return i == dim ;
}

void BLE_presentati(BLE_IC * x)
{
    do {
        if ( !bt_on ) {
            DBG_ERR ;
            break ;
        }

        if (connesso) {
            DBG_ERR ;
            break ;
        }

        if (advertising) {
            DBG_ERR ;
            break ;
        }
#if 0
        // Non funziona: l'indirizzo resta 00:A0:50:xxxx
        CYBLE_GAP_BD_ADDR_T ni = {
            .type = 0
        } ;

        if ( CYBLE_ERROR_OK != CyBle_GapGenerateDeviceAddress(&ni, CYBLE_GAP_RANDOM_STATIC_ADDR, NULL) ) {
            DBG_ERR ;
        }
        else if ( CYBLE_ERROR_OK != CyBle_SetDeviceAddress(&ni) ) {
            DBG_ERR ;
        }
        else if ( CYBLE_ERROR_OK != CyBle_GapSetIdAddress(&ni) ) {
            DBG_ERR ;
        }
        else {
            DBG_PRINTF("Appaio %02X:%02X:%02X:%02X:%02X:%02X",
                       ni.bdAddr[5], ni.bdAddr[4], ni.bdAddr[3],
                       ni.bdAddr[2], ni.bdAddr[1], ni.bdAddr[0]) ;
        }
#else
        if (x) {
            CYBLE_GAP_BD_ADDR_T ic = {
                .type = 1
            } ;

            if ( CYBLE_ERROR_OK != CyBle_GapGenerateDeviceAddress(&ic, CYBLE_GAP_RANDOM_STATIC_ADDR, NULL) ) {
                DBG_ERR ;
            }
            else if ( CYBLE_ERROR_OK == CyBle_SetDeviceAddress(&ic) ) {
                // brutto ma obbligatorio
                cyBle_discoveryModeInfo.advParam->ownAddrType = CYBLE_GAP_ADDR_TYPE_RANDOM ;

                DBG_PRINTF("Appaio %02X:%02X:%02X:%02X:%02X:%02X",
                           ic.bdAddr[5], ic.bdAddr[4], ic.bdAddr[3],
                           ic.bdAddr[2], ic.bdAddr[1], ic.bdAddr[0]) ;

                memcpy(x->bda, ic.bdAddr, 6) ;
                x->cb() ;
            }
            else {
                DBG_ERR ;
            }
        }
#endif

#ifdef BLE_AUTEN
        if (pCB->passkey <= BLE_MAX_PASSKEY) {
            DBG_PRINTF("Fisso passkey: %06d", pCB->passkey) ;
            CyBle_GapFixAuthPassKey(1, pCB->passkey) ;
        }
#endif
#ifdef BLE_AUTOR
        if ( CYBLE_GATT_ERR_NONE != CyBle_GattsDbAuthorize(0) ) {
            DBG_ERR ;
        }
        else {
            DBG_PUTS("NON autorizzato") ;
        }
#endif
        CHECK(CYBLE_ERROR_OK ==
                      CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST)) ;
    } while (false) ;
}

void BLE_nasconditi(void)
{
    do {
        if ( !bt_on ) {
            DBG_ERR ;
            break ;
        }

        if (connesso) {
            DBG_ERR ;
            break ;
        }

        if (!advertising) {
            DBG_ERR ;
            break ;
        }

        CyBle_GappStopAdvertisement() ;
        //DBG_PRINTF("BLE_presentati: advertising %s", advertising ? "SI" : "NO") ;
    } while (false) ;
}

void BLE_sconnetti(void)
{
    if ( !bt_on ) {
        DBG_ERR ;
    }
    else if (!connesso) {
        DBG_ERR ;
    }
    else {
        DBG_PUTS("BLE_sconnetti") ;

        CyBle_GapDisconnect(cyBle_connHandle.bdHandle) ;
    }
}

#ifdef BLE_AUTOR

bool BLE_autorizza(void)
{
    bool esito = false ;

    if ( !bt_on ) {
        DBG_ERR ;
    }
    else if (!connesso) {
        DBG_ERR ;
    }
    else {
        esito = true ;
        CyBle_GattsDbAuthorize(1) ;
        DBG_PUTS("AUTORIZZATO") ;
    }

    return esito ;
}

#endif

const void * BLE_central(void)
{
    const void * altro = NULL ;

    do {
        if ( !bt_on ) {
            break ;
        }

        if (!connesso) {
            break ;
        }

        altro = mac.bdAddr ;
    } while (false) ;

    return altro ;
}

bool BLE_mac(void * x, bool public)
{
    bool esito = false ;

    do {
        if ( !bt_on ) {
            break ;
        }

        CYBLE_GAP_BD_ADDR_T bdAddr = {
            .type = public ? 0 : 1,
        } ;
        if ( CYBLE_ERROR_OK != CyBle_GetDeviceAddress(&bdAddr) ) {
            break ;
        }

        memcpy(x, bdAddr.bdAddr, DIM_MAC) ;
        esito = true ;
    } while (false) ;

    return esito ;
}

#ifdef ABIL_BLE_NOTIF
void BLE_notify(uint16_t h, void * v, uint16_t lenght)
{
    if (connesso) {
        CYBLE_GATTS_HANDLE_VALUE_NTF_T hvntf = {
            .attrHandle = h
        } ;
        uint8_t * dati = (uint8_t *) v ;

        while (lenght) {
            if ( CYBLE_STACK_STATE_BUSY == CyBle_GattGetBusyStatus() ) {
                CyBle_ProcessEvents() ;
                continue ;
            }

            uint16_t dim = lenght ;
            if (dim > mtu - 3) {
                dim = mtu - 3 ;
            }

            hvntf.value.len = dim ;
            hvntf.value.val = dati ;

            CYBLE_API_RESULT_T ris = CyBle_GattsNotification(cyBle_connHandle, &hvntf) ;

            CyBle_ProcessEvents() ;

            switch (ris) {
            case CYBLE_ERROR_OK:
                //DBG_PRINT_HEX("notif ", dati, dim) ;
                lenght -= dim ;
                dati += dim ;
                break ;

            case CYBLE_ERROR_MEMORY_ALLOCATION_FAILED:
                // Bisogna aspettare che lo stack spedisca i dati
                CyBle_ProcessEvents() ;
                break ;

            default:
                // ??? esco
                DBG_PRINTF("ERR %s %d ris = 0x%04X\n", __FILE__, __LINE__, ris) ;
                lenght = 0 ;
                break ;

            case CYBLE_ERROR_INVALID_PARAMETER:
                // notifiche disabilitate
                DBG_ERR ;
                lenght = 0 ;
                break ;

            case CYBLE_ERROR_INVALID_OPERATION:
                // Riprovo
                DBG_ERR ;
                CyBle_ProcessEvents() ;
                break ;
            }
        }
    }
    else {
        DBG_ERR ;
    }
}
#endif

#ifdef ABIL_BLE_INDIC

static void to_indic(void * v)
{
    UNUSED(v) ;

    if (cbInd) {
        cbInd(false) ;
        cbInd = NULL ;
    }
}

// CyBle_GattcWriteCharacteristicDescriptors
bool BLE_indicate(const S_BLE_IND * x)
{
    bool esito = false ;

    do {
        if (NULL == x) {
            DBG_ERR ;
            break ;
        }

        if (cbInd != NULL) {
            DBG_ERR ;
            break ;
        }

        if (!connesso) {
            DBG_ERR ;
            break ;
        }

        timer_setcb(TIM_BLE_IND, to_indic) ;

        cbInd = x->cb ;
        indDim = x->dim ;

        uint16_t dim = indDim ;
        if (dim > mtu - 3) {
            dim = mtu - 3 ;
        }

        hvi.attrHandle = x->handle ;
        hvi.value.len = dim ;
        hvi.value.val = x->v ;

        esito = CYBLE_ERROR_OK ==
                CyBle_GattsIndication(cyBle_connHandle, &hvi) ;
        if (!esito) {
            DBG_ERR ;
            cbInd = NULL ;
        }
        else {
            timer_start(TIM_BLE_IND, x->timeout) ;
        }
    } while (false) ;

    return esito ;
}
#endif

void BLE_run(void)
{
    /***********************************************************************
    *  Process all BLE events in the stack
    ***********************************************************************/
    CyBle_ProcessEvents() ;

    /***********************************************************************
    *  Put BLE sub system in DeepSleep mode when it is idle
    ***********************************************************************/
    CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP) ;
}

RICH_CPU BLE_cpu(void)
{
    CYBLE_BLESS_STATE_T blePower = CyBle_GetBleSsState() ;

    if ( (0 == blePower) || (CYBLE_BLESS_STATE_INVALID == blePower) ) {
        // Mai inizializzato o spento
        return CPU_FERMA ;
    }

    if (connesso) {
        return CPU_PAUSA ;
    }
    // Vedi tabella 3
    else if ( (blePower == CYBLE_BLESS_STATE_DEEPSLEEP || blePower == CYBLE_BLESS_STATE_ECO_ON) ) {
        return CPU_FERMA ;
    }
    else if (blePower != CYBLE_BLESS_STATE_EVENT_CLOSE) {
        return CPU_PAUSA ;
    }
    else {
        return CPU_ATTIVA ;
    }
}

bool BLE_clock(void)
{
    CYBLE_BLESS_STATE_T blePower = CyBle_GetBleSsState() ;

    if (0 == blePower) {
        // Mai inizializzato
        return false ;
    }
    else if ( (blePower == CYBLE_BLESS_STATE_DEEPSLEEP || blePower == CYBLE_BLESS_STATE_ECO_ON) ) {
        return false ;
    }
    else {
        return blePower != CYBLE_BLESS_STATE_EVENT_CLOSE ;
    }
}

void BLE_enter_deep(void)
{
}
void BLE_leave_deep(void)
{
}

#else

void BLE_config(const BLE_WRITE_CFG * a, const size_t b)
{
}

void BLE_start(void)
{
}
void BLE_stop(void)
{
}

bool BLE_scrivi_attr_corr(const void * a, const uint16_t b)
{
    return false ;
}

bool BLE_scrivi_attr(uint16_t a, const void * b, const uint16_t c)
{
    return false ;
}

bool BLE_nome(const char * a)
{
    return false ;
}

bool BLE_mac(void * x)
{
    return false ;
}

void BLE_notify(uint16_t a, void * b, uint16_t c)
{
}

void BLE_enter_deep(void)
{
    CySysClkEcoStop() ;
}

void BLE_leave_deep(void)
{
}

bool BLE_clock(void)
{
    return false ;
}

void BLE_run(void)
{
}

RICH_CPU BLE_cpu(void)
{
    return CPU_FERMA ;
}

#endif
