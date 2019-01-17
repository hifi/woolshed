/*
 *  emul_ppc.cpp - PowerPC processor emulation
 *
 *  SheepShaver (C) 1997-2008 Christian Bauer and Marc Hellwig
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
TODO:
addme
addmeo
addze
addzeo
dcbst
dcbt
dcbtst
divwuo
fabs
fadd
fadds
fcmpo
fcmpu
fctiw
fctiwz
fdiv
fdivs
fmadd
fmadds
fmr
fmsub
fmsubs
fmul
fmuls
fnabs
fneg
fnmadd
fnmadds
fnmsub
fnmsubs
fres
frsp
frsqrte
fsel
fsqrt
fsqrts
fsub
fsubs
lfdu
lfdux
lfdx
lfs
lfsu
lfsux
lfsx
lhbrx
lwbrx
mcrfs
mcrxr
mtfsb0
mtfsb1
mtfsfi
mulhwu
mullwo
nego
sc
stfdu
stfdux
stfdx
stfs
stfsu
stfsux
stfsx
sthbrx
stwbrx
subfo
subfme
subfmeo
subfze
subfzeo
tw
twi

CHECK:
crxor
creqv
 */

#include <stdio.h>
#include <string.h>

#include "emul_ppc.h"
#include "debug.h"

// Convert 8-bit field mask (e.g. mtcrf) to bit mask
static uint32_t field2mask[256];

static inline uint8_t ReadMacInt8(emul_ppc_state *cpu, uint32_t addr)
{
    if (addr > cpu->ram_size)
        ERROR("Address outside RAM: 0x%08X\n", addr);

    return *((uint8_t *)cpu->ram + addr);
}

static inline uint16_t ReadMacInt16(emul_ppc_state *cpu, uint32_t addr)
{
    if (addr > cpu->ram_size)
        ERROR("Address outside RAM: 0x%08X\n", addr);

    return PPC_SHORT(*(uint16_t *)((uint8_t *)cpu->ram + addr));
}

static inline uint32_t ReadMacInt32(emul_ppc_state *cpu, uint32_t addr)
{
    if (addr > cpu->ram_size)
        ERROR("Address outside RAM: 0x%08X\n", addr);

    return PPC_INT(*(uint32_t *)((uint8_t *)cpu->ram + addr));
}

static inline uint64_t ReadMacInt64(emul_ppc_state *cpu, uint32_t addr)
{
    if (addr > cpu->ram_size)
        ERROR("Address outside RAM: 0x%08X\n", addr);

    return PPC_INT64(*(uint64_t *)((uint8_t *)cpu->ram + addr));
}

static inline void WriteMacInt8(emul_ppc_state *cpu, uint32_t addr, uint8_t val)
{
    if (addr > cpu->ram_size)
        ERROR("Address outside RAM: 0x%08X\n", addr);

    *((uint8_t *)cpu->ram + addr) = val;
}

static inline void WriteMacInt16(emul_ppc_state *cpu, uint32_t addr, uint16_t val)
{
    if (addr > cpu->ram_size)
        ERROR("Address outside RAM: 0x%08X\n", addr);

    *(uint16_t *)((uint8_t *)cpu->ram + addr) = PPC_SHORT(val);
}

static inline void WriteMacInt32(emul_ppc_state *cpu, uint32_t addr, uint32_t val)
{
    if (addr > cpu->ram_size)
        ERROR("Address outside RAM: 0x%08X\n", addr);

    *(uint32_t *)((uint8_t *)cpu->ram + addr) = PPC_INT(val);
}

static inline void WriteMacInt64(emul_ppc_state *cpu, uint32_t addr, uint64_t val)
{
    if (addr > cpu->ram_size)
        ERROR("Address outside RAM: 0x%08X\n", addr);

    *(uint64_t *)((uint8_t *)cpu->ram + addr) = PPC_INT64(val);
}

/*
 *  Dump PPC registers
 */

void emul_ppc_dump(emul_ppc_state *cpu)
{
    // Dump registers
    printf(" r0 %08x   r1 %08x   r2 %08x   r3 %08x\n", cpu->r[0], cpu->r[1], cpu->r[2], cpu->r[3]);
    printf(" r4 %08x   r5 %08x   r6 %08x   r7 %08x\n", cpu->r[4], cpu->r[5], cpu->r[6], cpu->r[7]);
    printf(" r8 %08x   r9 %08x  r10 %08x  r11 %08x\n", cpu->r[8], cpu->r[9], cpu->r[10], cpu->r[11]);
    printf("r12 %08x  r13 %08x  r14 %08x  r15 %08x\n", cpu->r[12], cpu->r[13], cpu->r[14], cpu->r[15]);
    printf("r16 %08x  r17 %08x  r18 %08x  r19 %08x\n", cpu->r[16], cpu->r[17], cpu->r[18], cpu->r[19]);
    printf("r20 %08x  r21 %08x  r22 %08x  r23 %08x\n", cpu->r[20], cpu->r[21], cpu->r[22], cpu->r[23]);
    printf("r24 %08x  r25 %08x  r26 %08x  r27 %08x\n", cpu->r[24], cpu->r[25], cpu->r[26], cpu->r[27]);
    printf("r28 %08x  r29 %08x  r30 %08x  r31 %08x\n", cpu->r[28], cpu->r[29], cpu->r[30], cpu->r[31]);
    printf(" lr %08x  ctr %08x   cr %08x  xer %08x\n", cpu->lr, cpu->ctr, cpu->cr, cpu->xer);
    printf(" pc %08x fpscr %08x\n", cpu->pc, cpu->fpscr);
}

/*
 *  Record result in CR0
 */

