#include <stdint.h>
#include <byteswap.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

// FIXME: it would be nice if the whole CPU emulation was using a state struct
// that could be used to handle syscalls and other fancy things in the calling
// code rather than emul_ppc calling out, see what Unicorn does? the CPU code
// should be pluggable enough a better emulator could be used if emul_ppc proves
// to be too incomplete or slow
//
// so in the case of the magic extended opcode 6 it could throw illegal
// instruction exception that's handled in the calling code by resetting it and
// reading the custom import address
typedef struct {
    void *ram;
    uint32_t ram_size;
    uint32_t r[32];
    double fr[32];
    uint32_t lr, ctr;
    uint32_t cr, xer;
    uint32_t fpscr;
    uint32_t pc;
} emul_ppc_state;

#define PPC_ARG_UINT(cpu, i) cpu->r[2 + i]
#define PPC_ARG_PTR(cpu, i) (void *)((uint8_t *)cpu->ram + cpu->r[2 + i])
#define PPC_RETURN_UINT(cpu, i) cpu->r[3] = i

#define BigWord bswap_32
#define BigHalf bswap_16

extern void *ram;
extern uint32_t ram_size;
void init_emul_ppc(uint32 start, uint32 toc);
uint32 emul_ppc();
void emul_ppc_dump();
