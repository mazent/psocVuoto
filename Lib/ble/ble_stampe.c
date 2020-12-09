#define STAMPA_DBG
#include "ble.h"

#ifdef CY_BLE_CYBLE_H

#ifdef DBG_ABIL

#if 1
void stampa_bonded(CYBLE_GAP_BONDED_DEV_ADDR_LIST_T * bdal)
{
    CYBLE_GAP_BD_ADDR_T * addr = bdal->bdAddrList ;

    DBG_PRINTF("BondedDevicesList [%d]", bdal->count) ;
    for (uint8_t i = 0 ; i < bdal->count ; ++i, ++addr) {
        DBG_PRINTF("\t %d = %02X:%02X:%02X:%02X:%02X:%02X",
                   i,
                   addr->bdAddr[5], addr->bdAddr[4], addr->bdAddr[3],
                   addr->bdAddr[2], addr->bdAddr[1], addr->bdAddr[0]) ;
    }
}

#else
void stampa_bonded(CYBLE_GAP_BONDED_DEV_ADDR_LIST_T * a)
{
    UNUSED(a) ;
}

#endif

#if 1
void stampa_au(const char * titolo, const void * v)
{
    DBG_PUTS(titolo) ;

    const CYBLE_GAP_AUTH_INFO_T * au = v ;

    switch (au->security) {
    case CYBLE_GAP_SEC_LEVEL_1:
        DBG_PRINTF("     %d modo 1 liv 1", au->security) ;
        break ;
    case CYBLE_GAP_SEC_LEVEL_2:
        DBG_PRINTF("     %d modo 1 liv 2", au->security) ;
        break ;
    case CYBLE_GAP_SEC_LEVEL_3:
        DBG_PRINTF("     %d modo 1 liv 3", au->security) ;
        break ;
    case CYBLE_GAP_SEC_LEVEL_4:
        DBG_PRINTF("     %d modo 1 liv 4", au->security) ;
        break ;
    default:
        DBG_PRINTF("     ?? %02X ??", au->security) ;
        break ;
    }

    if ( (au->security) == 0x00 ) {
        DBG_PUTS("     0 No Security") ;
    }
    else if ( (au->security) == 0x01 ) {
        DBG_PUTS("     1 Security: Unauthenticated and Encryption") ;
    }
    else if ( (au->security) == 0x02 ) {
        DBG_PUTS("     2 security: Authentication and Encryption") ;
    }
    else if ( (au->security) == 0x03 ) {
        DBG_PUTS("     3 Unauthenticated and data signing") ;
    }
    else if ( (au->security) == 0x04 ) {
        DBG_PUTS("     4 Authentication and data signing") ;
    }

    if (CYBLE_GAP_BONDING_NONE == au->bonding) {
        DBG_PUTS("     no bonding") ;
    }
    else {
        DBG_PUTS("     bonding") ;
    }

    DBG_PRINTF("     ekeySize = %d", au->ekeySize) ;

    if (au->pairingProperties & CYBLE_GAP_SMP_SC_PAIR_PROP_MITM_MASK) {
        DBG_PUTS("     mitm") ;
    }
    else {
        DBG_PUTS("     no mitm") ;
    }

    DBG_PRINTF("     authErr: %d", au->authErr) ;
}

#else
void stampa_au(const char * titolo, const void * v)
{
    UNUSED(titolo) ;
    UNUSED(v) ;
}

#endif

