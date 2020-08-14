#include "s25fl064l.h"
#include "spi/fspi.h"

#ifdef CY_PINS_SPI_CS_N_H

// https://community.cypress.com/docs/DOC-9250
// https://community.cypress.com/docs/DOC-14313

#define DIM_PAGE_BUFFER		256

#define CMD_RDID		0x9F
#define CMD_RUID 		0x4B
#define CMD_READ 		0x03
#define CMD_WREN		0x06
#define CMD_WRDI		0x04
#define CMD_PP 			0x02
#define CMD_RDSR1 		0x05
#define CMD_RDSR2 		0x07
#define CMD_SE 			0x20
#define CMD_HBE 		0x52
#define CMD_BE 			0xD8
#define CMD_CE 			0x60
#define CMD_SECRE 		0x44
#define CMD_SECRP 		0x42
#define CMD_SECRR 		0x48
#define CMD_RDCR1 		0x35
#define CMD_RDCR2 		0x15
#define CMD_RDCR3 		0x33
#define CMD_CLSR 		0x30
#define CMD_PES 		0x75
#define CMD_RSTEN 		0x66
#define CMD_RST 		0x99

// servono i primi 3 byte in big-endian
static uint32_t gira_indirizzo(uint32_t addr)
{
    addr = __REV(addr) ;
    addr >>= 8 ;
    
    return addr ;
}
    
    
void S25_Read_Identification(S25_READ_ID * pRI)
{
	uint8_t cmd = CMD_RDID ;
	uint8_t rsp[4] ;

	SPI_CS_N_Write(0) ;

	FSPI_write(&cmd, sizeof(cmd)) ;
	FSPI_read(rsp, sizeof(rsp)) ;

	SPI_CS_N_Write(1) ;

	pRI->manuf = rsp[0] ;
	memcpy(&pRI->device, rsp + 1, 2) ;
}

void S25_Read_Unique_ID(S25_READ_U_ID * pUI)
{
	uint8_t cmd = CMD_RUID ;

	SPI_CS_N_Write(0) ;

	FSPI_write(&cmd, sizeof(cmd)) ;

	FSPI_dummy_read(4) ;

	FSPI_read(pUI->id, 8) ;

	SPI_CS_N_Write(1) ;
}

void S25_Read(uint32_t addr, void * v, size_t dim)
{
	uint8_t cmd[1 + 3] = { CMD_READ } ;
    
    addr = gira_indirizzo(addr) ;

	memcpy(cmd + 1, &addr, 3) ;

	SPI_CS_N_Write(0) ;

	FSPI_write(cmd, sizeof(cmd)) ;
	FSPI_read(v, dim) ;

	SPI_CS_N_Write(1) ;
}

void S25_Write_Enable(void)
{
	uint8_t cmd = CMD_WREN ;

	SPI_CS_N_Write(0) ;

	FSPI_write(&cmd, 1) ;

	SPI_CS_N_Write(1) ;
}

void S25_Write_Disable(void)
{
	uint8_t cmd = CMD_WRDI ;

	SPI_CS_N_Write(0) ;

	FSPI_write(&cmd, 1) ;

	SPI_CS_N_Write(1) ;
}

void S25_Clear_Status_Register(void)
{
	uint8_t cmd = CMD_CLSR ;

	SPI_CS_N_Write(0) ;

	FSPI_write(&cmd, sizeof(cmd)) ;

	SPI_CS_N_Write(1) ;
}

void S25_Write(uint32_t addr, const void * v, size_t dim)
{
	uint8_t cmd[1 + 3] = { CMD_PP } ;
    
    addr = gira_indirizzo(addr) ;

	memcpy(cmd + 1, &addr, 3) ;

	SPI_CS_N_Write(0) ;

	FSPI_write(cmd, sizeof(cmd)) ;

	const size_t DIM = MIN(dim, DIM_PAGE_BUFFER) ;
	FSPI_write(v, DIM) ;

	SPI_CS_N_Write(1) ;
}

uint8_t S25_Read_Status_Register_1(void)
{
	uint8_t cmd = CMD_RDSR1 ;

	SPI_CS_N_Write(0) ;

	FSPI_write(&cmd, sizeof(cmd)) ;

	FSPI_read(&cmd, 1) ;

	SPI_CS_N_Write(1) ;

	return cmd ;
}

uint8_t S25_Read_Status_Register_2(void)
{
	uint8_t cmd = CMD_RDSR2 ;

	SPI_CS_N_Write(0) ;

	FSPI_write(&cmd, sizeof(cmd)) ;

	FSPI_read(&cmd, 1) ;

	SPI_CS_N_Write(1) ;

	return cmd ;
}

void S25_Sector_Erase(uint32_t addr)
{
	uint8_t cmd[1 + 3] = { CMD_SE } ;
    
    addr = gira_indirizzo(addr) ;

	memcpy(cmd + 1, &addr, 3) ;

	SPI_CS_N_Write(0) ;

	FSPI_write(cmd, sizeof(cmd)) ;

	SPI_CS_N_Write(1) ;
}

