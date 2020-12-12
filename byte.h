#ifndef BYTE_H_
#define BYTE_H_ 1

typedef unsigned char byte_t

#define byte_hi(_x) ({(byte_t)(_x >> 8)})
#define byte_lo(_x) ({(byte_t)(_x & 0xFF)})


#endif
