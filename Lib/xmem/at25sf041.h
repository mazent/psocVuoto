#ifndef AT25SF041_H_
#define AT25SF041_H_

#include "utili/includimi.h"

#define AT25SF041_BLOCK_SIZE        (64 * 1024)
#define AT25SF041_BLOCK_COUNT       8
#define AT25SF041_HALF_BLOCK_SIZE   (32 * 1024)
#define AT25SF041_HALF_BLOCK_COUNT  16
#define AT25SF041_SECTOR_SIZE       (4 * 1024)
#define AT25SF041_SECTOR_COUNT      128

//#define AT25SF041_SECURITY_REGION_SIZE		256
//#define AT25SF041_SECURITY_REGION_COUNT		4

#define AT25SF041_WRITE_PAGE_BUFFER_SIZE    256

// tSE max
#define AT25SF041_SECTOR_ERASE_TIME_MS      300

// tHBE max
#define AT25SF041_HALF_BLOCK_ERASE_TIME_MS  1300

// tBE max
#define AT25SF041_BLOCK_ERASE_TIME_MS       2200

// tCE max
#define AT25SF041_CHIP_ERASE_TIME_S         10

// tPP max = 2.5 ms
#define AT25SF041_PAGE_PROGRAMMING_MS       3

#define AT25SF041_MANUF     0x1F
#define AT25SF041_DEVICE    0x0184

void AT25_accendi(void) ;
void AT25_spegni(void) ;

typedef struct {
    uint8_t manuf ;
    uint16_t device ;
} AT25_READ_ID ;

void AT25_Read_Identification(AT25_READ_ID *) ;

//#define AT25_DIM_UID		8
//
//typedef struct {
//	uint8_t id[AT25_DIM_UID] ;
//} AT25_READ_U_ID ;
//
//void AT25_Read_Unique_ID(AT25_READ_U_ID *) ;

void AT25_Read(uint32_t addr, void *, size_t) ;

bool AT25_is_erased(uint32_t, size_t) ;

void AT25_Write_Enable(void) ;
//void AT25_Write_Disable(void) ;
//void AT25_Clear_Status_Register(void) ;

// Max una pagina
size_t AT25_Write(uint32_t addr, const void *, size_t) ;

void AT25_Sector_Erase(uint32_t addr) ;
//void AT25_Half_Block_Erase(uint32_t addr) ;
void AT25_Block_Erase(uint32_t addr) ;
void AT25_Chip_Erase(void) ;
//void AT25_Program_or_Erase_Suspend(void) ;

#define SR1_SRP0      (1 << 7)
#define SR1_SEC       (1 << 6)
#define SR1_TBPROT    (1 << 5)
#define SR1_BP2       (1 << 4)
#define SR1_BP1       (1 << 3)
#define SR1_BP0       (1 << 2)
#define SR1_WEL       (1 << 1)
#define SR1_WIP       (1 << 0)

uint8_t AT25_Read_Status_Register_1(void) ;

#define SR2_CMP     (1 << 6)
#define SR2_LB3     (1 << 5)
#define SR2_LB2     (1 << 4)
#define SR2_LB1     (1 << 3)
#define SR2_QE      (1 << 1)
#define SR2_SRP1    (1 << 0)

uint8_t AT25_Read_Status_Register_2(void) ;

//#define CR1_SUS_D	  (1 << 7)
//#define CR1_CMP       (1 << 6)
//#define CR1_LB3       (1 << 5)
//#define CR1_LB2       (1 << 4)
//#define CR1_LB1       (1 << 3)
//#define CR1_LB0       (1 << 2)
//#define CR1_QUAD      (1 << 1)
//#define CR1_SRP1_D    (1 << 0)
//
//uint8_t AT25_Read_Configuration_Register_1(void) ;
//
//#define CR2_IO3R	  (1 << 7)
//#define CR2_OI_MSK	  (3 << 5)
//#define CR2_QPI       (1 << 3)
//#define CR2_WPS       (1 << 2)
//#define CR2_ADP       (1 << 1)
//
//uint8_t AT25_Read_Configuration_Register_2(void) ;
//
//#define CR3_WL_MSK		(3 << 5)
//#define CR3_WE        (1 << 4)
//#define CR3_RL_MSK		0x0F
//
//uint8_t AT25_Read_Configuration_Register_3(void) ;
//
//void AT25_Security_Region_Erase(uint32_t addr) ;
//void AT25_Security_Region_Program(uint32_t addr, const void * v, size_t dim) ;
//void AT25_Security_Regions_Read(uint32_t addr, void * v, size_t dim) ;
//
//void AT25_Software_Reset(void) ;

#endif
