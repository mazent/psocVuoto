#include "../xmem/at25sf041.h"
#include "fspi.h"

#ifdef CY_PINS_SPI_CS_N_H

// https://community.cypress.com/docs/DOC-9250
// https://community.cypress.com/docs/DOC-14313


#define CMD_RDID		0x9F
//#define CMD_RUID 		0x4B
#define CMD_READ 		0x03
#define CMD_WREN		0x06
//#define CMD_WRDI		0x04
#define CMD_PP 			0x02
#define CMD_RDSR1 		0x05
//#define CMD_RDSR2 		0x07
//#define CMD_SE 			0x20
//#define CMD_HBE 		0x52
#define CMD_BE 			0xD8
//#define CMD_CE 			0x60
//#define CMD_SECRE 		0x44
//#define CMD_SECRP 		0x42
//#define CMD_SECRR 		0x48
//#define CMD_RDCR1 		0x35
//#define CMD_RDCR2 		0x15
//#define CMD_RDCR3 		0x33
//#define CMD_CLSR 		0x30
//#define CMD_PES 		0x75
//#define CMD_RSTEN 		0x66
//#define CMD_RST 		0x99

// servono i bit A23..A0 in big-endian

static uint32_t gira_indirizzo(uint32_t addr)
{
    addr = __REV(addr) ;
    addr >>= 8 ;

    return addr ;
}


void AT25_Read_Identification(AT25_READ_ID * pRI)
{
	uint8_t cmd = CMD_RDID ;
	uint8_t rsp[3] ;

	SPI_CS_N_Write(0) ;

	FSPI_write(&cmd, sizeof(cmd)) ;
	FSPI_read(rsp, sizeof(rsp)) ;

	SPI_CS_N_Write(1) ;

	pRI->manuf = rsp[0] ;
	memcpy(&pRI->device, rsp + 1, 2) ;
}

//void AT25_Read_Unique_ID(AT25_READ_U_ID * pUI)
//{
//	uint8_t cmd = CMD_RUID ;
//
//	SPI_CS_N_Write(0) ;
//
//	FSPI_write(&cmd, sizeof(cmd)) ;
//
//	FSPI_dummy_read(4) ;
//
//	FSPI_read(pUI->id, 8) ;
//
//	SPI_CS_N_Write(1) ;
//}

void AT25_Read(uint32_t addr, void * v, size_t dim)
{
	uint8_t cmd[1 + 3] = { CMD_READ } ;

    addr = gira_indirizzo(addr) ;

	memcpy(cmd + 1, &addr, 3) ;

	SPI_CS_N_Write(0) ;

	FSPI_write(cmd, sizeof(cmd)) ;
	FSPI_read(v, dim) ;

	SPI_CS_N_Write(1) ;
}

void AT25_Write_Enable(void)
{
	uint8_t cmd = CMD_WREN ;

	SPI_CS_N_Write(0) ;

	FSPI_write(&cmd, 1) ;

	SPI_CS_N_Write(1) ;
}

//void S25_Write_Disable(void)
//{
//	uint8_t cmd = CMD_WRDI ;
//
//	SPI_CS_N_Write(0) ;
//
//	FSPI_write(&cmd, 1) ;
//
//	SPI_CS_N_Write(1) ;
//}
//
//void S25_Clear_Status_Register(void)
//{
//	uint8_t cmd = CMD_CLSR ;
//
//	SPI_CS_N_Write(0) ;
//
//	FSPI_write(&cmd, sizeof(cmd)) ;
//
//	SPI_CS_N_Write(1) ;
//}

size_t AT25_Write(uint32_t addr, const void * v, size_t dim)
{
	uint8_t cmd[1 + 3] = { CMD_PP } ;

	{
		// Devo rimanere all'interno della pagina
		uint32_t addr_max = addr + AT25SF041_WRITE_PAGE_BUFFER_SIZE ;

		// tolgo i bit meno significativi -> inizio pag successiva
		addr_max &= NOT(AT25SF041_WRITE_PAGE_BUFFER_SIZE - 1) ;

		addr_max = MIN(addr_max, addr + dim) ;
		dim = addr_max - addr ;
	}

    addr = gira_indirizzo(addr) ;

	memcpy(cmd + 1, &addr, 3) ;

	SPI_CS_N_Write(0) ;

	FSPI_write(cmd, sizeof(cmd)) ;

	const size_t DIM = MIN(dim, AT25SF041_WRITE_PAGE_BUFFER_SIZE) ;
	FSPI_write(v, DIM) ;

	SPI_CS_N_Write(1) ;

	return DIM ;
}

uint8_t AT25_Read_Status_Register_1(void)
{
	uint8_t cmd = CMD_RDSR1 ;

	SPI_CS_N_Write(0) ;

	FSPI_write(&cmd, sizeof(cmd)) ;

	FSPI_read(&cmd, 1) ;

	SPI_CS_N_Write(1) ;

	return cmd ;
}