static void record(emul_ppc_state *cpu, uint32_t val)
{
    uint32_t crf = 0;
    if (val == 0)
        crf |= 0x20000000;
    else if (val & 0x80000000)
        crf |= 0x80000000;
    else
        crf |= 0x40000000;
    if (cpu->xer & 0x80000000)
        crf |= 0x10000000;
    cpu->cr = (cpu->cr & 0x0fffffff) | crf;
}


/*
 *  Record result in CR1
 */

static inline void record1(emul_ppc_state *cpu)
{
    cpu->cr = (cpu->cr & 0xf0ffffff) | ((cpu->fpscr >> 4) & 0x0f000000);
}


/*
 *  Convert mask begin/end to mask
 */

static uint32_t mbme2mask(uint32_t op)
{
    uint32_t mb = (op >> 6) & 0x1f;
    uint32_t me = (op >> 1) & 0x1f;
    uint32_t m = 0;
    uint32_t i;

    if (mb <= me)
        for (i=mb; i<=me; i++)
            m |= 0x80000000 >> i;
    else {
        for (i=0; i<=me; i++)
            m |= 0x80000000 >> i;
        for (i=mb; i<=31; i++)
            m |= 0x80000000 >> i;
    }
    return m;
}


/*
 *  Emulate instruction with primary opcode = 19
 */

static void emul19(emul_ppc_state *cpu, uint32_t op)
{
    uint32_t exop = (op >> 1) & 0x3ff;
    uint32_t rd = (op >> 21) & 0x1f;
    uint32_t ra = (op >> 16) & 0x1f;
    switch (exop) {

        case 0: {    // mcrf
            uint32_t crfd = 0x1c - (rd & 0x1c);
            uint32_t crfa = 0x1c - (ra & 0x1c);
            uint32_t crf = (cpu->cr >> crfa) & 0xf;
            cpu->cr = (cpu->cr & ~(0xf << crfd)) | (crf << crfd);
            break;
        }

        case 16: {    // bclr
            uint32_t oldpc = cpu->pc;
            if (!(rd & 4)) {
                cpu->ctr--;
                if (rd & 2) {
                    if (cpu->ctr)
                        goto blr_nobranch;
                } else {
                    if (!cpu->ctr)
                        goto blr_nobranch;
                }
            }
            if (!(rd & 0x10)) {
                if (rd & 8) {
                    if (!(cpu->cr & (0x80000000 >> ra)))
                        goto blr_nobranch;
                } else {
                    if (cpu->cr & (0x80000000 >> ra))
                        goto blr_nobranch;
                }
            }
            cpu->pc = cpu->lr & 0xfffffffc;
blr_nobranch:
            if (op & 1)
                cpu->lr = oldpc;
            break;
        }

        case 33:    // crnor
            if ((cpu->cr & (0x80000000 >> ra)) || ((cpu->cr & (0x80000000 >> ((op >> 11) & 0x1f)))))
                cpu->cr &= ~(0x80000000 >> rd);
            else
                cpu->cr |= 0x80000000 >> rd;
            break;

        case 129:    // crandc
            if ((cpu->cr & (0x80000000 >> ra)) && !((cpu->cr & (0x80000000 >> ((op >> 11) & 0x1f)))))
                cpu->cr |= 0x80000000 >> rd;
            else
                cpu->cr &= ~(0x80000000 >> rd);
            break;

        case 150:    // isync
            break;

        case 193: {    // crxor
            uint32_t mask = 0x80000000 >> rd;
            cpu->cr = (((((cpu->cr >> (31 - ra)) ^ (cpu->cr >> (31 - ((op >> 11) & 0x1f)))) & 1) << (31 - rd)) & mask) | (cpu->cr & ~mask);
            break;
        }

        case 225:    // crnand
            if ((cpu->cr & (0x80000000 >> ra)) && ((cpu->cr & (0x80000000 >> ((op >> 11) & 0x1f)))))
                cpu->cr &= ~(0x80000000 >> rd);
            else
                cpu->cr |= 0x80000000 >> rd;
            break;

        case 257:    // crand
            if ((cpu->cr & (0x80000000 >> ra)) && ((cpu->cr & (0x80000000 >> ((op >> 11) & 0x1f)))))
                cpu->cr |= 0x80000000 >> rd;
            else
                cpu->cr &= ~(0x80000000 >> rd);
            break;

        case 289: {    // cpu->creqv
            uint32_t mask = 0x80000000 >> rd;
            cpu->cr = (((~((cpu->cr >> (31 - ra)) ^ (cpu->cr >> (31 - ((op >> 11) & 0x1f)))) & 1) << (31 - rd)) & mask) | (cpu->cr & ~mask);
            break;
        }

        case 417:    // cpu->crorc
            if ((cpu->cr & (0x80000000 >> ra)) || !((cpu->cr & (0x80000000 >> ((op >> 11) & 0x1f)))))
                cpu->cr |= 0x80000000 >> rd;
            else
                cpu->cr &= ~(0x80000000 >> rd);
            break;

        case 449:    // cpu->cror
            if ((cpu->cr & (0x80000000 >> ra)) || ((cpu->cr & (0x80000000 >> ((op >> 11) & 0x1f)))))
                cpu->cr |= 0x80000000 >> rd;
            else
                cpu->cr &= ~(0x80000000 >> rd);
            break;

        case 528: {    // bcctr
            if (op & 1)
                cpu->lr = cpu->pc;
            if (!(rd & 4)) {
                cpu->ctr--;
                if (rd & 2) {
                    if (cpu->ctr)
                        goto bctr_nobranch;
                } else {
                    if (!cpu->ctr)
                        goto bctr_nobranch;
                }
            }
            if (!(rd & 0x10)) {
                if (rd & 8) {
                    if (!(cpu->cr & (0x80000000 >> ra)))
                        goto bctr_nobranch;
                } else {
                    if (cpu->cr & (0x80000000 >> ra))
                        goto bctr_nobranch;
                }
            }
            cpu->pc = cpu->ctr & 0xfffffffc;
bctr_nobranch:
            break;
        }

        default:
            fprintf(stderr, "emul_ppc: Illegal 19 opcode %08x (exop %d) at %08x\n", op, exop, cpu->pc-4);
            cpu->fault = 1;
            break;
    }
}


