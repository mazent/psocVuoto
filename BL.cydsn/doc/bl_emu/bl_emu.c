#define STAMPA_DBG
#include "utili/includimi.h"
#include "soc/soc.h"
#include "ExternalMemoryInterface.h"

extern void BootloaderEmulator_iniz(void) ;
extern void BootloaderEmulator_HostLink(const uint8) ;

static bool abil = false ;

static void poll_host_link(void * v)
{
	UNUSED(v) ;

	if (abil) {
		BootloaderEmulator_HostLink(1) ;

		SOC_apc(APC_BL_EMU, poll_host_link) ;
	}
}

void EMU_iniz(void)
{
	EMI_Start() ;
	BootloaderEmulator_iniz() ;

	abil = true ;

	SOC_apc(APC_BL_EMU, poll_host_link) ;
}

bool EMU_incorso(void)
{
	return abil ;
}

void EMU_fine(void)
{
	EMI_stop() ;

	abil = false ;
}

//bool EMU_run(void)
//{
//	if (abil)
//		BootloaderEmulator_HostLink(1) ;
//
//	return abil ;
//}