void S25_Half_Block_Erase(uint32_t addr)
{
	uint8_t cmd[1 + 3] = { CMD_HBE } ;
    
    addr = gira_indirizzo(addr) ;

	memcpy(cmd + 1, &addr, 3) ;

	SPI_CS_N_Write(0) ;

	FSPI_write(cmd, sizeof(cmd)) ;

	SPI_CS_N_Write(1) ;
}

void S25_Block_Erase(uint32_t addr)
{
	uint8_t cmd[1 + 3] = { CMD_BE } ;
    
    addr = gira_indirizzo(addr) ;

	memcpy(cmd + 1, &addr, 3) ;

	SPI_CS_N_Write(0) ;

	FSPI_write(cmd, sizeof(cmd)) ;

	SPI_CS_N_Write(1) ;
}

void S25_Chip_Erase(void)
{
	uint8_t cmd = CMD_CE ;

	SPI_CS_N_Write(0) ;

	FSPI_write(&cmd, 1) ;

	SPI_CS_N_Write(1) ;
}

void S25_Program_or_Erase_Suspend(void)
{
	uint8_t cmd = CMD_PES ;

	SPI_CS_N_Write(0) ;

	FSPI_write(&cmd, 1) ;

	SPI_CS_N_Write(1) ;
}

uint8_t S25_Read_Configuration_Register_1(void)
{
	uint8_t cmd = CMD_RDCR1 ;

	SPI_CS_N_Write(0) ;

	FSPI_write(&cmd, sizeof(cmd)) ;

	FSPI_read(&cmd, 1) ;

	SPI_CS_N_Write(1) ;

	return cmd ;
}

uint8_t S25_Read_Configuration_Register_2(void)
{
	uint8_t cmd = CMD_RDCR2 ;

	SPI_CS_N_Write(0) ;

	FSPI_write(&cmd, sizeof(cmd)) ;

	FSPI_read(&cmd, 1) ;

	SPI_CS_N_Write(1) ;

	return cmd ;
}

uint8_t S25_Read_Configuration_Register_3(void)
{
	uint8_t cmd = CMD_RDCR3 ;

	SPI_CS_N_Write(0) ;

	FSPI_write(&cmd, sizeof(cmd)) ;

	FSPI_read(&cmd, 1) ;

	SPI_CS_N_Write(1) ;

	return cmd ;
}

void S25_Security_Region_Erase(uint32_t addr)
{
	uint8_t cmd[1 + 3] = { CMD_SECRE } ;
    
    addr = gira_indirizzo(addr) ;

	memcpy(cmd + 1, &addr, 3) ;

	SPI_CS_N_Write(0) ;

	FSPI_write(cmd, sizeof(cmd)) ;

	SPI_CS_N_Write(1) ;
}

void S25_Security_Region_Program(uint32_t addr, const void * v, size_t dim)
{
	uint8_t cmd[1 + 3] = { CMD_SECRP } ;
    
    addr = gira_indirizzo(addr) ;

	memcpy(cmd + 1, &addr, 3) ;

	SPI_CS_N_Write(0) ;

	FSPI_write(cmd, sizeof(cmd)) ;

	const size_t DIM = MIN(dim, DIM_PAGE_BUFFER) ;
	FSPI_write(v, DIM) ;

	SPI_CS_N_Write(1) ;
}

void S25_Security_Regions_Read(uint32_t addr, void * v, size_t dim)
{
	uint8_t cmd[1 + 3] = { CMD_SECRR } ;
    
    addr = gira_indirizzo(addr) ;

	memcpy(cmd + 1, &addr, 3) ;

	SPI_CS_N_Write(0) ;

	FSPI_write(cmd, sizeof(cmd)) ;

	FSPI_dummy_read(1) ;

	FSPI_read(v, dim) ;

	SPI_CS_N_Write(1) ;
}

void S25_Software_Reset(void)
{
	// Abilito
	uint8_t cmd = CMD_RSTEN ;

	SPI_CS_N_Write(0) ;

	FSPI_write(&cmd, sizeof(cmd)) ;

	SPI_CS_N_Write(1) ;

	// Resetto
	cmd = CMD_RST ;

	SPI_CS_N_Write(0) ;

	FSPI_write(&cmd, sizeof(cmd)) ;

	SPI_CS_N_Write(1) ;
}

#else

void S25_Read_Identification(S25_READ_ID * x) {}
void S25_Read_Unique_ID(S25_READ_U_ID * x) {}

void S25_Read(uint32_t addr, void * v, size_t d) {}

void S25_Write_Enable(void) {}
void S25_Write_Disable(void) {}

void S25_Write(uint32_t addr, const void * v, size_t d) {}

void S25_Sector_Erase(uint32_t addr) {}

uint8_t S25_Read_Status_Register_1(void) { return 0 ; }
uint8_t S25_Read_Status_Register_2(void) { return 0 ; }

uint8_t S25_Read_Configuration_Register_1(void) { return 0 ; }
uint8_t S25_Read_Configuration_Register_2(void) { return 0 ; }
uint8_t S25_Read_Configuration_Register_3(void) { return 0 ; }

void S25_Security_Region_Erase(uint32_t addr) {}
void S25_Security_Region_Program(uint32_t addr, const void * v, size_t dim) {}
void S25_Security_Regions_Read(uint32_t addr, void * v, size_t dim) {}


#endif