/*
 *  Emulate instruction with primary opcode = 31
 */

static void emul31(emul_ppc_state *cpu, uint32_t op)
{
    uint32_t exop = (op >> 1) & 0x3ff;
    uint32_t rd = (op >> 21) & 0x1f;
    uint32_t ra = (op >> 16) & 0x1f;
    uint32_t rb = (op >> 11) & 0x1f;
    switch (exop) {

        case 0:    {    // cmpw
            uint32_t crfd = 0x1c - (rd & 0x1c);
            uint8_t crf = 0;
            if (cpu->r[ra] == cpu->r[rb])
                crf |= 2;
            else if ((int32_t)cpu->r[ra] < (int32_t)cpu->r[rb])
                crf |= 8;
            else
                crf |= 4;
            if (cpu->xer & 0x80000000)
                crf |= 1;
            cpu->cr = (cpu->cr & ~(0xf << crfd)) | (crf << crfd);
            break;
        }

        case 8: {    // subfc
            uint64_t tmp = (uint64_t)cpu->r[rb] - (uint64_t)cpu->r[ra];
            cpu->r[rd] = tmp;
            if (tmp & 0x100000000LL)
                cpu->xer &= ~0x20000000;
            else
                cpu->xer |= 0x20000000;
            if (op & 1)
                record(cpu, cpu->r[rd]);
            break;
        }

        case 10: {    // addc
            uint64_t tmp = (uint64_t)cpu->r[ra] + (uint64_t)cpu->r[rb];
            cpu->r[rd] = tmp;
            if (tmp & 0x100000000LL)
                cpu->xer |= 0x20000000;
            else
                cpu->xer &= ~0x20000000;
            if (op & 1)
                record(cpu, cpu->r[rd]);
            break;
        }

        case 19:    // mfcr
            cpu->r[rd] = cpu->cr;
            break;

        case 20:    // lwarx
            cpu->r[rd] = ReadMacInt32(cpu, cpu->r[rb] + (ra ? cpu->r[ra] : 0));
            //!! set reservation bit
            break;

        case 23:    // lwzx
            cpu->r[rd] = ReadMacInt32(cpu, cpu->r[rb] + (ra ? cpu->r[ra] : 0));
            break;

        case 24:    // slw
            cpu->r[ra] = cpu->r[rd] << (cpu->r[rb] & 0x3f);
            if (op & 1)
                record(cpu, cpu->r[ra]);
            break;

        case 26: {    // cntlzw
            uint32_t mask = 0x80000000;
            for (int i=0; i<32; i++, mask>>=1) {
                if (cpu->r[rd] & mask) {
                    cpu->r[ra] = i;
                    goto cntlzw_done;
                }
            }
            cpu->r[ra] = 32;
cntlzw_done:if (op & 1)
                record(cpu, cpu->r[ra]);
            break;
        }

        case 28:    // and
            cpu->r[ra] = cpu->r[rd] & cpu->r[rb];
            if (op & 1)
                record(cpu, cpu->r[ra]);
            break;

        case 32: {    // cmplw
            uint32_t crfd = 0x1c - (rd & 0x1c);
            uint8_t crf = 0;
            if (cpu->r[ra] == cpu->r[rb])
                crf |= 2;
            else if (cpu->r[ra] < cpu->r[rb])
                crf |= 8;
            else
                crf |= 4;
            if (cpu->xer & 0x80000000)
                crf |= 1;
            cpu->cr = (cpu->cr & ~(0xf << crfd)) | (crf << crfd);
            break;
        }

        case 40:    // subf
            cpu->r[rd] = cpu->r[rb] - cpu->r[ra];
            if (op & 1)
                record(cpu, cpu->r[rd]);
            break;

        case 55:    // lwzux
            cpu->r[ra] += cpu->r[rb];
            cpu->r[rd] = ReadMacInt32(cpu, cpu->r[ra]);
            break;

        case 60:    // andc
            cpu->r[ra] = cpu->r[rd] & ~cpu->r[rb];
            if (op & 1)
                record(cpu, cpu->r[ra]);
            break;

        case 75:    // mulhw
            cpu->r[rd] = ((int64_t)(int32_t)cpu->r[ra] * (int32_t)cpu->r[rb]) >> 32;
            if (op & 1)
                record(cpu, cpu->r[rd]);
            break;

        case 86:    // dcbf
            break;

        case 87:    // lbzx
            cpu->r[rd] = ReadMacInt8(cpu, cpu->r[rb] + (ra ? cpu->r[ra] : 0));
            break;

        case 104:    // neg
            if (cpu->r[ra] == 0x80000000)
                cpu->r[rd] = 0x80000000;
            else
                cpu->r[rd] = -(int32_t)cpu->r[ra];
            if (op & 1)
                record(cpu, cpu->r[rd]);
            break;

        case 119:    // lbzux
            cpu->r[ra] += cpu->r[rb];
            cpu->r[rd] = ReadMacInt8(cpu, cpu->r[ra]);
            break;

        case 124:    // nor
            cpu->r[ra] = ~(cpu->r[rd] | cpu->r[rb]);
            if (op & 1)
                record(cpu, cpu->r[ra]);
            break;

        case 136: {    // subfe
            uint64_t tmp = (uint64_t)cpu->r[rb] - (uint64_t)cpu->r[ra];
            if (!(cpu->xer & 0x20000000))
                tmp--;
            cpu->r[rd] = tmp;
            if (tmp & 0x100000000LL)
                cpu->xer &= ~0x20000000;
            else
                cpu->xer |= 0x20000000;
            if (op & 1)
                record(cpu, cpu->r[rd]);
            break;
        }

        case 138: {    // adde
            uint64_t tmp = (uint64_t)cpu->r[ra] + (uint64_t)cpu->r[rb];
            if (cpu->xer & 0x20000000)
                tmp++;
            cpu->r[rd] = tmp;
            if (tmp & 0x100000000LL)
                cpu->xer |= 0x20000000;
            else
                cpu->xer &= ~0x20000000;
            if (op & 1)
                record(cpu, cpu->r[rd]);
            break;
        }

        case 144: {    // mtcrf
            uint32_t mask = field2mask[(op >> 12) & 0xff];
            cpu->cr = (cpu->r[rd] & mask) | (cpu->cr & ~mask);
            break;
        }

        case 150:    // stwcx
            //!! check reserved bit
            WriteMacInt32(cpu, cpu->r[rb] + (ra ? cpu->r[ra] : 0), cpu->r[rd]);
            record(cpu,0);
            break;

        case 151:    // stwx
            WriteMacInt32(cpu, cpu->r[rb] + (ra ? cpu->r[ra] : 0), cpu->r[rd]);
            break;

        case 183:    // stwux
            cpu->r[ra] += cpu->r[rb];
            WriteMacInt32(cpu, cpu->r[ra], cpu->r[rd]);
            break;

        case 202: {  // addze
            // FIXME: verify implementation
            uint64_t tmp = (uint64_t)cpu->r[ra];
            if (cpu->xer & 0x20000000)
                tmp++;
            cpu->r[rd] = tmp;
            if (tmp & 0x100000000LL)
                cpu->xer |= 0x20000000;
            else
                cpu->xer &= ~0x20000000;
            if (op & 1)
                record(cpu, cpu->r[rd]);
            break;
        }

        case 215:    // stbx
            WriteMacInt8(cpu, cpu->r[rb] + (ra ? cpu->r[ra] : 0), cpu->r[rd]);
            break;

        case 235:    // mullw
            cpu->r[rd] = (int32_t)cpu->r[ra] * (int32_t)cpu->r[rb];
            if (op & 1)
                record(cpu, cpu->r[rd]);
            break;

        case 247:    // stbux
            cpu->r[ra] += cpu->r[rb];
            WriteMacInt8(cpu, cpu->r[ra], cpu->r[rd]);
            break;

        case 266:    // add
            cpu->r[rd] = cpu->r[ra] + cpu->r[rb];
            if (op & 1)
                record(cpu, cpu->r[rd]);
            break;

        case 279:    // lhzx
            cpu->r[rd] = ReadMacInt16(cpu, cpu->r[rb] + (ra ? cpu->r[ra] : 0));
            break;

        case 284:    // eqv
            cpu->r[ra] = ~(cpu->r[rd] ^ cpu->r[rb]);
            if (op & 1)
                record(cpu, cpu->r[ra]);
            break;

        case 311:    // lhzux
            cpu->r[ra] += cpu->r[rb];
            cpu->r[rd] = ReadMacInt16(cpu, cpu->r[ra]);
            break;

        case 316:    // xor
            cpu->r[ra] = cpu->r[rd] ^ cpu->r[rb];
            if (op & 1)
                record(cpu, cpu->r[ra]);
            break;

        case 339: {    // mfspr
            uint32_t spr = ra | (rb << 5);
            switch (spr) {
                case 1: cpu->r[rd] = cpu->xer; break;
                case 8: cpu->r[rd] = cpu->lr; break;
                case 9: cpu->r[rd] = cpu->ctr; break;
                default:
                    fprintf(stderr, "emul_ppc: Illegal mfspr opcode %08x at %08x\n", op, cpu->pc-4);
                    cpu->fault = 1;
            }
            break;
        }

        case 343:    // lhax
            cpu->r[rd] = (int32_t)(int16_t)ReadMacInt16(cpu, cpu->r[rb] + (ra ? cpu->r[ra] : 0));
            break;

        case 371:    // mftb
            cpu->r[rd] = 0;    //!!
            break;

        case 375:    // lhaux
            cpu->r[ra] += cpu->r[rb];
            cpu->r[rd] = (int32_t)(int16_t)ReadMacInt16(cpu, cpu->r[ra]);
            break;

        case 407:    // sthx
            WriteMacInt16(cpu, cpu->r[rb] + (ra ? cpu->r[ra] : 0), cpu->r[rd]);
            break;

        case 412:    // orc
            cpu->r[ra] = cpu->r[rd] | ~cpu->r[rb];
            if (op & 1)
                record(cpu, cpu->r[ra]);
            break;

        case 439:    // sthux
            cpu->r[ra] += cpu->r[rb];
            WriteMacInt16(cpu, cpu->r[ra], cpu->r[rd]);
            break;

        case 444:    // or
            cpu->r[ra] = cpu->r[rd] | cpu->r[rb];
            if (op & 1)
                record(cpu, cpu->r[ra]);
            break;

        case 459:    // divwu
            if (cpu->r[rb])
                cpu->r[rd] = cpu->r[ra] / cpu->r[rb];
            if (op & 1)
                record(cpu, cpu->r[rd]);
            break;

        case 467: {    // mtspr
            uint32_t spr = ra | (rb << 5);
            switch (spr) {
                case 1: cpu->xer = cpu->r[rd] & 0xe000007f; break;
                case 8: cpu->lr = cpu->r[rd]; break;
                case 9: cpu->ctr = cpu->r[rd]; break;
                default:
                    fprintf(stderr, "emul_ppc: Illegal mtspr opcode %08x at %08x\n", op, cpu->pc-4);
                    cpu->fault = 1;
            }
            break;
        }

        case 476:    // nand
            cpu->r[ra] = ~(cpu->r[rd] & cpu->r[rb]);
            if (op & 1)
                record(cpu, cpu->r[ra]);
            break;

        case 491:    // divw
            if (cpu->r[rb])
                cpu->r[rd] = (int32_t)cpu->r[ra] / (int32_t)cpu->r[rb];
            if (op & 1)
                record(cpu, cpu->r[rd]);
            break;

        case 520: {    // subfco
            uint64_t tmp = (uint64_t)cpu->r[rb] - (uint64_t)cpu->r[ra];
            uint32_t ov = (cpu->r[ra] ^ cpu->r[rb]) & ((uint32_t)tmp ^ cpu->r[rb]);
            cpu->r[rd] = tmp;
            if (tmp & 0x100000000LL)
                cpu->xer &= ~0x20000000;
            else
                cpu->xer |= 0x20000000;
            if (ov & 0x80000000)
                cpu->xer |= 0xc0000000;
            else
                cpu->xer &= ~0x40000000;
            if (op & 1)
                record(cpu, cpu->r[rd]);
            break;
        }

        case 522: {    // addco
            uint64_t tmp = (uint64_t)cpu->r[ra] + (uint64_t)cpu->r[rb];
            uint32_t ov = (cpu->r[ra] ^ (uint32_t)tmp) & (cpu->r[rb] ^ (uint32_t)tmp);
            cpu->r[rd] = tmp;
            if (tmp & 0x100000000LL)
                cpu->xer |= 0x20000000;
            else
                cpu->xer &= ~0x20000000;
            if (ov & 0x80000000)
                cpu->xer |= 0xc0000000;
            else
                cpu->xer &= ~0x40000000;
            if (op & 1)
                record(cpu, cpu->r[rd]);
            break;
        }

        case 533: {    // lswx
            uint32_t addr = cpu->r[rb] + (ra ? cpu->r[ra] : 0);
            int nb = cpu->xer & 0x7f;
            int reg = rd;
            for (int i=0; i<nb; i++) {
                switch (i & 3) {
                    case 0:
                        cpu->r[reg] = ReadMacInt8(cpu, addr + i) << 24;
                        break;
                    case 1:
                        cpu->r[reg] = (cpu->r[reg] & 0xff00ffff) | (ReadMacInt8(cpu, addr + i) << 16);
                        break;
                    case 2:
                        cpu->r[reg] = (cpu->r[reg] & 0xffff00ff) | (ReadMacInt8(cpu, addr + i) << 8);
                        break;
                    case 3:
                        cpu->r[reg] = (cpu->r[reg] & 0xffffff00) | ReadMacInt8(cpu, addr + i);
                        reg = (reg + 1) & 0x1f;
                        break;
                }
            }
            break;
        }

        case 536:    // srw
            cpu->r[ra] = cpu->r[rd] >> (cpu->r[rb] & 0x3f);
            if (op & 1)
                record(cpu, cpu->r[ra]);
            break;

        case 597: {    // lswi
            uint32_t addr = ra ? cpu->r[ra] : 0;
            int nb = rb ? rb : 32;
            int reg = rd;
            for (int i=0; i<nb; i++) {
                switch (i & 3) {
                    case 0:
                        cpu->r[reg] = ReadMacInt8(cpu, addr + i) << 24;
                        break;
                    case 1:
                        cpu->r[reg] = (cpu->r[reg] & 0xff00ffff) | (ReadMacInt8(cpu, addr + i) << 16);
                        break;
                    case 2:
                        cpu->r[reg] = (cpu->r[reg] & 0xffff00ff) | (ReadMacInt8(cpu, addr + i) << 8);
                        break;
                    case 3:
                        cpu->r[reg] = (cpu->r[reg] & 0xffffff00) | ReadMacInt8(cpu, addr + i);
                        reg = (reg + 1) & 0x1f;
                        break;
                }
            }
            break;
        }

        case 598:    // sync
            break;

        case 648: {    // subfeo
            uint64_t tmp = (uint64_t)cpu->r[rb] - (uint64_t)cpu->r[ra];
            if (!(cpu->xer & 0x20000000))
                tmp--;
            uint32_t ov = (cpu->r[ra] ^ cpu->r[rb]) & ((uint32_t)tmp ^ cpu->r[rb]);
            cpu->r[rd] = tmp;
            if (tmp & 0x100000000LL)
                cpu->xer &= ~0x20000000;
            else
                cpu->xer |= 0x20000000;
            if (ov & 0x80000000)
                cpu->xer |= 0xc0000000;
            else
                cpu->xer &= ~0x40000000;
            if (op & 1)
                record(cpu, cpu->r[rd]);
            break;
        }

        case 650: {    // addeo
            uint64_t tmp = (uint64_t)cpu->r[ra] + (uint64_t)cpu->r[rb];
            if (cpu->xer & 0x20000000)
                tmp++;
            uint32_t ov = (cpu->r[ra] ^ (uint32_t)tmp) & (cpu->r[rb] ^ (uint32_t)tmp);
            cpu->r[rd] = tmp;
            if (tmp & 0x100000000LL)
                cpu->xer |= 0x20000000;
            else
                cpu->xer &= ~0x20000000;
            if (ov & 0x80000000)
                cpu->xer |= 0xc0000000;
            else
                cpu->xer &= ~0x40000000;
            if (op & 1)
                record(cpu, cpu->r[rd]);
            break;
        }

        case 661: {    // stswx
            uint32_t addr = cpu->r[rb] + (ra ? cpu->r[ra] : 0);
            int nb = cpu->xer & 0x7f;
            int reg = rd;
            int shift = 24;
            for (int i=0; i<nb; i++) {
                WriteMacInt8(cpu, addr + i, (cpu->r[reg] >> shift));
                shift -= 8;
                if ((i & 3) == 3) {
                    shift = 24;
                    reg = (reg + 1) & 0x1f;
                }
            }
            break;
        }

        case 725: {    // stswi
            uint32_t addr = ra ? cpu->r[ra] : 0;
            int nb = rb ? rb : 32;
            int reg = rd;
            int shift = 24;
            for (int i=0; i<nb; i++) {
                WriteMacInt8(cpu, addr + i, (cpu->r[reg] >> shift));
                shift -= 8;
                if ((i & 3) == 3) {
                    shift = 24;
                    reg = (reg + 1) & 0x1f;
                }
            }
            break;
        }

        case 778: {    // addo
            uint32_t tmp = cpu->r[ra] + cpu->r[rb];
            uint32_t ov = (cpu->r[ra] ^ tmp) & (cpu->r[rb] ^ tmp);
            cpu->r[rd] = tmp;
            if (ov & 0x80000000)
                cpu->xer |= 0xc0000000;
            else
                cpu->xer &= ~0x40000000;
            if (op & 1)
                record(cpu, cpu->r[rd]);
            break;
        }

        case 792: {    // sraw
            uint32_t sh = cpu->r[rb] & 0x3f;
            uint32_t mask = ~(0xffffffff << sh);
            if ((cpu->r[rd] & 0x80000000) && (cpu->r[rd] & mask))
                cpu->xer |= 0x20000000;
            else
                cpu->xer &= ~0x20000000;
            cpu->r[ra] = (int32_t)cpu->r[rd] >> sh;
            if (op & 1)
                record(cpu, cpu->r[ra]);
            break;
        }

        case 824: {    // srawi
            uint32_t mask = ~(0xffffffff << rb);
            if ((cpu->r[rd] & 0x80000000) && (cpu->r[rd] & mask))
                cpu->xer |= 0x20000000;
            else
                cpu->xer &= ~0x20000000;
            cpu->r[ra] = (int32_t)cpu->r[rd] >> rb;
            if (op & 1)
                record(cpu, cpu->r[ra]);
            break;
        }

        case 854:    // eieio
            break;

        case 922:    // extsh
            cpu->r[ra] = (int32_t)(int16_t)cpu->r[rd];
            if (op & 1)
                record(cpu, cpu->r[ra]);
            break;

        case 954:    // extsb
            cpu->r[ra] = (int32_t)(int8_t)cpu->r[rd];
            if (op & 1)
                record(cpu, cpu->r[ra]);
            break;

        case 982:    // icbi
            break;

        case 1003:    // divwo
            if (cpu->r[rb] == 0 || (cpu->r[ra] == 0x80000000 && cpu->r[rb] == 0xffffffff))
                cpu->xer |= 0xc0000000;
            else {
                cpu->r[rd] = (int32_t)cpu->r[ra] / (int32_t)cpu->r[rb];
                cpu->xer &= ~0x40000000;
            }
            if (op & 1)
                record(cpu, cpu->r[rd]);
            break;

#if 0
        case 1014:    // dcbz
            memset(r[rb] + (ra ? cpu->r[ra] : 0), 0, 32);
            break;
#endif

        default:
            fprintf(stderr, "emul_ppc: Illegal 31 opcode %08x (exop %d) at %08x\n", op, exop, cpu->pc-4);
            cpu->fault = 1;
            break;
    }
}