#if 0
void stampa_CYBLE_STATE_T(CYBLE_STATE_T s)
{
    switch (s) {
    case CYBLE_STATE_STOPPED:      // BLE is turned off
        DBG_PUTS("CYBLE_STATE_STOPPED") ;
        break ;
    case CYBLE_STATE_INITIALIZING: // Initializing state
        DBG_PUTS("CYBLE_STATE_INITIALIZING") ;
        break ;
    case CYBLE_STATE_CONNECTED:    // Peer device is connected
        DBG_PUTS("CYBLE_STATE_CONNECTED") ;
        break ;
    case CYBLE_STATE_ADVERTISING:  // Advertising process
        DBG_PUTS("CYBLE_STATE_ADVERTISING") ;
        break ;
//          case CYBLE_STATE_SCANNING    : // Scanning process
//              DBG_PUTS("CYBLE_STATE_SCANNING") ;
//              break ;
//          case CYBLE_STATE_CONNECTING  : // Connecting
//              DBG_PUTS("CYBLE_STATE_CONNECTING") ;
//              break ;
    case CYBLE_STATE_DISCONNECTED: // Essentially idle state
        DBG_PUTS("CYBLE_STATE_DISCONNECTED") ;
        break ;
    default:
        DBG_PRINTF("CyBle_GetState = ? %d ?\n", s) ;
        break ;
    }
}

#else
void stampa_CYBLE_STATE_T(CYBLE_STATE_T s)
{
    UNUSED(s) ;
}

#endif

