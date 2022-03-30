#ifndef COD_PCOM_CFG_TEMPLATE_H_
#define COD_PCOM_CFG_TEMPLATE_H_

// Un messaggio di dimensione ...
#define PCOM_DIM_MIN    	2
#define PCOM_DIM_MAX        102

// ... viene incapsulato dentro una trama ...
#define PCOM_INIZIO_MSG     0xC5
#define PCOM_FINE_MSG       0xC2
#define PCOM_FUGA        	0xCF

// ... protetta da crc
#define PCOM_CRC_INI        0xCCCC


#else
#   warning pcom_cfg_template.h incluso
#endif
