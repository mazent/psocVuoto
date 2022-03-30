#ifndef I2C_H_
#define I2C_H_

#include <stdbool.h>
#include <stdint.h>

bool I2C_write(uint8_t ind, const uint8_t * v, int dim) ;
bool I2C_read(uint8_t ind, uint8_t * v, int dim) ;

bool I2C_write_reg(uint8_t ind, uint8_t reg, const uint8_t * v, int dim) ;

#endif
