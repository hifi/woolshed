#include <stdio.h>
#include <stdlib.h>

#include "emul_ppc.h"
#include "debug.h"

// FIXME: how to properly handle varargs?
void ppc_printf(emul_ppc_state *cpu)
{
    FIXME("(...) stub");

    const char *fmt = (char *)cpu->ram + cpu->r[3];
    printf("%s", fmt);
    printf("\n");

    PPC_RETURN_INT(cpu, 0);
}

void ppc___setjmp(emul_ppc_state *cpu)
{
    FIXME("() stub");
    PPC_RETURN_INT(cpu, 0);
}

// FIXME: should end CPU emulation instead of calling exit() directly
void ppc_exit(emul_ppc_state *cpu)
{
    FIXME("()");
    exit(0);
}
