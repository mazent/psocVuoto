#define STAMPA_DBG
#include "soc/utili.h"

#ifdef CY_PINS_SPI_CS_N_H

#ifdef SPI_ATOMIC
#   define ATOMIC_ENTER     \
    uint32 irqmsk = CyDisableInts() ;
#   define ATOMIC_LEAVE     \
    CyEnableInts(irqmsk) ;
#else
#   define ATOMIC_ENTER
#   define ATOMIC_LEAVE
#endif

#define BIT_ALTO        7
#define BIT_X_BYTE      8

#include "fspi.h"


void FSPI_iniz(void){}

void FSPI_sel(void)
{
    SPI_CS_N_Write(0) ;
    while ( 1 == SPI_CS_N_Read() ) {}
}

void FSPI_les(void)
{
    SPI_CS_N_Write(1) ;
    while ( 0 == SPI_CS_N_Read() ) {}
}

void FSPI_write(
    const void * v,
    size_t dim)
{
    const uint8_t * tx = v ;

    ATOMIC_ENTER

    for ( size_t i = 0 ; i < dim ; ++i ) {
        uint8_t msk = 1 << BIT_ALTO ;
        uint8_t x = tx[i] ;
        for ( size_t bit = 0 ; bit < BIT_X_BYTE ; ++bit ) {
            uint8_t y = x & msk ? 1 : 0 ;
            SPI_MOSI_Write(y) ;
            SPI_SCK_Write(1) ;
            SPI_SCK_Write(0) ;
            msk >>= 1 ;
        }
    }

    ATOMIC_LEAVE
}

void FSPI_read(
    void * v,
    size_t dim)
{
    uint8_t * rx = v ;

    ATOMIC_ENTER

    for ( size_t i = 0 ; i < dim ; ++i ) {
        uint8_t x = 0 ;
        size_t bit = BIT_ALTO ;
        for ( size_t j = 0 ; j < BIT_X_BYTE ; --bit, ++j ) {
            SPI_SCK_Write(1) ;
            uint8_t y = SPI_MISO_Read() ;
            SPI_SCK_Write(0) ;
            x |= y << bit ;
        }
        rx[i] = x ;
    }

    ATOMIC_LEAVE
}

void FSPI_read_cb(PF_FSPI_READ cb)
{
    bool leggi = true ;
    ATOMIC_ENTER

    while ( leggi ) {
        uint8_t x = 0 ;
        size_t bit = BIT_ALTO ;
        for ( size_t j = 0 ; j < BIT_X_BYTE ; --bit, ++j ) {
            SPI_SCK_Write(1) ;
            uint8_t y = SPI_MISO_Read() ;
            SPI_SCK_Write(0) ;
            x |= y << bit ;
        }

        leggi = cb(x) ;
    }

    ATOMIC_LEAVE
}

void FSPI_dummy_read(size_t dim)
{
    ATOMIC_ENTER

    for ( size_t i = 0 ; i < dim ; ++i ) {
        uint8_t x = 0 ;
        size_t bit = BIT_ALTO ;
        for ( size_t j = 0 ; j < BIT_X_BYTE ; --bit, ++j ) {
            SPI_SCK_Write(1) ;
            uint8_t y = SPI_MISO_Read() ;
            SPI_SCK_Write(0) ;
            x |= y << bit ;
        }
    }

    ATOMIC_LEAVE
}

//#else
//
//
//
//void FSPI_write(const void * tx, size_t dim)
//{
//}
//
//void FSPI_read(void * rx, size_t dim)
//{
//}
//
//void FSPI_dummy_read(size_t dim)
//{
//}

#endif
