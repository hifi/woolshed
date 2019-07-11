#include <stdint.h>
#include <stdbool.h>
#include "emul_ppc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RES_NAME(a,b,c,d) ((a) << 24 | (b) << 16 | (c) << 8 | (d))
#define RES_PTR(handle) PPC_PTR(cpu, PPC_INT(*(uint32_t *)PPC_PTR(cpu, handle)))

void res_init(emul_ppc_state *cpu, uint32_t resourcePtr);
uint32_t res_find(uint32_t name, uint32_t id);

#ifdef __cplusplus
}
#endif
