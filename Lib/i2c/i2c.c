#define STAMPA_DBG
#include "soc/utili.h"
#include "i2c.h"

#ifdef CY_PINS_SCL_H

#ifdef SPI_ATOMIC
#	define ATOMIC_ENTER		\
	uint32 irqmsk = CyDisableInts() ;
#	define ATOMIC_LEAVE		\
	CyEnableInts(irqmsk) ;
#else
#	define ATOMIC_ENTER
#	define ATOMIC_LEAVE
#endif


// cfr https://en.wikipedia.org/wiki/I%C2%B2C
// https://en.wikipedia.org/wiki/Bit_banging

static inline void clear_SDA(void)
{
	SDA_Write(0) ;
}

static inline void set_SDA(void)
{
	SDA_Write(1) ;
}

static inline bool read_SDA(void)
{
	return 1 == SDA_Read() ;
}

static inline void clear_SCL(void)
{
	SCL_Write(0) ;
}

static inline void set_SCL(void)
{
	SCL_Write(1) ;
}

static inline void I2C_delay(void)
{
	CyDelayUs(1) ;
}

static void i2c_start_cond(void)
{
    // SCL is high, set SDA from 1 to 0.
    clear_SDA() ;
    I2C_delay() ;
    clear_SCL() ;
}

static void i2c_stop_cond(void)
{
    // set SDA to 0
    clear_SDA() ;
    I2C_delay() ;

    set_SCL() ;

    // Stop bit setup time, minimum 4us
    I2C_delay() ;

    // SCL is high, set SDA from 0 to 1
    set_SDA() ;
    I2C_delay() ;
}

// Write a bit to I2C bus
static void i2c_write_bit(bool bit)
{
    if (bit) {
        set_SDA() ;
    }
    else {
        clear_SDA() ;
    }

    // SDA change propagation delay
    I2C_delay() ;

    // Set SCL high to indicate a new valid SDA value is available
    set_SCL() ;

    // Wait for SDA value to be read by slave, minimum of 4us for standard mode
    I2C_delay() ;

    // SCL is high, now data is valid

    // Clear the SCL to low in preparation for next change
    clear_SCL() ;
}

// Read a bit from I2C bus
static bool i2c_read_bit(void)
{
    bool bit ;

    // Let the slave drive data
    set_SDA() ;

    // Wait for SDA value to be written by slave, minimum of 4us for standard mode
    I2C_delay() ;

    // Set SCL high to indicate a new valid SDA value is available
    set_SCL() ;

    // Wait for SDA value to be written by slave, minimum of 4us for standard mode
    I2C_delay() ;

    // SCL is high, read out bit
    bit = read_SDA() ;

    // Set SCL low in preparation for next operation
    clear_SCL() ;

    return bit ;
}

// Write a byte to I2C bus. Return 0 if ack by the slave.
static bool i2c_write_byte(bool send_start,
                    bool send_stop,
                    uint8_t byte)
{
    unsigned bit ;
    bool nack ;

    if (send_start) {
        i2c_start_cond() ;
    }

    for (bit = 0 ; bit < 8 ; ++bit) {
        i2c_write_bit( (byte & 0x80) != 0 ) ;
        byte <<= 1 ;
    }

    nack = i2c_read_bit() ;

    if (send_stop) {
        i2c_stop_cond() ;
    }

    return nack ;
}

// Read a byte from I2C bus
static uint8_t i2c_read_byte(bool nack, bool send_stop)
{
    uint8_t byte = 0 ;
    uint8_t bit ;

    for (bit = 0 ; bit < 8 ; ++bit) {
        byte = (byte << 1) | i2c_read_bit() ;
    }

    i2c_write_bit(nack) ;

    if (send_stop) {
        i2c_stop_cond() ;
    }

    return byte ;
}

bool I2C_write(uint8_t ind, const uint8_t * v, int dim)
{
	bool esito = false ;

	ind &= 0xFE ;

	ATOMIC_ENTER

	do {
		if (i2c_write_byte(true,
		                    false,
		                    ind)) {
			DBG_ERR ;
			break ;
		}

		int i = 0 ;
		for (; i<dim ; ++i) {
			if (i2c_write_byte(false,
			                    false,
			                    v[i])) {
				DBG_ERR ;
				break ;
			}
		}

		esito = i == dim ;
	} while (false) ;

	i2c_stop_cond() ;

	ATOMIC_LEAVE

    return esito ;
}

bool I2C_read(uint8_t ind, uint8_t * v, int dim)
{
	bool esito = false ;

	ind |= 0x01 ;

	ATOMIC_ENTER

	do {
		if (i2c_write_byte(true,
		                    false,
		                    ind)) {
			break ;
		}

		for (int i = 0 ; i<dim-1 ; ++i) {
			v[i] = i2c_read_byte(false, false) ;
		}

		// l'ultimo vuole nack
		v[dim-1] = i2c_read_byte(true, false) ;

		esito = true ;
	} while (false) ;

	i2c_stop_cond() ;

	ATOMIC_LEAVE

    return esito ;
}

bool I2C_write_reg(uint8_t ind, uint8_t reg, const uint8_t * v, int dim)
{
	bool esito = false ;

	ind &= 0xFE ;

	ATOMIC_ENTER

	do {
		if (i2c_write_byte(true,
		                    false,
		                    ind)) {
			DBG_ERR ;
			break ;
		}

		if (i2c_write_byte(false,
		                    false,
		                    reg)) {
			DBG_ERR ;
			break ;
		}

		int i = 0 ;
		for (; i<dim ; ++i) {
			if (i2c_write_byte(false,
			                    false,
			                    v[i])) {
				DBG_ERR ;
				break ;
			}
		}

		esito = i == dim ;
	} while (false) ;

	i2c_stop_cond() ;

	ATOMIC_LEAVE

    return esito ;
}

//void I2C_iniz(void)
//{
//	i2c_start_cond() ;
//
//	CyDelayUs(1) ;
//
//	i2c_stop_cond() ;
//}

#else

bool I2C_write(uint8_t ind, const uint8_t * v, int dim)
{
    DBG_ERR ;

    UNUSED(ind) ;
    UNUSED(v) ;
    UNUSED(dim) ;

    return false ;
}

bool I2C_read(uint8_t ind, uint8_t * v, int dim)
{
    DBG_ERR ;

    UNUSED(ind) ;
    UNUSED(v) ;
    UNUSED(dim) ;

    return false ;
}

bool I2C_write_reg(uint8_t ind, uint8_t reg, const uint8_t * v, int dim)
{
    DBG_ERR ;

    UNUSED(ind) ;
    UNUSED(reg) ;
    UNUSED(v) ;
    UNUSED(dim) ;

    return false ;
}

#endif