#if 1
void stampa_evento(const char * titolo, uint32_t evn)
{
    switch (evn) {
    case CYBLE_EVT_HOST_INVALID:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_HOST_INVALID") ;
        break ;
    case CYBLE_EVT_STACK_ON:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_STACK_ON") ;
        break ;
    case CYBLE_EVT_TIMEOUT:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_TIMEOUT") ;
        break ;
    case CYBLE_EVT_HARDWARE_ERROR:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_HARDWARE_ERROR") ;
        break ;
    case CYBLE_EVT_HCI_STATUS:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_HCI_STATUS") ;
        break ;
    case CYBLE_EVT_STACK_BUSY_STATUS:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_STACK_BUSY_STATUS") ;
        break ;
    case CYBLE_EVT_MEMORY_REQUEST:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_MEMORY_REQUEST") ;
        break ;
    case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT") ;
        break ;
    case CYBLE_EVT_GAP_AUTH_REQ:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_AUTH_REQ") ;
        break ;
    case CYBLE_EVT_GAP_PASSKEY_ENTRY_REQUEST:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_PASSKEY_ENTRY_REQUEST") ;
        break ;
    case CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST") ;
        break ;
    case CYBLE_EVT_GAP_AUTH_COMPLETE:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_AUTH_COMPLETE") ;
        break ;
    case CYBLE_EVT_GAP_AUTH_FAILED:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_AUTH_FAILED") ;
        break ;
    case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP") ;
        break ;
    case CYBLE_EVT_GAP_DEVICE_CONNECTED:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_DEVICE_CONNECTED") ;
        break ;
    case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_DEVICE_DISCONNECTED") ;
        break ;
    case CYBLE_EVT_GAP_ENCRYPT_CHANGE:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_ENCRYPT_CHANGE") ;
        break ;
    case CYBLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE") ;
        break ;
    case CYBLE_EVT_GAPC_SCAN_START_STOP:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAPC_SCAN_START_STOP") ;
        break ;
    case CYBLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT") ;
        break ;
    case CYBLE_EVT_GAP_NUMERIC_COMPARISON_REQUEST:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_NUMERIC_COMPARISON_REQUEST") ;
        break ;
    case CYBLE_EVT_GAP_KEYPRESS_NOTIFICATION:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_KEYPRESS_NOTIFICATION") ;
        break ;
    case CYBLE_EVT_GAP_OOB_GENERATED_NOTIFICATION:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_OOB_GENERATED_NOTIFICATION") ;
        break ;
    case CYBLE_EVT_GAP_DATA_LENGTH_CHANGE:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_DATA_LENGTH_CHANGE") ;
        break ;
    case CYBLE_EVT_GAP_ENHANCE_CONN_COMPLETE:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_ENHANCE_CONN_COMPLETE") ;
        break ;
    case CYBLE_EVT_GAPC_DIRECT_ADV_REPORT:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAPC_DIRECT_ADV_REPORT") ;
        break ;
    case CYBLE_EVT_GAP_SMP_NEGOTIATED_AUTH_INFO:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_SMP_NEGOTIATED_AUTH_INFO") ;
        break ;
    case CYBLE_EVT_GAP_CONN_ESTB:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_CONN_ESTB") ;
        break ;
    case CYBLE_EVT_GAP_SCAN_REQ_RECVD:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_SCAN_REQ_RECVD") ;
        break ;
    case CYBLE_EVT_GAP_AUTH_REQ_REPLY_ERR:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GAP_AUTH_REQ_REPLY_ERR") ;
        break ;
    case CYBLE_EVT_GAP_SMP_LOC_P256_KEYS_GEN_AND_SET_COMPLETE:
        DBG_PRINTF("%s: %s",
                   titolo,
                   "CYBLE_EVT_GAP_SMP_LOC_P256_KEYS_GEN_AND_SET_COMPLETE") ;
        break ;
    case CYBLE_EVT_GATTC_ERROR_RSP:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTC_ERROR_RSP") ;
        break ;
    case CYBLE_EVT_GATT_CONNECT_IND:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATT_CONNECT_IND") ;
        break ;
    case CYBLE_EVT_GATT_DISCONNECT_IND:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATT_DISCONNECT_IND") ;
        break ;
    case CYBLE_EVT_GATTS_XCNHG_MTU_REQ:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTS_XCNHG_MTU_REQ") ;
        break ;
    case CYBLE_EVT_GATTC_XCHNG_MTU_RSP:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTC_XCHNG_MTU_RSP") ;
        break ;
    case CYBLE_EVT_GATTC_READ_BY_GROUP_TYPE_RSP:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTC_READ_BY_GROUP_TYPE_RSP") ;
        break ;
    case CYBLE_EVT_GATTC_READ_BY_TYPE_RSP:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTC_READ_BY_TYPE_RSP") ;
        break ;
    case CYBLE_EVT_GATTC_FIND_INFO_RSP:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTC_FIND_INFO_RSP") ;
        break ;
    case CYBLE_EVT_GATTC_FIND_BY_TYPE_VALUE_RSP:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTC_FIND_BY_TYPE_VALUE_RSP") ;
        break ;
    case CYBLE_EVT_GATTC_READ_RSP:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTC_READ_RSP") ;
        break ;
    case CYBLE_EVT_GATTC_READ_BLOB_RSP:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTC_READ_BLOB_RSP") ;
        break ;
    case CYBLE_EVT_GATTC_READ_MULTI_RSP:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTC_READ_MULTI_RSP") ;
        break ;
    case CYBLE_EVT_GATTS_WRITE_REQ:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTS_WRITE_REQ") ;
        break ;
    case CYBLE_EVT_GATTC_WRITE_RSP:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTC_WRITE_RSP") ;
        break ;
    case CYBLE_EVT_GATTS_WRITE_CMD_REQ:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTS_WRITE_CMD_REQ") ;
        break ;
    case CYBLE_EVT_GATTS_PREP_WRITE_REQ:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTS_PREP_WRITE_REQ") ;
        break ;
    case CYBLE_EVT_GATTS_EXEC_WRITE_REQ:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTS_EXEC_WRITE_REQ") ;
        break ;
    case CYBLE_EVT_GATTC_EXEC_WRITE_RSP:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTC_EXEC_WRITE_RSP") ;
        break ;
    case CYBLE_EVT_GATTC_HANDLE_VALUE_NTF:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTC_HANDLE_VALUE_NTF") ;
        break ;
    case CYBLE_EVT_GATTC_HANDLE_VALUE_IND:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTC_HANDLE_VALUE_IND") ;
        break ;
    case CYBLE_EVT_GATTS_HANDLE_VALUE_CNF:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTS_HANDLE_VALUE_CNF") ;
        break ;
    case CYBLE_EVT_GATTS_DATA_SIGNED_CMD_REQ:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTS_DATA_SIGNED_CMD_REQ") ;
        break ;
    case CYBLE_EVT_GATTC_STOP_CMD_COMPLETE:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTC_STOP_CMD_COMPLETE") ;
        break ;
    case CYBLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ") ;
        break ;
    case CYBLE_EVT_GATTC_LONG_PROCEDURE_END:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_GATTC_LONG_PROCEDURE_END") ;
        break ;
    case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ") ;
        break ;
    case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP") ;
        break ;
    case CYBLE_EVT_L2CAP_COMMAND_REJ:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_L2CAP_COMMAND_REJ") ;
        break ;
    case CYBLE_EVT_L2CAP_CBFC_CONN_IND:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_L2CAP_CBFC_CONN_IND") ;
        break ;
    case CYBLE_EVT_L2CAP_CBFC_CONN_CNF:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_L2CAP_CBFC_CONN_CNF") ;
        break ;
    case CYBLE_EVT_L2CAP_CBFC_DISCONN_IND:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_L2CAP_CBFC_DISCONN_IND") ;
        break ;
    case CYBLE_EVT_L2CAP_CBFC_DISCONN_CNF:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_L2CAP_CBFC_DISCONN_CNF") ;
        break ;
    case CYBLE_EVT_L2CAP_CBFC_DATA_READ:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_L2CAP_CBFC_DATA_READ") ;
        break ;
    case CYBLE_EVT_L2CAP_CBFC_RX_CREDIT_IND:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_L2CAP_CBFC_RX_CREDIT_IND") ;
        break ;
    case CYBLE_EVT_L2CAP_CBFC_TX_CREDIT_IND:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_L2CAP_CBFC_TX_CREDIT_IND") ;
        break ;
    case CYBLE_EVT_L2CAP_CBFC_DATA_WRITE_IND:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_L2CAP_CBFC_DATA_WRITE_IND") ;
        break ;
#ifdef CYBLE_HOST_QUALIFICATION
    case CYBLE_EVT_QUAL_SMP_PAIRING_REQ_RSP:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_QUAL_SMP_PAIRING_REQ_RSP") ;
        break ;
    case CYBLE_EVT_QUAL_SMP_LOCAL_PUBLIC_KEY:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_QUAL_SMP_LOCAL_PUBLIC_KEY") ;
        break ;
    case CYBLE_EVT_QUAL_SMP_PAIRING_FAILED_CMD:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_QUAL_SMP_PAIRING_FAILED_CMD") ;
        break ;
#endif /* CYBLE_HOST_QUALIFICATION */
    case CYBLE_EVT_PENDING_FLASH_WRITE:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_PENDING_FLASH_WRITE") ;
        break ;
    case CYBLE_EVT_LE_PING_AUTH_TIMEOUT:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_LE_PING_AUTH_TIMEOUT") ;
        break ;
    case CYBLE_EVT_HCI_PKT:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_HCI_PKT") ;
        break ;
    case CYBLE_EVT_FLASH_CORRUPT:
        DBG_PRINTF("%s: %s", titolo, "CYBLE_EVT_FLASH_CORRUPT") ;
        break ;
    default:
        DBG_PRINTF("BLE evn %d = 0x%02X", evn, evn) ;
        break ;
    }
}

