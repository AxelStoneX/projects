#ifndef __EM_COMMON_DEFS_H__
#define __EM_COMMON_DEFS_H__

#include <errno.h>  // Standard functions return codes
// ----- Common Constants --------------------------------------
#define L_ON    1   // Same as logical TRUE
#define L_OFF   0   // Same as logical FALSE

#define ID_UNDEF -77  // Indefinite state indicator for any type int ID

#define RET_OK       0    /* Typical expected result return code */
#define RET_FAIL    -1    /* Typical return error code  */
#define RET_UF    0xFF    /* Unsigned return error code */
#define BAD_IX      -2    /* Incorrect index parameter  */
#define RET_WARN    -3    /* Warning return code        */
#define RET_FATAL   -4    /* Fatal error return code    */

#define EOK          0    /* Standart return code used in errno.h in QNX */

#define BYTES_LWORD_SH   2     // For shift conversion
#define BYTES_DBL_SHIFT  3     // For shift conversion
#define LWORD_TO_DBL_SH  1     // For shift conversion

#define LWORD_MAX   ( 0xFFFFFFFF - 1 )

// For err_code parameter in rt_halt() -> system error code not defined
#define NO_ERR_CODE -7777    // Coinside with <errno.h> is not probable

#include <stdint.h>

//  --> BWORD unused now
// typedef unsigned long BWORD;
typedef uint64_t QWORD;
typedef uint32_t LWORD;
typedef uint16_t SWORD;
typedef uint8_t  BYTE;

typedef  int64_t Q_INT;
typedef  int16_t S_INT;
typedef  unsigned short ushort_t;

#endif // __EM_COMMON_DEFS_H_
