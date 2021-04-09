#include <stdint.h>
#include <stdlib.h>

#define POCSAG_SYNC 0x7CD215D8
#define POCSAG_IDLE 0x7A89C197

#define POCSAG_FRAME_SIZE 2
#define POCSAG_BATCH_SIZE 16

#define POCSAG_PREAMBLE_LENGTH 576

#define POCSAG_FLAG_ADDRESS 0x000000
#define POCSAG_FLAG_MESSAGE 0x100000

#define POCSAG_FLAG_TEXT_DATA 0x3
#define POCSAG_FLAG_NUMERIC_DATA = 0x0

#define TEXT_BITS_PER_WORD 20
#define TEXT_BITS_PER_CHAR 7
#define CRC_BITS 10
#define CRC_GENERATOR 0b11101101001

uint32_t crc(uint32_t inputMsg);
uint32_t parity(uint32_t x);
uint32_t encodeCodeword(uint32_t msg);
uint32_t encodeASCII(uint32_t initial_offset, char *str, uint32_t *out);
int addressOffset(int address);
void encodeTransmission(int repeatIndex, int address, int fb, char *message, uint32_t *out);
size_t textMessageLength(int repeatIndex, int address, int numChars);