#else
void stampa_evento(const char * titolo, uint32_t evn)
{
    UNUSED(titolo) ;
    UNUSED(evn) ;
}

#endif

#if 1
void stampa_CYBLE_GATT_ERR_CODE_T(CYBLE_GATT_ERR_CODE_T err)
{
    switch (err) {
    case CYBLE_GATT_ERR_NONE:
        break ;
    case CYBLE_GATT_ERR_INVALID_HANDLE:
        DBG_PUTS("CYBLE_GATT_ERR_INVALID_HANDLE") ;
        break ;
    case CYBLE_GATT_ERR_READ_NOT_PERMITTED:
        DBG_PUTS("CYBLE_GATT_ERR_READ_NOT_PERMITTED") ;
        break ;
    case CYBLE_GATT_ERR_WRITE_NOT_PERMITTED:
        DBG_PUTS("CYBLE_GATT_ERR_WRITE_NOT_PERMITTED") ;
        break ;
    case CYBLE_GATT_ERR_INVALID_PDU:
        DBG_PUTS("CYBLE_GATT_ERR_INVALID_PDU") ;
        break ;
    case CYBLE_GATT_ERR_INSUFFICIENT_AUTHENTICATION:
        DBG_PUTS("CYBLE_GATT_ERR_INSUFFICIENT_AUTHENTICATION") ;
        break ;
    case CYBLE_GATT_ERR_REQUEST_NOT_SUPPORTED:
        DBG_PUTS("CYBLE_GATT_ERR_REQUEST_NOT_SUPPORTED") ;
        break ;
    case CYBLE_GATT_ERR_INVALID_OFFSET:
        DBG_PUTS("CYBLE_GATT_ERR_INVALID_OFFSET") ;
        break ;
    case CYBLE_GATT_ERR_INSUFFICIENT_AUTHORIZATION:
        DBG_PUTS("CYBLE_GATT_ERR_INSUFFICIENT_AUTHORIZATION") ;
        break ;
    case CYBLE_GATT_ERR_PREPARE_WRITE_QUEUE_FULL:
        DBG_PUTS("CYBLE_GATT_ERR_PREPARE_WRITE_QUEUE_FULL") ;
        break ;
    case CYBLE_GATT_ERR_ATTRIBUTE_NOT_FOUND:
        DBG_PUTS("CYBLE_GATT_ERR_ATTRIBUTE_NOT_FOUND") ;
        break ;
    case CYBLE_GATT_ERR_ATTRIBUTE_NOT_LONG:
        DBG_PUTS("CYBLE_GATT_ERR_ATTRIBUTE_NOT_LONG") ;
        break ;
    case CYBLE_GATT_ERR_INSUFFICIENT_ENC_KEY_SIZE:
        DBG_PUTS("CYBLE_GATT_ERR_INSUFFICIENT_ENC_KEY_SIZE") ;
        break ;
    case CYBLE_GATT_ERR_INVALID_ATTRIBUTE_LEN:
        DBG_PUTS("CYBLE_GATT_ERR_INVALID_ATTRIBUTE_LEN") ;
        break ;
    case CYBLE_GATT_ERR_UNLIKELY_ERROR:
        DBG_PUTS("CYBLE_GATT_ERR_UNLIKELY_ERROR") ;
        break ;
    case CYBLE_GATT_ERR_INSUFFICIENT_ENCRYPTION:
        DBG_PUTS("CYBLE_GATT_ERR_INSUFFICIENT_ENCRYPTION") ;
        break ;
    case CYBLE_GATT_ERR_UNSUPPORTED_GROUP_TYPE:
        DBG_PUTS("CYBLE_GATT_ERR_UNSUPPORTED_GROUP_TYPE") ;
        break ;
    case CYBLE_GATT_ERR_INSUFFICIENT_RESOURCE:
        DBG_PUTS("CYBLE_GATT_ERR_INSUFFICIENT_RESOURCE") ;
        break ;
    case CYBLE_GATT_ERR_TRIGGER_CODITION_VALUE_NOT_SUPPORTED:
        DBG_PUTS("CYBLE_GATT_ERR_TRIGGER_CODITION_VALUE_NOT_SUPPORTED") ;
        break ;
    /*  stesso valore!
        case CYBLE_GATT_ERR_HEART_RATE_CONTROL_POINT_NOT_SUPPORTED: DBG_PUTS("CYBLE_GATT_ERR_HEART_RATE_CONTROL_POINT_NOT_SUPPORTED") ; break ;
        case CYBLE_GATT_ERR_USER_DATA_ACCESS_NOT_PERMITTED: DBG_PUTS("CYBLE_GATT_ERR_USER_DATA_ACCESS_NOT_PERMITTED") ; break ;
        case CYBLE_GATT_ERR_CPS_INAPPROPRIATE_CONNECTION_PARAMETERS: DBG_PUTS("CYBLE_GATT_ERR_CPS_INAPPROPRIATE_CONNECTION_PARAMETERS") ; break ;
        case CYBLE_GATT_ERR_HTS_OUT_OF_RANGE: DBG_PUTS("CYBLE_GATT_ERR_HTS_OUT_OF_RANGE") ; break ;
        case CYBLE_GATTS_ERR_PROCEDURE_ALREADY_IN_PROGRESS: DBG_PUTS("CYBLE_GATTS_ERR_PROCEDURE_ALREADY_IN_PROGRESS") ; break ;
        case CYBLE_GATT_ERR_OP_CODE_NOT_SUPPORTED: DBG_PUTS("CYBLE_GATT_ERR_OP_CODE_NOT_SUPPORTED") ; break ;
        case CYBLE_GATT_ERR_MISSING_CRC: DBG_PUTS("CYBLE_GATT_ERR_MISSING_CRC") ; break ;
    */
    case CYBLE_GATTS_ERR_CCCD_IMPROPERLY_CONFIGURED:
        DBG_PUTS("CYBLE_GATTS_ERR_CCCD_IMPROPERLY_CONFIGURED") ;
        break ;
    /*  stesso valore!
        case CYBLE_GATTS_ERR_OPERATION_FAILED: DBG_PUTS("CYBLE_GATTS_ERR_OPERATION_FAILED") ; break ;
        case CYBLE_GATT_ERR_INVALID_CRC: DBG_PUTS("CYBLE_GATT_ERR_INVALID_CRC") ; break ;
        case CYBLE_GATTS_ERR_HPS_INVALID_REQUEST: DBG_PUTS("CYBLE_GATTS_ERR_HPS_INVALID_REQUEST") ; break ;
    */
    case CYBLE_GATTS_ERR_NETWORK_NOT_AVAILABLE:
        DBG_PUTS("CYBLE_GATTS_ERR_NETWORK_NOT_AVAILABLE") ;
        break ;
    case CYBLE_GATT_ERR_ANS_COMMAND_NOT_SUPPORTED:
        DBG_PUTS("CYBLE_GATT_ERR_ANS_COMMAND_NOT_SUPPORTED") ;
        break ;
    /*  stesso valore!
        case CYBLE_GATT_ERR_ANCS_UNKNOWN_COMMAND: DBG_PUTS("CYBLE_GATT_ERR_ANCS_UNKNOWN_COMMAND") ; break ;
    */
    case CYBLE_GATT_ERR_ANCS_INVALID_COMMAND:
        DBG_PUTS("CYBLE_GATT_ERR_ANCS_INVALID_COMMAND") ;
        break ;
    case CYBLE_GATT_ERR_ANCS_INVALID_PARAMETER:
        DBG_PUTS("CYBLE_GATT_ERR_ANCS_INVALID_PARAMETER") ;
        break ;
    case CYBLE_GATT_ERR_ANCS_ACTION_FAILED:
        DBG_PUTS("CYBLE_GATT_ERR_ANCS_ACTION_FAILED") ;
        break ;
    case CYBLE_GATT_ERR_CCCD_IMPROPERLY_CONFIGURED:
        DBG_PUTS("CYBLE_GATT_ERR_CCCD_IMPROPERLY_CONFIGURED") ;
        break ;
    case CYBLE_GATT_ERR_PROCEDURE_ALREADY_IN_PROGRESS:
        DBG_PUTS("CYBLE_GATT_ERR_PROCEDURE_ALREADY_IN_PROGRESS") ;
        break ;
    case CYBLE_GATT_ERR_OUT_OF_RANGE:
        DBG_PUTS("CYBLE_GATT_ERR_OUT_OF_RANGE") ;
        break ;
    default:
        DBG_PRINTF("BLE err %d = 0x%02X", err, err) ;
        break ;
    }
}

