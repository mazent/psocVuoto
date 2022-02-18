#include "at25sf041.h"
#include "spi/fspi.h"

#ifdef CY_PINS_SPI_CS_N_H

#define BIT_X_BYTE      8

#define VAL_ERASED      0xFF

// https://community.cypress.com/docs/DOC-9250
// https://community.cypress.com/docs/DOC-14313

#define CMD_RDID        0x9F
//#define CMD_RUID      0x4B
#define CMD_READ        0x03
#define CMD_WREN        0x06
//#define CMD_WRDI		0x04
#define CMD_PP          0x02
#define CMD_RDSR1       0x05
#define CMD_RDSR2       0x35
#define CMD_SE          0x20
//#define CMD_HBE       0x52
#define CMD_BE          0xD8
//#define CMD_CE            0x60
//#define CMD_SECRE         0x44
//#define CMD_SECRP         0x42
//#define CMD_SECRR         0x48
//#define CMD_RDCR1         0x35
//#define CMD_RDCR2         0x15
//#define CMD_RDCR3         0x33
//#define CMD_CLSR      0x30
//#define CMD_PES       0x75
//#define CMD_RSTEN         0x66
//#define CMD_RST       0x99

#define CMD_NTR_DPDOWN  0xB9
#define CMD_XIT_DPDOWN  0xAB

// servono i bit A23..A0 in big-endian

static uint32_t gira_indirizzo(uint32_t addr)
{
    addr = __REV(addr) ;
    addr >>= BIT_X_BYTE ;

    return addr ;
}

void AT25_accendi(void)
{
    uint8_t cmd = CMD_XIT_DPDOWN ;

    SPI_CS_N_Write(0) ;

    FSPI_write(&cmd, 1) ;

    SPI_CS_N_Write(1) ;
}

void AT25_spegni(void)
{
    uint8_t cmd = CMD_NTR_DPDOWN ;

    SPI_CS_N_Write(0) ;

    FSPI_write(&cmd, 1) ;

    SPI_CS_N_Write(1) ;
}

void AT25_Read_Identification(AT25_READ_ID * pRI)
{
    uint8_t cmd = CMD_RDID ;
    uint8_t rsp[3] ;

    SPI_CS_N_Write(0) ;

    FSPI_write( &cmd, sizeof(cmd) ) ;
    FSPI_read( rsp, sizeof(rsp) ) ;

    SPI_CS_N_Write(1) ;

    pRI->manuf = rsp[0] ;
    memcpy(&pRI->device, rsp + 1, 2) ;
}

void AT25_Read(
    uint32_t addr,
    void * v,
    size_t dim)
{
    uint8_t cmd[1 + 3] = {
        CMD_READ
    } ;

    addr = gira_indirizzo(addr) ;

    memcpy(cmd + 1, &addr, 3) ;

    SPI_CS_N_Write(0) ;

    FSPI_write( cmd, sizeof(cmd) ) ;
    FSPI_read(v, dim) ;

    SPI_CS_N_Write(1) ;
}

static size_t dim_erased ;

static bool is_erased(uint8_t x)
{
    bool esito = VAL_ERASED == x ;

    if ( esito ) {
        dim_erased += 1 ;
    }

    return esito ;
}

bool AT25_is_erased(
    uint32_t addr,
    size_t dim)
{
    uint8_t cmd[1 + 3] = {
        CMD_READ
    } ;

    dim_erased = 0 ;
    addr = gira_indirizzo(addr) ;

    memcpy(cmd + 1, &addr, 3) ;

    SPI_CS_N_Write(0) ;

    FSPI_write( cmd, sizeof(cmd) ) ;
    FSPI_read_cb(is_erased) ;

    SPI_CS_N_Write(1) ;

    return dim_erased == dim ;
}

void AT25_Write_Enable(void)
{
    uint8_t cmd = CMD_WREN ;

    SPI_CS_N_Write(0) ;

    FSPI_write(&cmd, 1) ;

    SPI_CS_N_Write(1) ;
}

static size_t max_dim(
    uint32_t addr,
    size_t dim)
{
    // Devo rimanere all'interno della pagina
    uint32_t addr_max = addr + AT25SF041_WRITE_PAGE_BUFFER_SIZE ;

    // tolgo i bit meno significativi -> inizio pag successiva
    addr_max &= NOT(AT25SF041_WRITE_PAGE_BUFFER_SIZE - 1) ;

    addr_max = MIN(addr_max, addr + dim) ;
    dim = addr_max - addr ;

    return dim ;
}

size_t AT25_Write(
    uint32_t addr,
    const void * v,
    size_t dim)
{
    uint8_t cmd[1 + 3] = {
        CMD_PP
    } ;

    dim = max_dim(addr, dim) ;

    addr = gira_indirizzo(addr) ;

    memcpy(cmd + 1, &addr, 3) ;

    SPI_CS_N_Write(0) ;

    FSPI_write( cmd, sizeof(cmd) ) ;

    const size_t DIM = MIN(dim, AT25SF041_WRITE_PAGE_BUFFER_SIZE) ;
    FSPI_write(v, DIM) ;

    SPI_CS_N_Write(1) ;

    return DIM ;
}

static uint8_t AT25_Read_Status_Register_x(uint8_t cmd)
{
    SPI_CS_N_Write(0) ;

    FSPI_write( &cmd, sizeof(cmd) ) ;

    FSPI_read(&cmd, 1) ;

    SPI_CS_N_Write(1) ;

    return cmd ;
}

uint8_t AT25_Read_Status_Register_1(void)
{
    return AT25_Read_Status_Register_x(CMD_RDSR1) ;
}

uint8_t AT25_Read_Status_Register_2(void)
{
    return AT25_Read_Status_Register_x(CMD_RDSR2) ;
}

void AT25_Sector_Erase(uint32_t addr)
{
    uint8_t cmd[1 + 3] = {
        CMD_SE
    } ;

    addr = gira_indirizzo(addr) ;

    memcpy(cmd + 1, &addr, 3) ;

    SPI_CS_N_Write(0) ;

    FSPI_write( cmd, sizeof(cmd) ) ;

    SPI_CS_N_Write(1) ;
}

void AT25_Block_Erase(uint32_t addr)
{
    uint8_t cmd[1 + 3] = {
        CMD_BE
    } ;

    addr = gira_indirizzo(addr) ;

    memcpy(cmd + 1, &addr, 3) ;

    SPI_CS_N_Write(0) ;

    FSPI_write( cmd, sizeof(cmd) ) ;

    SPI_CS_N_Write(1) ;
}

//void AT25_Chip_Erase(void)
//{
//	uint8_t cmd = CMD_CE ;
//
//	SPI_CS_N_Write(0) ;
//
//	FSPI_write(&cmd, 1) ;
//
//	SPI_CS_N_Write(1) ;
//}

#endif
