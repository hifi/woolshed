#include "res.h"
#include "debug.h"
#include "emul_ppc.h"
#include "heap.h"

// FIXME: support multiple files/forks
static uint8_t *res_data;
static uint32_t res_data_int;
static uint8_t *res_map;
static uint32_t res_max;
static emul_ppc_state *cpu;

typedef int (*res_callback)(uint32_t type, uint32_t id, uint32_t length, uint32_t data, uint32_t *handle, void *user);

static uint32_t read_int(uint8_t **p)
{
    uint32_t ret = PPC_INT(*(uint32_t *)*p);
    (*p) += 4;
    return ret;
}

static uint16_t read_short(uint8_t **p)
{
    uint16_t ret = PPC_SHORT(*(uint16_t *)*p);
    (*p) += 2;
    return ret;
}

static void res_iter(res_callback cb, void *user)
{
    uint8_t *p = res_map;

    read_int(&p); // header
    read_int(&p); //
    read_int(&p); //
    read_int(&p); //

    read_int(&p); // next
    read_short(&p); // res
    read_short(&p); // attr

    uint16_t typeOffset = read_short(&p);
    uint16_t nameOffset = read_short(&p);
    uint16_t count = read_short(&p) + 1;

    INFO("typeOffset = %d", typeOffset);
    INFO("nameOffset = %d", nameOffset);
    INFO("count = %d", count);

    p = res_map + typeOffset + 2; // why magic 2?

    for (uint32_t i = 0; i < count; i++)
    {
        uint32_t type = read_int(&p);
        uint16_t ntype = read_short(&p) + 1;
        uint16_t toffset = read_short(&p);

        INFO("resource type: %08X, ntype: %d, toffset: %d", type, ntype, toffset);

        uint8_t *typeList = res_map + typeOffset + toffset;

        for (uint32_t j = 0; j < ntype; j++)
        {
            uint16_t rid = read_short(&typeList);
            uint16_t noffset = read_short(&typeList);
            uint32_t tmp = read_int(&typeList);
            uint8_t rattr = tmp >> 24;
            uint16_t doffset = tmp & 0xFFFF;
            uint32_t *rhandle = (uint32_t *)typeList;
            typeList += 4;

            INFO("  rid = %d, noffset = %08X, rattr = %01X, doffset = %04X, handle = %08X", rid, noffset, rattr, doffset, *rhandle);

            uint8_t *rdata = res_data + doffset;
            uint32_t dataLength = read_int(&rdata);

            if (!cb(type, rid, dataLength, res_data_int + doffset + 4, rhandle, user))
                return;
        }
    }
}

static int res_init_handle(uint32_t type, uint32_t id, uint32_t length, uint32_t data, uint32_t *handle, void *user)
{
    INFO("(type=%08X, id=%d, length=%d, data=%08X, handle=%p [%08X])", type, id, length, data, handle, *handle);

    uint32_t newPtr = heap_alloc(4);
    *handle = PPC_INT(newPtr);

    uint32_t *ptr = PPC_PTR(cpu, newPtr);
    *ptr = PPC_INT(data);

    INFO(" data at %08X set to new ptr at %08X", data, newPtr);

    return 1;
}

void res_init(emul_ppc_state *_cpu, uint32_t resourcePtr)
{
    cpu = _cpu;
    uint32_t *hdr = PPC_PTR(cpu, resourcePtr);

    INFO("data offset: %d, map offset: %d\n", PPC_INT(hdr[0]), PPC_INT(hdr[1]));

    res_data = (uint8_t *)hdr + PPC_INT(hdr[0]);
    res_data_int = resourcePtr + PPC_INT(hdr[0]);
    res_map = (uint8_t *)hdr + PPC_INT(hdr[1]);
    res_max = 1;

    res_iter(res_init_handle, 0);
}

typedef struct {
    uint32_t name;
    uint32_t id;
    uint32_t handle;
} res_find_args;

static int res_find_by_name_id(uint32_t name, uint32_t id, uint32_t length, uint32_t data, uint32_t *handle, void *user)
{
    res_find_args *args = user;

    if (name != args->name || id != args->id)
        return 1;

    INFO("found target handle %08X", PPC_INT(*handle));
    args->handle = PPC_INT(*handle);

    return 0;
}

uint32_t res_find(uint32_t name, uint32_t id)
{
    res_find_args args;

    INFO("(name=%08X, id=%d)", name, id);

    args.name = name;
    args.id = id;
    args.handle = 0;

    res_iter(res_find_by_name_id, &args);

    return args.handle;
}
