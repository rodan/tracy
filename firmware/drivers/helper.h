
// helper functions for embedded projects
//  to be used when either the math functions are missing from libc
//  or when space is at a premium and snprintf is overkill
//
//   author:          Petre Rodan <2b4eda@subdimension.ro>
//   available from:  https://github.com/rodan/reference_libs_msp430fr5x
//   license:         GNU GPLv3

#ifndef __HELPER_H__
#define __HELPER_H__

#define                  PI  3.14159265358f
#define            PI_FLOAT  3.14159265f
#define         PIBY2_FLOAT  1.5707963f
#define                PREC  5

#define  CONV_BASE_2_BUF_SZ  19
#define  CONV_BASE_8_BUF_SZ  12
#define CONV_BASE_10_BUF_SZ  12 // -2147483648 + ending 0 is 12 bytes long

typedef enum {
    PAD_NONE,
    PAD_ZEROES,
    PAD_SPACES,
} pad_type;

#ifdef __cplusplus
extern "C" {
#endif

// math functions

/** 
    @param
    @return
*/
float _atan2f(float y, float x);

/** calculate sine of an input
    @param input value (float)
    @return sin(input)
*/
float _sin(const float x);

/** calculate the cosine of an input
    @param input value (float)
    @return sin(input) (float)
*/
float _cos(const float x);

/** calulate the square root of an input
    @param input value (float)
    @return sqrt(input) (float)
*/
float _sqrt(const float number);

/** convert degrees to radians
    @param degree value to be converted (float)
    @return resulting radians (float)
*/
float radians(const float x);

/** return the square of a float
    @param input value to be squared (float)
    @return input^2 (float)
*/
float sq(const float x);


// string functions

/** return a binary string for an uint16_t integer
    @param temporary buffer used to build up the string. buf[CONV_BASE_2_BUF_SZ] needs to be pre-allocated
    @param uint16_t value
    @return pointer to the string
*/
char *_utob(char *buf, const uint16_t val);

/** return a hex string for an uint32_t integer
    @param temporary buffer used to build up the string. buf[CONV_BASE_8_BUF_SZ] needs to be pre-allocated
    @param uint32_t value
    @return pointer to the string
*/
char *_utoh(char *buf, const uint32_t val);

/** return a decimal string for an uint32_t integer
    @param temporary buffer used to build up the string. buf[CONV_BASE_10_BUF_SZ] needs to be pre-allocated
    @param uint32_t value
    @return pointer to the string
*/
char *_utoa(char *buf, const uint32_t val);

/** return a decimal string for an int32_t integer
    @param temporary buffer used to build up the string. buf[CONV_BASE_10_BUF_SZ] needs to be pre-allocated
    @param int32_t value
    @return pointer to the string
*/
char *_itoa(char *buf, const int32_t val);

/** return a string with prefixed padding
    @param pointer to originally allocated buffer
    @param pointer within the originally allocated buffer where the conversion is stored
    @param type of padding
    @param target total length of resulting string
    @return pointer to the resulting string. *converted_buf is returned on parameter error
*/
char *prepend_padding(char *buf, char *converted_buf, const pad_type padding_type, const uint8_t target_len);

/** convert a string to a 32bit unsigned integer
    @param pointer to string to be converted
    @param pointer to resulting uint32_t value [min .. max]
    @param how many chars to skip [0 .. len-1]
    @param length of string to parse
    @param minimal value for result
    @param maximum value for result
    @return EXIT_FAILURE if resulting value is outside the limits, EXIT_SUCCESS otherwise
*/
uint8_t str_to_uint32(char *str, uint32_t * out, const uint8_t seek,
                      const uint8_t len, const uint32_t min, const uint32_t max);

/** convert a string to a 16bit unsigned integer
    @param pointer to string to be converted
    @param pointer to resulting uint16_t value [min .. max]
    @param how many chars to skip [0 .. len-1]
    @param length of string to parse
    @param minimal value for result
    @param maximum value for result
    @return EXIT_FAILURE if resulting value is outside the limits, EXIT_SUCCESS otherwise
*/
uint8_t str_to_uint16(char *str, uint16_t * out, const uint8_t seek,
                      const uint8_t len, const uint16_t min, const uint16_t max);

/** convert a base-10 integer value to a binary-coded decimal (BCD)
    @param input decimal value to be converted
    @return BCD conversion
*/
uint8_t dec_to_bcd(const uint8_t val);

/** convert a binary-coded decimal (BCD) to a base-10 integer value
    @param BCD integer to be converted
    @return input decimal value to be converted
*/
uint8_t bcd_to_dec(const uint8_t val);

#ifdef __cplusplus
}
#endif

#endif
