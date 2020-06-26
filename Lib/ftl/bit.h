#ifndef BIT_H_
#define BIT_H_

static inline
void BIT_zeri(uint8_t * v, size_t numbit)
{
    const size_t NUM_BYTE = (numbit + 7) / 8 ;

    memset(v, 0, NUM_BYTE) ;
}

static inline
void BIT_uni(uint8_t * v, size_t numbit)
{
    const size_t NUM_BYTE = (numbit + 7) / 8 ;

    if (numbit % 8) {
        if (NUM_BYTE > 1)
        	memset(v, 0xFF, NUM_BYTE - 1) ;
        v[NUM_BYTE-1] = (1 << (numbit % 8)) - 1  ;
    }
    else
    	memset(v, 0xFF, NUM_BYTE) ;
}

static inline
void BIT_uno(uint8_t * pB, size_t bit)
{
    size_t byte = bit >> 3 ;
    bit &= 7 ;

    pB[byte] |= (1 << bit) ;
}

static inline
void BIT_zero(uint8_t * pB, size_t bit)
{
    size_t byte = bit >> 3 ;
    bit &= 7 ;

    pB[byte] &= ~(1 << bit) ;
}

// http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan

static inline
size_t BIT_quanti_uni(uint8_t * pB, size_t numbit)
{
    size_t conta = 0 ;
    size_t num_byte = (numbit + 7) / 8 ;

    while ( num_byte >= sizeof(uint32_t) ) {
        uint32_t v ;

        memcpy( &v, pB, sizeof(uint32_t) ) ;
        pB += sizeof(uint32_t) ;
        num_byte -= sizeof(uint32_t) ;

        for ( ; v ; conta++) {
            v &= v - 1 ;
        }
    }

    if ( num_byte >= sizeof(uint16_t) ) {
        uint16_t v ;

        memcpy( &v, pB, sizeof(uint16_t) ) ;
        pB += sizeof(uint16_t) ;
        num_byte -= sizeof(uint16_t) ;

        for ( ; v ; conta++) {
            v &= v - 1 ;
        }
    }

    if (num_byte) {
        uint8_t v = *pB ;

        for ( ; v ; conta++) {
            v &= v - 1 ;
        }
    }

    return conta ;
}

static inline
size_t BIT_primo_uno(uint8_t * pB, size_t numbit)
{
    size_t primo = numbit ;
    const size_t NUM_BYTE = (numbit + 7) / 8 ;
    size_t byte = 0 ;

    for ( ; byte < NUM_BYTE ; byte++) {
        if (pB[byte]) {
            break ;
        }
    }

    if (byte < NUM_BYTE) {
        uint8_t bit = 1 ;

        for (int i = 0 ; i < 8 ; i++) {
            if (pB[byte] & bit) {
                primo = (byte << 3) + i ;
                if (primo >= numbit) {
                    // Metto il valore che segnala l'errore
                    primo = numbit ;
                }
                break ;
            }
            else {
                bit <<= 1 ;
            }
        }
    }

    return primo ;
}

#else
#   warning bit.h incluso
#endif /* BIT_H_ */
