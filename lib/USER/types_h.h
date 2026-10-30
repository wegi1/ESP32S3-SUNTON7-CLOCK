/*
 * types_h.h
 *
 *  Created on: 14 maj 2023
 *      Author: BOLO
 */

#ifndef TYPES_H_H_
#define TYPES_H_H_

#include <inttypes.h>

#include "stdbool.h"

/*
typedef uint8_t       bool;
typedef char          ascii;
typedef uint8_t       u8;
typedef int8_t        s8;
typedef uint16_t      u16;
typedef int16_t       s16;
typedef uint32_t      u32;
typedef int32_t       s32;
*/
#define __IO volatile
#define __I volatile
typedef  char                ascii;

typedef          int32_t     s32;
typedef  const   int32_t     sc32;    /*!< Read Only */
typedef  __IO    int32_t     vs32;
typedef         uint32_t     u32;
typedef  __I     int32_t     vsc32;   /*!< Read Only */
typedef  const  uint32_t     uc32;    /*!< Read Only */
typedef  __IO   uint32_t     vu32;
typedef  __I    uint32_t     vuc32;   /*!< Read Only */


typedef          int16_t     s16;
typedef  const   int16_t     sc16;    /*!< Read Only */
typedef  __IO    int16_t     vs16;
typedef  __I     int16_t     vsc16;   /*!< Read Only */
typedef         uint16_t     u16;
typedef  const  uint16_t     uc16;    /*!< Read Only */
typedef  __IO   uint16_t     vu16;
typedef  __I    uint16_t     vuc16;   /*!< Read Only */


typedef           int8_t     s8;
typedef  const    int8_t     sc8;     /*!< Read Only */
typedef  __IO     int8_t     vs8;
typedef  __I      int8_t     vsc8;    /*!< Read Only */
typedef          uint8_t     u8;
typedef  const   uint8_t     uc8;     /*!< Read Only */
typedef  __IO    uint8_t     vu8;
typedef  __I     uint8_t     vuc8;    /*!< Read Only */

//#define bool    _Bool

// #define LOW   0
// #define HIGH  1
#define SPI_I2S_FLAG_RXNE               ((uint16_t)0x0001)
#define SPI_I2S_FLAG_TXE                ((uint16_t)0x0002)

#endif /* TYPES_H_H_ */
