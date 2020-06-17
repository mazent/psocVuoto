#include "fspi.h"

#ifdef CY_PINS_SPI_CS_N_H

void FSPI_write(const void * v, size_t dim)
{
	const uint8_t * tx = v ;

	for (size_t i=0 ; i<dim ; ++i) {
		uint8_t msk = 1 << 7 ;
		uint8_t x = tx[i] ;
		for (size_t bit = 0 ; bit < 8 ; ++bit) {
			uint8_t y = x & msk ? 1 : 0 ;
			SPI_MOSI_Write(y) ;
			SPI_SCK_Write(1) ;
			SPI_SCK_Write(0) ;
			msk >>= 1 ;
		}
	}
}

void FSPI_read(void * v, size_t dim)
{
	uint8_t * rx = v ;

	for (size_t i=0 ; i<dim ; ++i) {
		uint8_t x = 0 ;
		size_t bit = 7 ;
		for (size_t j=0 ; j<8 ; --bit, ++j) {
			SPI_SCK_Write(1) ;
			uint8_t y = SPI_MISO_Read() ;
			SPI_SCK_Write(0) ;
			x |= y << bit ;
		}
		rx[i] = x ;
	}
}

void FSPI_dummy_read(size_t dim)
{
	for (size_t i=0 ; i<dim ; ++i) {
		uint8_t x = 0 ;
		size_t bit = 7 ;
		for (size_t j=0 ; j<8 ; --bit, ++j) {
			SPI_SCK_Write(1) ;
			uint8_t y = SPI_MISO_Read() ;
			SPI_SCK_Write(0) ;
			x |= y << bit ;
		}
	}
}

#else

void FSPI_write(const void * tx, size_t dim) {}
void FSPI_read(void * rx, size_t dim) {}
void FSPI_dummy_read(size_t dim) {}

#endif
