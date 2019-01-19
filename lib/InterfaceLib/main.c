#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "debug.h"
#include "../src/emul_ppc.h"
#include "defs.h"
#include "SDL.h"

// FIXME: these must be part of the returned window ptr for multi-window apps to work
static SDL_Window *window;
static SDL_Renderer *renderer;

// application may give us this at some point
static QDGlobals *qd;

int ppc_GetApplLimit(emul_ppc_state *cpu)
{
    uint32_t ret = 0x0130;
    FIXME("() stub -> %08X", ret);
    PPC_RETURN_INT(cpu, ret);
}

int ppc_SetApplLimit(emul_ppc_state *cpu)
{
    FIXME("(%08X) stub", PPC_ARG_INT(cpu, 1));
    return 0;
}

int ppc_MaxApplZone(emul_ppc_state *cpu)
{
    FIXME("() stub");
    return 0;
}

int ppc_MoreMasters(emul_ppc_state *cpu)
{
    FIXME("() stub");
    return 0;
}

int ppc_SysEnvirons(emul_ppc_state *cpu)
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

int ppc_InitGraf(emul_ppc_state *cpu)
{
    void *thePort = PPC_ARG_PTR(cpu, 1);

    FIXME("(thePort=%p [0x%08X]) stub", thePort, PPC_ARG_INT(cpu, 1));

    qd = (void *)(((uint8_t *)thePort + sizeof(uint32_t)) - sizeof(QDGlobals));

    memset(qd, 0, sizeof(*qd));

    qd->screenBits.bounds.right = 1024;
    qd->screenBits.bounds.bottom = 768;

    FIXME("Setting screen size to %dx%d", qd->screenBits.bounds.right, qd->screenBits.bounds.bottom);
    return 0;
}

int ppc_InitFonts(emul_ppc_state *cpu)
{
    FIXME("() stub");
    return 0;
}

int ppc_InitWindows(emul_ppc_state *cpu)
{
    FIXME("() stub");
    return 0;
}

int ppc_InitMenus(emul_ppc_state *cpu)
{
    FIXME("() stub");
    return 0;
}

int ppc_TEInit(emul_ppc_state *cpu)
{
    FIXME("() stub");
    return 0;
}

int ppc_InitDialogs(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
    return 0;
}

int ppc_InitCursor(emul_ppc_state *cpu)
{
    FIXME("() stub");
    return 0;
}

int ppc_GetDateTime(emul_ppc_state *cpu)
{
    uint32_t *seconds = PPC_ARG_PTR(cpu, 1);

    FIXME("using UNIX time instead of correct epoch");
    *seconds = time(NULL);

    return 0;
}

int ppc_InsetRect(emul_ppc_state *cpu)
{
    Rect *r = PPC_ARG_PTR(cpu, 1);
    int16_t dv = PPC_ARG_SHORT(cpu, 2);
    int16_t dh = PPC_ARG_SHORT(cpu, 3);

    FIXME("(r=%p [0x%08X], dv=%d, dh=%d) stub", (void *)r, PPC_ARG_INT(cpu, 1), dv, dh);

    INFO("From:");
    INFO("r->top = %d", r->top);
    INFO("r->left = %d", r->left);
    INFO("r->right = %d", r->right);
    INFO("r->bottom = %d", r->bottom);

    r->top += dv;
    r->bottom -= fminl(dv, r->right);

    r->left += dh;
    r->right -= fminl(dh, r->right);

    INFO("To:");
    INFO("r->top = %d", r->top);
    INFO("r->left = %d", r->left);
    INFO("r->right = %d", r->right);
    INFO("r->bottom = %d", r->bottom);

    return 0;
}

int ppc_NewCWindow(emul_ppc_state *cpu)
{
    void *wStorage = PPC_ARG_PTR(cpu, 1);
    Rect *boundsRect = PPC_ARG_PTR(cpu, 2);
    const Str255 *title = PPC_ARG_PTR(cpu, 3);
    uint32_t visible = PPC_ARG_INT(cpu, 4);
    uint32_t procID = PPC_ARG_INT(cpu, 5);
    void *behind = PPC_ARG_PTR(cpu, 6);
    uint32_t goAwayFlag = PPC_ARG_INT(cpu, 7);
    uint32_t refCon = PPC_ARG_INT(cpu, 8);

    SDL_Surface *surface;

    FIXME("(wStorage=%p, boundsRect=%p, title = '%s' (len=%d), visible=%d, procID=%d, behind=%p, goAwayFlag=%d, refCon=%d) stub",
            wStorage, boundsRect, title->str, title->len, visible, procID, behind, goAwayFlag, refCon);

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
            title->str,
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            boundsRect->right - boundsRect->left,
            boundsRect->bottom - boundsRect->top,
            0);

    surface = SDL_GetWindowSurface(window);
    renderer = SDL_CreateSoftwareRenderer(surface);

    return 0;
}

int ppc_SetPort(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
    return 0;
}

int ppc_TextSize(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
    return 0;
}

// FIXME: implement custom Mersenne Twister that uses qd->randSeed directly
static int rand_init = 0;
int ppc_Random(emul_ppc_state *cpu)
{
    if (!rand_init && qd)
    {
        srand(qd->randSeed);
        rand_init = 1;
    }

    int16_t ret = (int16_t)((rand() % 65536) - 32767);
    FIXME("Returning %d", ret);
    PPC_RETURN_INT(cpu, ret & 0xFFFF);
}

int ppc_RGBForeColor(emul_ppc_state *cpu)
{
    RGBColor *rgb = PPC_ARG_PTR(cpu, 1);

    INFO("(%d, %d, %d)\n", rgb->red, rgb->green, rgb->blue);

    SDL_SetRenderDrawColor(
            renderer,
            rgb->red >> 16,
            rgb->green >> 16,
            rgb->blue >> 16,
            0xFF);

    return 0;
}

int ppc_SetRect(emul_ppc_state *cpu)
{
    Rect *r = PPC_ARG_PTR(cpu, 1);
    uint16_t left = PPC_ARG_INT(cpu, 2);
    uint16_t top = PPC_ARG_INT(cpu, 3);
    uint16_t right = PPC_ARG_INT(cpu, 4);
    uint16_t bottom = PPC_ARG_INT(cpu, 5);

    FIXME("(rect=%p, left=%d, top=%d, right=%d, bottom=%d)", r, left, top, right, bottom);

    return 0;
}

int ppc_MoveTo(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
    return 0;
}

int ppc_PaintOval(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
    return 0;
}

int ppc_InvertColor(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
    return 0;
}

int ppc_DrawString(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
    return 0;
}

int ppc_Button(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
    PPC_RETURN_INT(cpu, 1);
}

int ppc_FlushEvents(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
    return 0;
}

int ppc_GetMainDevice(emul_ppc_state *cpu)
{
    FIXME("() stub");
    return 0;
}

int ppc_SetGDevice(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
    return 0;
}

int ppc_GetCTable(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
    return 0;
}

int ppc_GetIndString(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
    return 0;
}

int ppc_ParamText(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
    return 0;
}

int ppc_GetResource(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
    PPC_RETURN_INT(cpu, 0);
}

int ppc_GetMBarHeight(emul_ppc_state *cpu)
{
    FIXME("() stub");
    PPC_RETURN_INT(cpu, 8);
}

int ppc_Alert(emul_ppc_state *cpu)
{
    FIXME("(...) stub");
    PPC_RETURN_INT(cpu, -1);
}

int ppc_ExitToShell(emul_ppc_state *cpu)
{
    FIXME("()");
    SDL_Quit();
    return 1;
}
