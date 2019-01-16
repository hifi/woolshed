#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "../src/emul_ppc.h"
#include "defs.h"

void ppc_GetApplLimit(emul_ppc_state *cpu)
{
    uint32_t ret = 0x0130;
    FIXME("() stub -> %08X", ret);
    PPC_RETURN_INT(cpu, ret);
}

void ppc_SetApplLimit(emul_ppc_state *cpu)
{
    FIXME("(%08X) stub", PPC_ARG_INT(cpu, 1));
}

void ppc_MaxApplZone(emul_ppc_state *cpu)
{
    FIXME("() stub");
}

void ppc_MoreMasters(emul_ppc_state *cpu)
{
    FIXME("() stub");
}

void ppc_SysEnvirons(emul_ppc_state *cpu)
{
    uint32_t versionRequested = PPC_ARG_INT(cpu, 1);
    SysEnvRec *theWorld = PPC_ARG_PTR(cpu, 2);

    FIXME("(versionRequested=%d, theWorld=%p [0x%08X])", versionRequested, (void *)theWorld, PPC_ARG_INT(cpu, 2));

    theWorld->environsVersion = 0xFFFF;
    theWorld->machineType = 0xFFFF;
    theWorld->systemVersion = 0xFFFF;
    theWorld->processor = 0xFFFF;
    theWorld->hasFPU = 0;
    theWorld->hasColorQD = 1;

    PPC_RETURN_INT(cpu, 0);
}

void ppc_InitGraf(emul_ppc_state *cpu)
{
    void *thePort = PPC_ARG_PTR(cpu, 1);

    FIXME("(thePort=%p [0x%08X]) stub", thePort, PPC_ARG_INT(cpu, 1));

    QDGlobals *qd = (void *)((uint8_t *)thePort - (sizeof(QDGlobals) - sizeof(uint32_t)));

    memset(qd, 0, sizeof(*qd));
    qd->screenBits.bounds.right = 1024;
    qd->screenBits.bounds.bottom = 768;
}

void ppc_InitFonts(emul_ppc_state *cpu)
{
    FIXME("() stub");
}

void ppc_InitWindows(emul_ppc_state *cpu)
{
    FIXME("() stub");
}

void ppc_InitMenus(emul_ppc_state *cpu)
{
    FIXME("() stub");
}

void ppc_TEInit(emul_ppc_state *cpu)
{
    FIXME("() stub");
}

void ppc_InitDialogs(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
}

void ppc_InitCursor(emul_ppc_state *cpu)
{
    FIXME("() stub");
}

void ppc_GetDateTime(emul_ppc_state *cpu)
{
    uint32_t *seconds = PPC_ARG_PTR(cpu, 1);

    FIXME("(seconds=%p) stub", (void *)seconds);
}

void ppc_InsetRect(emul_ppc_state *cpu)
{
    Rect *r = PPC_ARG_PTR(cpu, 1);
    int16_t dv = PPC_ARG_SHORT(cpu, 2);
    int16_t dh = PPC_ARG_SHORT(cpu, 3);

    FIXME("(r=%p [0x%08X], dv=%d, dh=%d) stub", (void *)r, PPC_ARG_INT(cpu, 1), dv, dh);
    INFO("r->top = %d", r->top);
    INFO("r->left = %d", r->left);
    INFO("r->right = %d", r->right);
    INFO("r->bottom = %d", r->bottom);
}

void ppc_NewCWindow(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
}

void ppc_SetPort(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
}

void ppc_TextSize(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
}

void ppc_Random(emul_ppc_state *cpu)
{
    FIXME("() stub");
}

void ppc_RGBForeColor(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
}

void ppc_SetRect(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
}

void ppc_MoveTo(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
}

void ppc_PaintOval(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
}

void ppc_InvertColor(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
}

void ppc_DrawString(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
}

void ppc_Button(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
    PPC_RETURN_INT(cpu, 1);
}