/*
 *  Emulate instruction with primary opcode = 59
 */

static void emul59(emul_ppc_state *cpu, uint32_t op)
{
    uint32_t exop = (op >> 1) & 0x3ff;
    switch (exop) {
        default:
            fprintf(stderr, "emul_ppc: Illegal 59 opcode %08x (exop %d) at %08x\n", op, exop, cpu->pc-4);
            cpu->fault = 1;
            break;
    }
}


/*
 *  Emulate instruction with primary opcode = 63
 */

static void emul63(emul_ppc_state *cpu, uint32_t op)
{
    uint32_t exop = (op >> 1) & 0x3ff;
    uint32_t rd = (op >> 21) & 0x1f;
#if 0
    uint32_t ra = (op >> 16) & 0x1f;
    uint32_t rb = (op >> 11) & 0x1f;
#endif

    switch (exop) {

        case 583:    // mffs
            cpu->fr[rd] = (double)(uint64_t)cpu->fpscr;
            if (op & 1)
                record1(cpu);
            break;

        case 711:    // mtfsf
            //!!
            if (op & 1)
                record1(cpu);
            break;

        default:
            fprintf(stderr, "emul_ppc: Illegal 63 opcode %08x (exop %d) at %08x\n", op, exop, cpu->pc-4);
            cpu->fault = 1;
            break;
    }
}


