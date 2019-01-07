#include <stdio.h>
#include "../src/emul_ppc.h"

void emul_GetApplLimit(emul_ppc_state *cpu)
{
    cpu->r[3] = 0x0130;
    printf("GetApplLimit() -> %08X\n", cpu->r[3]);
}

void emul_SetApplLimit(emul_ppc_state *cpu)
{
    printf("SetApplLimit(%08X)\n", cpu->r[3]);
}

void emul_MaxApplZone(emul_ppc_state *cpu)
{
    printf("MaxApplZone()\n");
}

void emul_MoreMasters(emul_ppc_state *cpu)
{
    printf("MoreMasters()\n");
}
