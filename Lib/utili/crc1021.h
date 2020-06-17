#ifndef CRC1021_H_
#define CRC1021_H_

uint16_t CRC_1021_v(uint16_t crc, const void * v, int dim) ;
uint16_t CRC_1021_b(uint16_t crc, uint8_t val) ;

// compatibilita'
#define crc1021V(a, b, c)		CRC_1021_v(a, b, c)
#define crc1021(a, b)			CRC_1021_b(a, b)


#endif
