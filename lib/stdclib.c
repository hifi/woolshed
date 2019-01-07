#include <stdio.h>
#include <byteswap.h>
#include <stdlib.h>
#include "../src/emul_ppc.h"

// FIXME: how to properly handle varargs?
void emul_printf(emul_ppc_state *cpu)
{
    printf("printf\n");

    const char *fmt = (char *)cpu->ram + cpu->r[3];
    printf("%s", fmt);
    printf("\n");

    cpu->r[3] = 0;
}

// FIXME: stub
void emul___setjmp(emul_ppc_state *cpu)
{
    printf("__setjmp\n");
    cpu->r[3] = 0;
}

// FIXME: should end CPU emulation instead of calling exit() directly
void emul_exit(emul_ppc_state *cpu)
{
    printf("exit()\n");
    exit(0);
}