/*
 *  Emulation step
 */

// FIXME: refactor imports
void run_import(uint32_t idx, emul_ppc_state *state);

int emul_ppc_run(emul_ppc_state *cpu, int step)
{
    while (!cpu->fault) {
        uint32_t op = ReadMacInt32(cpu, cpu->pc);
        uint32_t primop = op >> 26;
        cpu->pc += 4;

        switch (primop) {

            case 6: {      // FIXME: custom import call trap, use syscall instead?
                //printf("Extended opcode %08x at %08x (68k pc %08x)\n", op, pc-4, cpu->r[24]);
                uint32_t idx = (op & 0x3FFFFFF);
                run_import(idx, cpu);
                break;
            }

            case 7: {    // mulli
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[rd] = (int32_t)cpu->r[ra] * (int32_t)(int16_t)(op & 0xffff);
                break;
            }

            case 8: {    // subfic
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                uint64_t tmp = (uint32_t)(int32_t)(int16_t)(op & 0xffff) - (uint64_t)cpu->r[ra];
                cpu->r[rd] = tmp;
                if (tmp & 0x100000000LL)
                    cpu->xer &= ~0x20000000;
                else
                    cpu->xer |= 0x20000000;
                break;
            }

            case 10: {    // cmpli
                uint32_t crfd = 0x1c - ((op >> 21) & 0x1c);
                uint32_t ra = (op >> 16) & 0x1f;
                uint32_t val = (op & 0xffff);
                uint8_t crf = 0;
                if (cpu->r[ra] == val)
                    crf |= 2;
                else if (cpu->r[ra] < val)
                    crf |= 8;
                else
                    crf |= 4;
                if (cpu->xer & 0x80000000)
                    crf |= 1;
                cpu->cr = (cpu->cr & ~(0xf << crfd)) | (crf << crfd);
                break;
            }

            case 11: {    // cmpi
                uint32_t crfd = 0x1c - ((op >> 21) & 0x1c);
                uint32_t ra = (op >> 16) & 0x1f;
                int32_t val = (int32_t)(int16_t)(op & 0xffff);
                uint8_t crf = 0;
                if ((int32_t)cpu->r[ra] == val)
                    crf |= 2;
                else if ((int32_t)cpu->r[ra] < val)
                    crf |= 8;
                else
                    crf |= 4;
                if (cpu->xer & 0x80000000)
                    crf |= 1;
                cpu->cr = (cpu->cr & ~(0xf << crfd)) | (crf << crfd);
                break;
            }

            case 12: {    // addic
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                uint64_t tmp = (uint64_t)cpu->r[ra] + (uint32_t)(int32_t)(int16_t)(op & 0xffff);
                cpu->r[rd] = tmp;
                if (tmp & 0x100000000LL)
                    cpu->xer |= 0x20000000;
                else
                    cpu->xer &= ~0x20000000;
                break;
            }

            case 13: {    // addic.
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                uint64_t tmp = (uint64_t)cpu->r[ra] + (uint32_t)(int32_t)(int16_t)(op & 0xffff);
                cpu->r[rd] = tmp;
                if (tmp & 0x100000000LL)
                    cpu->xer |= 0x20000000;
                else
                    cpu->xer &= ~0x20000000;
                record(cpu, cpu->r[rd]);
                break;
            }

            case 14: {    // addi
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[rd] = (ra ? cpu->r[ra] : 0) + (int32_t)(int16_t)(op & 0xffff);
                break;
            }

            case 15: {    // addis
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[rd] = (ra ? cpu->r[ra] : 0) + (op << 16);
                break;
            }

            case 16: {    // bc
                uint32_t bo = (op >> 21) & 0x1f;
                uint32_t bi = (op >> 16) & 0x1f;
                if (op & 1)
                    cpu->lr = cpu->pc;
                if (!(bo & 4)) {
                    cpu->ctr--;
                    if (bo & 2) {
                        if (cpu->ctr)
                            goto bc_nobranch;
                    } else {
                        if (!cpu->ctr)
                            goto bc_nobranch;
                    }
                }
                if (!(bo & 0x10)) {
                    if (bo & 8) {
                        if (!(cpu->cr & (0x80000000 >> bi)))
                            goto bc_nobranch;
                    } else {
                        if (cpu->cr & (0x80000000 >> bi))
                            goto bc_nobranch;
                    }
                }
                if (op & 2)
                    cpu->pc = (int32_t)(int16_t)(op & 0xfffc);
                else
                    cpu->pc += (int32_t)(int16_t)(op & 0xfffc) - 4;
bc_nobranch:
                break;
            }

            case 18: {    // b
                int32_t target = op & 0x03fffffc;
                if (target & 0x02000000)
                    target |= 0xfc000000;
                if (op & 1)
                    cpu->lr = cpu->pc;
                if (op & 2)
                    cpu->pc = target;
                else
                    cpu->pc += target - 4;
                break;
            }

            case 19:
                emul19(cpu, op);
                break;

            case 20: {    // rlwimi
                uint32_t rs = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                uint32_t sh = (op >> 11) & 0x1f;
                uint32_t mask = mbme2mask(op);
                cpu->r[ra] = (((cpu->r[rs] << sh) | (cpu->r[rs] >> (32-sh))) & mask) | (cpu->r[ra] & ~mask);
                if (op & 1)
                    record(cpu, cpu->r[ra]);
                break;
            }

            case 21: {    // rlwinm
                uint32_t rs = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                uint32_t sh = (op >> 11) & 0x1f;
                cpu->r[ra] = ((cpu->r[rs] << sh) | (cpu->r[rs] >> (32-sh))) & mbme2mask(op);
                if (op & 1)
                    record(cpu, cpu->r[ra]);
                break;
            }

            case 23: {    // rlwnm
                uint32_t rs = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                uint32_t sh = cpu->r[(op >> 11) & 0x1f] & 0x1f;
                cpu->r[ra] = ((cpu->r[rs] << sh) | (cpu->r[rs] >> (32-sh))) & mbme2mask(op);
                if (op & 1)
                    record(cpu, cpu->r[ra]);
                break;
            }

            case 24: {    // ori
                uint32_t rs = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[ra] = cpu->r[rs] | (op & 0xffff);
                break;
            }

            case 25: {    // oris
                uint32_t rs = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[ra] = cpu->r[rs] | (op << 16);
                break;
            }

            case 26: {    // xori
                uint32_t rs = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[ra] = cpu->r[rs] ^ (op & 0xffff);
                break;
            }

            case 27: {    // xoris
                uint32_t rs = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[ra] = cpu->r[rs] ^ (op << 16);
                break;
            }

            case 28: {    // andi.
                uint32_t rs = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[ra] = cpu->r[rs] & (op & 0xffff);
                record(cpu, cpu->r[ra]);
                break;
            }

            case 29: {    // andis.
                uint32_t rs = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[ra] = cpu->r[rs] & (op << 16);
                record(cpu, cpu->r[ra]);
                break;
            }

            case 31:
                emul31(cpu, op);
                break;

            case 32: {    // lwz
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[rd] = ReadMacInt32(cpu, (int16_t)(op & 0xffff) + (ra ? cpu->r[ra] : 0));
                break;
            }

            case 33: {    // lwzu
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[ra] += (int16_t)(op & 0xffff);
                cpu->r[rd] = ReadMacInt32(cpu, cpu->r[ra]);
                break;
            }

            case 34: {    // lbz
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[rd] = ReadMacInt8(cpu, (int16_t)(op & 0xffff) + (ra ? cpu->r[ra] : 0));
                break;
            }

            case 35: {    // lbzu
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[ra] += (int16_t)(op & 0xffff);
                cpu->r[rd] = ReadMacInt8(cpu, cpu->r[ra]);
                break;
            }

            case 36: {    // stw
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                WriteMacInt32(cpu, (int16_t)(op & 0xffff) + (ra ? cpu->r[ra] : 0), cpu->r[rd]);
                break;
            }

            case 37: {    // stwu
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[ra] += (int16_t)(op & 0xffff);
                WriteMacInt32(cpu, cpu->r[ra], cpu->r[rd]);
                break;
            }

            case 38: {    // stb
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                WriteMacInt8(cpu, (int16_t)(op & 0xffff) + (ra ? cpu->r[ra] : 0), cpu->r[rd]);
                break;
            }

            case 39: {    // stbu
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[ra] += (int16_t)(op & 0xffff);
                WriteMacInt8(cpu, cpu->r[ra], cpu->r[rd]);
                break;
            }

            case 40: {    // lhz
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[rd] = ReadMacInt16(cpu, (int16_t)(op & 0xffff) + (ra ? cpu->r[ra] : 0));
                break;
            }

            case 41: {    // lhzu
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[ra] += (int16_t)(op & 0xffff);
                cpu->r[rd] = ReadMacInt16(cpu, cpu->r[ra]);
                break;
            }

            case 42: {    // lha
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[rd] = (int32_t)(int16_t)ReadMacInt16(cpu, (int16_t)(op & 0xffff) + (ra ? cpu->r[ra] : 0));
                break;
            }

            case 43: {    // lhau
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[ra] += (int16_t)(op & 0xffff);
                cpu->r[rd] = (int32_t)(int16_t)ReadMacInt16(cpu, cpu->r[ra]);
                break;
            }

            case 44: {    // sth
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                WriteMacInt16(cpu, (int16_t)(op & 0xffff) + (ra ? cpu->r[ra] : 0), cpu->r[rd]);
                break;
            }

            case 45: {    // sthu
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->r[ra] += (int16_t)(op & 0xffff);
                WriteMacInt16(cpu, cpu->r[ra], cpu->r[rd]);
                break;
            }

            case 46: {    // lmw
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                uint32_t addr = (int16_t)(op & 0xffff) + (ra ? cpu->r[ra] : 0);
                while (rd <= 31) {
                    cpu->r[rd] = ReadMacInt32(cpu, addr);
                    rd++;
                    addr += 4;
                }
                break;
            }

            case 47: {    // stmw
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                uint32_t addr = (int16_t)(op & 0xffff) + (ra ? cpu->r[ra] : 0);
                while (rd <= 31) {
                    WriteMacInt32(cpu, addr, cpu->r[rd]);
                    rd++;
                    addr += 4;
                }
                break;
            }

            case 50: {    // lfd
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                cpu->fr[rd] = (double)ReadMacInt64(cpu, (int16_t)(op & 0xffff) + (ra ? cpu->r[ra] : 0));
                break;
            }

            case 54: {    // stfd
                uint32_t rd = (op >> 21) & 0x1f;
                uint32_t ra = (op >> 16) & 0x1f;
                WriteMacInt64(cpu, (int16_t)(op & 0xffff) + (ra ? cpu->r[ra] : 0), (uint64_t)cpu->fr[rd]);
                break;
            }

            case 59:
                emul59(cpu, op);
                break;

            case 63:
                emul63(cpu, op);
                break;

            default:
                fprintf(stderr, "emul_ppc: Illegal opcode %08x at %08x (prim %d)\n", op, cpu->pc-4, primop);
                cpu->fault = 1;
                break;
        }

        if (step)
            return !cpu->fault;
    }

    return 0;
}

void emul_ppc_init(emul_ppc_state *cpu)
{
    // Init field2mask
    for (int i=0; i<256; i++) {
        uint32_t mask = 0;
        if (i & 0x01) mask |= 0x0000000f;
        if (i & 0x02) mask |= 0x000000f0;
        if (i & 0x04) mask |= 0x00000f00;
        if (i & 0x08) mask |= 0x0000f000;
        if (i & 0x10) mask |= 0x000f0000;
        if (i & 0x20) mask |= 0x00f00000;
        if (i & 0x40) mask |= 0x0f000000;
        if (i & 0x80) mask |= 0xf0000000;
        field2mask[i] = mask;
    }

    memset(cpu->r, 0, sizeof(cpu->r));
    memset(cpu->fr, 0, sizeof(cpu->r));
    cpu->lr = cpu->ctr = 0;
    cpu->cr = cpu->xer = 0;
    cpu->fpscr = 0;
    cpu->fault = 0;
}