#else
void stampa_CYBLE_GATT_ERR_CODE_T(CYBLE_GATT_ERR_CODE_T err)
{
    UNUSED(err) ;
}

#endif

#if 1
void stampa_CYBLE_API_RESULT_T(CYBLE_API_RESULT_T err)
{
    switch (err) {
    case CYBLE_ERROR_OK:
        //DBG_PUTS("CYBLE_ERROR_OK") ;
        break ;
    case CYBLE_ERROR_INVALID_PARAMETER:
        DBG_PUTS("CYBLE_ERROR_INVALID_PARAMETER") ;
        break ;
    case CYBLE_ERROR_INVALID_OPERATION:
        DBG_PUTS("CYBLE_ERROR_INVALID_OPERATION") ;
        break ;
    case CYBLE_ERROR_MEMORY_ALLOCATION_FAILED:
        DBG_PUTS("CYBLE_ERROR_MEMORY_ALLOCATION_FAILED") ;
        break ;
    case CYBLE_ERROR_INSUFFICIENT_RESOURCES:
        DBG_PUTS("CYBLE_ERROR_INSUFFICIENT_RESOURCES") ;
        break ;
    case CYBLE_ERROR_OOB_NOT_AVAILABLE:
        DBG_PUTS("CYBLE_ERROR_OOB_NOT_AVAILABLE") ;
        break ;
    case CYBLE_ERROR_NO_CONNECTION:
        DBG_PUTS("CYBLE_ERROR_NO_CONNECTION") ;
        break ;
    case CYBLE_ERROR_NO_DEVICE_ENTITY:
        DBG_PUTS("CYBLE_ERROR_NO_DEVICE_ENTITY") ;
        break ;
    case CYBLE_ERROR_REPEATED_ATTEMPTS:
        DBG_PUTS("CYBLE_ERROR_REPEATED_ATTEMPTS") ;
        break ;
    case CYBLE_ERROR_GAP_ROLE:
        DBG_PUTS("CYBLE_ERROR_GAP_ROLE") ;
        break ;
    case CYBLE_ERROR_TX_POWER_READ:
        DBG_PUTS("CYBLE_ERROR_TX_POWER_READ") ;
        break ;
    case CYBLE_ERROR_BT_ON_NOT_COMPLETED:
        DBG_PUTS("CYBLE_ERROR_BT_ON_NOT_COMPLETED") ;
        break ;
    case CYBLE_ERROR_SEC_FAILED:
        DBG_PUTS("CYBLE_ERROR_SEC_FAILED") ;
        break ;
    case CYBLE_ERROR_L2CAP_PSM_WRONG_ENCODING:
        DBG_PUTS("CYBLE_ERROR_L2CAP_PSM_WRONG_ENCODING") ;
        break ;
    case CYBLE_ERROR_L2CAP_PSM_ALREADY_REGISTERED:
        DBG_PUTS("CYBLE_ERROR_L2CAP_PSM_ALREADY_REGISTERED") ;
        break ;
    case CYBLE_ERROR_L2CAP_PSM_NOT_REGISTERED:
        DBG_PUTS("CYBLE_ERROR_L2CAP_PSM_NOT_REGISTERED") ;
        break ;
    case CYBLE_ERROR_L2CAP_CONNECTION_ENTITY_NOT_FOUND:
        DBG_PUTS("CYBLE_ERROR_L2CAP_CONNECTION_ENTITY_NOT_FOUND") ;
        break ;
    case CYBLE_ERROR_L2CAP_CHANNEL_NOT_FOUND:
        DBG_PUTS("CYBLE_ERROR_L2CAP_CHANNEL_NOT_FOUND") ;
        break ;
    case CYBLE_ERROR_L2CAP_PSM_NOT_IN_RANGE:
        DBG_PUTS("CYBLE_ERROR_L2CAP_PSM_NOT_IN_RANGE") ;
        break ;
    case CYBLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE:
        DBG_PUTS("CYBLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE") ;
        break ;
    case CYBLE_ERROR_DEVICE_ALREADY_EXISTS:
        DBG_PUTS("CYBLE_ERROR_DEVICE_ALREADY_EXISTS") ;
        break ;
    case CYBLE_ERROR_FLASH_WRITE_NOT_PERMITED:
        DBG_PUTS("CYBLE_ERROR_FLASH_WRITE_NOT_PERMITED") ;
        break ;
    case CYBLE_ERROR_MIC_AUTH_FAILED:
        DBG_PUTS("CYBLE_ERROR_MIC_AUTH_FAILED") ;
        break ;
    case CYBLE_ERROR_HARDWARE_FAILURE:
        DBG_PUTS("CYBLE_ERROR_HARDWARE_FAILURE") ;
        break ;
    case CYBLE_ERROR_UNSUPPORTED_FEATURE_OR_PARAMETER_VALUE:
        DBG_PUTS("CYBLE_ERROR_UNSUPPORTED_FEATURE_OR_PARAMETER_VALUE") ;
        break ;
    case CYBLE_ERROR_FLASH_WRITE:
        DBG_PUTS("CYBLE_ERROR_FLASH_WRITE") ;
        break ;
    case CYBLE_ERROR_LL_SAME_TRANSACTION_COLLISION:
        DBG_PUTS("CYBLE_ERROR_LL_SAME_TRANSACTION_COLLISION") ;
        break ;
    case CYBLE_ERROR_CONTROLLER_BUSY:
        DBG_PUTS("CYBLE_ERROR_CONTROLLER_BUSY") ;
        break ;
    case CYBLE_ERROR_MAX:
        DBG_PUTS("CYBLE_ERROR_MAX") ;
        break ;
    case CYBLE_ERROR_NTF_DISABLED:
        DBG_PUTS("CYBLE_ERROR_NTF_DISABLED") ;
        break ;
    case CYBLE_ERROR_IND_DISABLED:
        DBG_PUTS("CYBLE_ERROR_IND_DISABLED") ;
        break ;
    case CYBLE_ERROR_CHAR_IS_NOT_DISCOVERED:
        DBG_PUTS("CYBLE_ERROR_CHAR_IS_NOT_DISCOVERED") ;
        break ;
    case CYBLE_ERROR_INVALID_STATE:
        DBG_PUTS("CYBLE_ERROR_INVALID_STATE") ;
        break ;
    case CYBLE_ERROR_STACK_BUSY:
        DBG_PUTS("CYBLE_ERROR_STACK_BUSY") ;
        break ;
    default:
        DBG_PRINTF("CYBLE_API_RESULT_T ? 0x%02X ?", err) ;
        break ;
    }
}

#else
void stampa_CYBLE_API_RESULT_T(CYBLE_API_RESULT_T err)
{
    UNUSED(err) ;
}

#endif

#else
void stampa_CYBLE_STATE_T(CYBLE_STATE_T s)
{
    UNUSED(s) ;
}

void stampa_evento(const char * titolo, uint32_t evn)
{
    UNUSED(titolo) ;
    UNUSED(evn) ;
}

void stampa_CYBLE_GATT_ERR_CODE_T(CYBLE_GATT_ERR_CODE_T err)
{
    UNUSED(err) ;
}

void stampa_CYBLE_API_RESULT_T(CYBLE_API_RESULT_T err)
{
    UNUSED(err) ;
}

void stampa_au(const char * titolo, const void * v)
{
    UNUSED(titolo) ;
    UNUSED(v) ;
}

void stampa_bonded(CYBLE_GAP_BONDED_DEV_ADDR_LIST_T * a)
{
    UNUSED(a) ;
}

#endif

#endif
