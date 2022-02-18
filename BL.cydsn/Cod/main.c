#define STAMPA_DBG
#include "main.h"

#include "Options.h"

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  Starts the bootloader component.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

// Condivisa con APP
CY_NOINIT uint32_t causa ;

int main()
{
    CyGlobalIntEnable;

    DBG_INIZ ;

    DBG_PRINT_TEXT("");
    DBG_PRINT_TEXT("");
    DBG_PRINT_TEXT("===============================================================================");
    DBG_PRINT_TEXT("=              BLE_External_Memory_Bootloader Application Started              ");
    DBG_PRINT_TEXT("=              Version: 1.0                                                    ");
    DBG_PRINTF    ("=              Compile Date and Time : %s %s                                   ", __DATE__,__TIME__);
    DBG_PRINT_TEXT("===============================================================================");
    DBG_PRINT_TEXT("");
    DBG_PRINT_TEXT("");

    causa = CySysGetResetReason(CY_SYS_RESET_WDT | CY_SYS_RESET_PROTFAULT | CY_SYS_RESET_SW) ;
    DBG_PRINTF("causa %08X", causa) ;

	CyBle_AesCcmInit();    
    
    Bootloader_Start();
    
    for(;;)
    {
        /* Place your application code here. */
    }
}


/* [] END OF FILE */
