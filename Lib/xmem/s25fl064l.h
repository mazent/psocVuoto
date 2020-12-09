#ifndef S25FL064L_H_
#define S25FL064L_H_

#include "includimi.h"

#define S25FL064L_BLOCK_SIZE		(64 * 1024)
#define S25FL064L_BLOCK_COUNT		128
#define S25FL064L_HALF_BLOCK_SIZE	(32 * 1024)
#define S25FL064L_HALF_BLOCK_COUNT	256
#define S25FL064L_SECTOR_SIZE		( 4 * 1024)
#define S25FL064L_SECTOR_COUNT		2048

#define S25FL064L_SECURITY_REGION_SIZE		256
#define S25FL064L_SECURITY_REGION_COUNT		4

#define S25FL064L_WRITE_PAGE_BUFFER_SIZE	256

// tSE max
#define S25FL064L_SECTOR_ERASE_TIME_MS		320

// tHBE max
#define S25FL064L_HALF_BLOCK_ERASE_TIME_MS	600

// tBE max
#define S25FL064L_BLOCK_ERASE_TIME_MS 		1150

// tCE max
#define S25FL064L_CHIP_ERASE_TIME_S 		150

// tPP max = 1350 us
#define S25FL064L_PAGE_PROGRAMMING_MS		2



typedef struct {
	uint8_t manuf ;
	uint16_t device ;
} S25_READ_ID ;

void S25_Read_Identification(S25_READ_ID *) ;


#define S25_DIM_UID		8

typedef struct {
	uint8_t id[S25_DIM_UID] ;
} S25_READ_U_ID ;

void S25_Read_Unique_ID(S25_READ_U_ID *) ;

void S25_Read(uint32_t addr, void *, size_t) ;

void S25_Write_Enable(void) ;
void S25_Write_Disable(void) ;
void S25_Clear_Status_Register(void) ;

void S25_Write(uint32_t addr, const void *, size_t) ;

void S25_Sector_Erase(uint32_t addr) ;
void S25_Half_Block_Erase(uint32_t addr) ;
void S25_Block_Erase(uint32_t addr) ;
void S25_Chip_Erase(void) ;
void S25_Program_or_Erase_Suspend(void) ;

#define SR1_SRP0	  (1 << 7)
#define SR1_SEC       (1 << 6)
#define SR1_TBPROT    (1 << 5)
#define SR1_BP2       (1 << 4)
#define SR1_BP1       (1 << 3)
#define SR1_BP0       (1 << 2)
#define SR1_WEL       (1 << 1)
#define SR1_WIP       (1 << 0)

uint8_t S25_Read_Status_Register_1(void) ;

#define SR2_E_ERR     (1 << 6)
#define SR2_P_ERR     (1 << 5)
#define SR2_ES        (1 << 1)
#define SR2_PS        (1 << 0)

uint8_t S25_Read_Status_Register_2(void) ;

#define CR1_SUS_D	  (1 << 7)
#define CR1_CMP       (1 << 6)
#define CR1_LB3       (1 << 5)
#define CR1_LB2       (1 << 4)
#define CR1_LB1       (1 << 3)
#define CR1_LB0       (1 << 2)
#define CR1_QUAD      (1 << 1)
#define CR1_SRP1_D    (1 << 0)

uint8_t S25_Read_Configuration_Register_1(void) ;

#define CR2_IO3R	  (1 << 7)
#define CR2_OI_MSK	  (3 << 5)
#define CR2_QPI       (1 << 3)
#define CR2_WPS       (1 << 2)
#define CR2_ADP       (1 << 1)

uint8_t S25_Read_Configuration_Register_2(void) ;

#define CR3_WL_MSK		(3 << 5)
#define CR3_WE       	(1 << 4)
#define CR3_RL_MSK		0x0F

uint8_t S25_Read_Configuration_Register_3(void) ;

void S25_Security_Region_Erase(uint32_t addr) ;
void S25_Security_Region_Program(uint32_t addr, const void * v, size_t dim) ;
void S25_Security_Regions_Read(uint32_t addr, void * v, size_t dim) ;

void S25_Software_Reset(void) ;

#endif
