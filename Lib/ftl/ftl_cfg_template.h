#ifndef FTL_CFG_
#define FTL_CFG_

#define STAMPA_DBG
#include "utili/includimi.h"

/*
 * La libreria FTL usa due blocchi di flash, suddivisi in
 * settori fisici, per salvare dei settori logici
 *
 * Dato che le funzioni che accedono alla flash sono esterne,
 * il numero del blocco potrebbe rappresentarne di piu'
 *
 * L'unico parametro modificabile senza perdere quanto gia'
 * scritto e' FTL_LSECT_NUM, sempre che non superi il numero
 * di settori fisici
 */

#define FTL_ERASED		0xFF

// Uso due blocchi di flash ...
#define FTL_BLK_1 		64
#define FTL_BLK_2 		65
// ... della stessa dimensione
#define FTL_BLK_DIM 	4096

// Deve poter rappresentare il blocco massimo
typedef uint8_t BLK_T ;

// Salvo un certo numero di settori logici ...
#define FTL_LSECT_NUM 	3
// ... della stessa dimensione
#define FTL_LSECT_DIM 	(257 - 2)

// Deve poter rappresentare il settore massimo
typedef uint8_t SECT_T ;

// Siccome ci sono due blocchi uguali, la distanza massima fra due
// copie dello stesso settore sara' al massimo il doppio del numero
// dei settori fisici piu' due (valori riservati)
typedef uint8_t LRU_T ;


#else
#	error ftl_cfg_template.h incluso
#endif