//uint8_t S25_Read_Status_Register_2(void)
//{
//	uint8_t cmd = CMD_RDSR2 ;
//
//	SPI_CS_N_Write(0) ;
//
//	FSPI_write(&cmd, sizeof(cmd)) ;
//
//	FSPI_read(&cmd, 1) ;
//
//	SPI_CS_N_Write(1) ;
//
//	return cmd ;
//}
//
//void S25_Sector_Erase(uint32_t addr)
//{
//	uint8_t cmd[1 + 3] = { CMD_SE } ;
//
//    addr = gira_indirizzo(addr) ;
//
//	memcpy(cmd + 1, &addr, 3) ;
//
//	SPI_CS_N_Write(0) ;
//
//	FSPI_write(cmd, sizeof(cmd)) ;
//
//	SPI_CS_N_Write(1) ;
//}
//
//void S25_Half_Block_Erase(uint32_t addr)
//{
//	uint8_t cmd[1 + 3] = { CMD_HBE } ;
//
//    addr = gira_indirizzo(addr) ;
//
//	memcpy(cmd + 1, &addr, 3) ;
//
//	SPI_CS_N_Write(0) ;
//
//	FSPI_write(cmd, sizeof(cmd)) ;
//
//	SPI_CS_N_Write(1) ;
//}

void AT25_Block_Erase(uint32_t addr)
{
	uint8_t cmd[1 + 3] = { CMD_BE } ;

    addr = gira_indirizzo(addr) ;

	memcpy(cmd + 1, &addr, 3) ;

	SPI_CS_N_Write(0) ;

	FSPI_write(cmd, sizeof(cmd)) ;

	SPI_CS_N_Write(1) ;
}

//void S25_Chip_Erase(void)
//{
//	uint8_t cmd = CMD_CE ;
//
//	SPI_CS_N_Write(0) ;
//
//	FSPI_write(&cmd, 1) ;
//
//	SPI_CS_N_Write(1) ;
//}
//
//void S25_Program_or_Erase_Suspend(void)
//{
//	uint8_t cmd = CMD_PES ;
//
//	SPI_CS_N_Write(0) ;
//
//	FSPI_write(&cmd, 1) ;
//
//	SPI_CS_N_Write(1) ;
//}
//
//uint8_t S25_Read_Configuration_Register_1(void)
//{
//	uint8_t cmd = CMD_RDCR1 ;
//
//	SPI_CS_N_Write(0) ;
//
//	FSPI_write(&cmd, sizeof(cmd)) ;
//
//	FSPI_read(&cmd, 1) ;
//
//	SPI_CS_N_Write(1) ;
//
//	return cmd ;
//}
//
//uint8_t S25_Read_Configuration_Register_2(void)
//{
//	uint8_t cmd = CMD_RDCR2 ;
//
//	SPI_CS_N_Write(0) ;
//
//	FSPI_write(&cmd, sizeof(cmd)) ;
//
//	FSPI_read(&cmd, 1) ;
//
//	SPI_CS_N_Write(1) ;
//
//	return cmd ;
//}
//
//uint8_t S25_Read_Configuration_Register_3(void)
//{
//	uint8_t cmd = CMD_RDCR3 ;
//
//	SPI_CS_N_Write(0) ;
//
//	FSPI_write(&cmd, sizeof(cmd)) ;
//
//	FSPI_read(&cmd, 1) ;
//
//	SPI_CS_N_Write(1) ;
//
//	return cmd ;
//}
//
//void S25_Security_Region_Erase(uint32_t addr)
//{
//	uint8_t cmd[1 + 3] = { CMD_SECRE } ;
//
//    addr = gira_indirizzo(addr) ;
//
//	memcpy(cmd + 1, &addr, 3) ;
//
//	SPI_CS_N_Write(0) ;
//
//	FSPI_write(cmd, sizeof(cmd)) ;
//
//	SPI_CS_N_Write(1) ;
//}
//
//void S25_Security_Region_Program(uint32_t addr, const void * v, size_t dim)
//{
//	uint8_t cmd[1 + 3] = { CMD_SECRP } ;
//
//    addr = gira_indirizzo(addr) ;
//
//	memcpy(cmd + 1, &addr, 3) ;
//
//	SPI_CS_N_Write(0) ;
//
//	FSPI_write(cmd, sizeof(cmd)) ;
//
//	const size_t DIM = MIN(dim, AT25SF041_WRITE_PAGE_BUFFER_SIZE) ;
//	FSPI_write(v, DIM) ;
//
//	SPI_CS_N_Write(1) ;
//}
//
//void S25_Security_Regions_Read(uint32_t addr, void * v, size_t dim)
//{
//	uint8_t cmd[1 + 3] = { CMD_SECRR } ;
//
//    addr = gira_indirizzo(addr) ;
//
//	memcpy(cmd + 1, &addr, 3) ;
//
//	SPI_CS_N_Write(0) ;
//
//	FSPI_write(cmd, sizeof(cmd)) ;
//
//	FSPI_dummy_read(1) ;
//
//	FSPI_read(v, dim) ;
//
//	SPI_CS_N_Write(1) ;
//}
//
//void S25_Software_Reset(void)
//{
//	// Abilito
//	uint8_t cmd = CMD_RSTEN ;
//
//	SPI_CS_N_Write(0) ;
//
//	FSPI_write(&cmd, sizeof(cmd)) ;
//
//	SPI_CS_N_Write(1) ;
//
//	// Resetto
//	cmd = CMD_RST ;
//
//	SPI_CS_N_Write(0) ;
//
//	FSPI_write(&cmd, sizeof(cmd)) ;
//
//	SPI_CS_N_Write(1) ;
//}

#else

void AT25_Read_Identification(AT25_READ_ID * p)
{
	memset(p, 0, sizeof(AT25_READ_ID)) ;
}

#endif
