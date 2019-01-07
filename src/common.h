#include <stdint.h>
#include <stdbool.h>

int open_and_read(FILE**, int8_t**, uint32_t *, const char*, const char*);
bool file_exists(const char *path);
const char *file_basename(const char *path);
int file_copy(const char* from, const char *to);

const char* sectionKindStr(uint8_t kind);
const char* shareKindStr(uint8_t kind);

#ifdef __BIG_ENDIAN__
#define bswap16(a) (a)
#define bswap32(a) (a)
#else
#define bswap16(a) (((a >> 8) | (a << 8)) & 0xFFFF)
#define bswap32(a) (((a >> 24) & 0xFF) | ((a << 8) & 0xFF0000) | ((a >> 8) & 0xFF00) | ((a << 24) & 0xFF000000))
#endif
