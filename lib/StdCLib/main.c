#include <stdio.h>
#include <stdlib.h>

#include "emul_ppc.h"
#include "debug.h"

// FIXME: how to properly handle varargs?
int ppc_printf(emul_ppc_state *cpu)
{
    FIXME("(...) stub");

    const char *fmt = (char *)cpu->ram + cpu->r[3];
    printf("%s", fmt);
    printf("\n");

    PPC_RETURN_INT(cpu, 0);
}

int ppc___setjmp(emul_ppc_state *cpu)
{
    FIXME("() stub");
    PPC_RETURN_INT(cpu, 0);
}

int ppc_exit(emul_ppc_state *cpu)
{
    INFO("()");
    return 1;
}
