#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "../src/emul_ppc.h"
#include "../src/res.h"
#include "defs.h"

#define MAX_RES 128
static void* resources[MAX_RES];
static uint32_t next_res;

extern "C" int ppc_SetResLoad(emul_ppc_state *cpu)
{
    FIXME("(%d) stub", PPC_ARG_INT(cpu, 1));
    PPC_RETURN_INT(cpu, 0);
}

extern "C" int ppc_GetResource(emul_ppc_state *cpu)
{
    union {
        char s[4];
        uint32_t i;
    } name;

    name.i = PPC_INT(PPC_ARG_INT(cpu, 1));
    uint32_t id = PPC_ARG_INT(cpu, 2);

    FIXME("(name='%c%c%c%c', id=%d) stub", name.s[0], name.s[1], name.s[2], name.s[3], id);

    uint32_t handle = 0;
    if ((handle = res_find(PPC_INT(name.i), id)) > 0)
    {
        INFO("found! returning %08X", handle);
        PPC_RETURN_INT(cpu, handle);
    }
    else
    {
        WARN("NOT found!");
        PPC_RETURN_INT(cpu, 0);
    }
}

extern "C" int ppc_GetResourceSizeOnDisk(emul_ppc_state *cpu)
{
    uint32_t handle = PPC_ARG_INT(cpu, 1);

    FIXME("(handle=%08X) stub", handle);
#if 0

    uint32_t size = 0;
    if (res_find(0, 0, handle, NULL, &size))
    {
        PPC_RETURN_INT(cpu, size);
    }

#endif
    PPC_RETURN_INT(cpu, 0);
}

extern "C" int ppc_ReadPartialResource(emul_ppc_state *cpu)
{
    uint32_t handle = PPC_ARG_INT(cpu, 1);
    uint32_t offset = PPC_ARG_INT(cpu, 2);
    uint8_t *buffer = (uint8_t *)PPC_ARG_PTR(cpu, 3);
    uint32_t count = PPC_ARG_INT(cpu, 4);

    FIXME("(handle=%08X, offset=%d, buffer=0x%08X, count=%d) stub", handle, offset, PPC_ARG_INT(cpu, 3), count);
#if 0

    INFO("(handle=%08X, offset=%d, buffer=0x%08X, count=%d)", handle, offset, PPC_ARG_INT(cpu, 3), count);

    uint8_t *ptr = NULL;
    if (!res_find(0, 0, handle, &ptr, NULL))
    {
        WARN("trying to read from invalid resource handle %08X", handle);
        PPC_RETURN_INT(cpu, 0);
    }

    for (uint32_t i = 0; i < count; i++)
        buffer[i] = ptr[i + offset];

#endif
    PPC_RETURN_INT(cpu, 1);
}

extern "C" int ppc_HLock(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
    PPC_RETURN_INT(cpu, 0);
}
