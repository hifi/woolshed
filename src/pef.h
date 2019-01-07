/*
 *  Copyright (C) 2019 Toni Spets <toni.spets@iki.fi>
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
 *  along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#define PEF_TAG1        0x4A6F7921 // Joy!
#define PEF_TAG2        0x70656666 // peff
#define PEF_ARCH_PPC    0x70777063 // pwpc

#define PEF_SEC_CODE        0
#define PEF_SEC_UNPACKED    1
#define PEF_SEC_PATTERN     2
#define PEF_SEC_CONSTANT    3
#define PEF_SEC_LOADER      4
#define PEF_SEC_DEBUG       5
#define PEF_SEC_EXECUTABLE  6

typedef union {
    uint32_t i;
    char s[4];
} OSType;

#pragma pack(push,1)

typedef struct {
    OSType tag1;
    OSType tag2;
    OSType architecture;
    uint32_t formatVersion;
    uint32_t dateTimeStamp;
    uint32_t oldDefVersion;
    uint32_t oldImpVersion;
    uint32_t currentVersion;
    uint16_t sectionCount;
    uint16_t instSectionCount;
    uint32_t reservedA;
} PEFContainerHeader;

typedef struct {
    int32_t nameOffset;
    uint32_t defaultAddress;
    uint32_t totalSize;
    uint32_t unpackedSize;
    uint32_t packedSize;
    uint32_t containerOffset;
    uint8_t sectionKind;
    uint8_t shareKind;
    uint8_t alignment;
    uint8_t reservedA;
} PEFSectionHeader;

typedef struct {
    int32_t mainSection;
    uint32_t mainOffset;
    int32_t initSection;
    uint32_t initOffset;
    int32_t termSection;
    uint32_t termOffset;
    uint32_t importedLibraryCount;
    uint32_t totalImportedSymbolCount;
    uint32_t relocSectionCount;
    uint32_t relocInstrOffset;
    uint32_t loaderStringsOffset;
    uint32_t exportHashOffset;
    uint32_t exportHashTablePower;
    uint32_t exportedSymbolCount;
} PEFLoaderInfoHeader;

typedef struct {
    uint32_t nameOffset;
    uint32_t oldImpVersion;
    uint32_t currentVersion;
    uint32_t importedSymbolCount;
    uint32_t firstImportedSymbol;
    uint8_t options;
    uint8_t reservedA;
    uint16_t reservedB;
} PEFImportedLibrary;

typedef union {
    struct {
        uint32_t offset:24;
        uint8_t name:4;
        uint8_t flags:4;
    } __attribute__((packed)) s;
    uint32_t i;
} PEFSymbolTableEntry;

typedef struct {
    uint16_t sectionIndex;
    uint16_t reservedA;
    uint32_t relocCount;
    uint32_t firstRelocOffset;
} PEFLoaderRelocationHeader;

enum {

   kPEFRelocBySectDWithSkip= 0x00,/* binary: 00xxxxx */

   kPEFRelocBySectC     = 0x20,  /* binary: 0100000 */
   kPEFRelocBySectD     = 0x21,  /* binary: 0100001 */
   kPEFRelocTVector12   = 0x22,  /* binary: 0100010 */
   kPEFRelocTVector8    = 0x23,  /* binary: 0100011 */
   kPEFRelocVTable8     = 0x24,  /* binary: 0100100 */
   kPEFRelocImportRun   = 0x25,  /* binary: 0100101 */

   kPEFRelocSmByImport  = 0x30,  /* binary: 0110000 */
   kPEFRelocSmSetSectC  = 0x31,  /* binary: 0110001 */
   kPEFRelocSmSetSectD  = 0x32,  /* binary: 0110010 */
   kPEFRelocSmBySection = 0x33,  /* binary: 0110011 */

   kPEFRelocIncrPosition= 0x40,  /* binary: 1000xxx */
   kPEFRelocSmRepeat    = 0x48,  /* binary: 1001xxx */

   kPEFRelocSetPosition = 0x50,  /* binary: 101000x */
   kPEFRelocLgByImport  = 0x52,  /* binary: 101001x */
   kPEFRelocLgRepeat    = 0x58,  /* binary: 101100x */
   kPEFRelocLgSetOrBySection= 0x5A,/* binary: 101101x */

};

typedef struct {
    PEFContainerHeader *container;
    PEFSectionHeader *sections;
    PEFLoaderInfoHeader *loader;
    PEFImportedLibrary *libraries;
    PEFSymbolTableEntry *symbols;
    PEFLoaderRelocationHeader *relocSections;
    uint16_t *relocations;
} PEFImage;

#define PEFLoaderString(image, offset) (char *)((uint8_t *)image.loader + image.loader->loaderStringsOffset + offset)
#define PEFLoaderOffset(image, offset) (void *)((uint8_t *)(image)->loader + offset)
#define PEFContainerOffset(image, offset) (void *)((uint8_t *)(image)->container + offset)

void pef_init(PEFImage *pef, void *src, uint32_t size);
void pef_load_section(PEFImage *pef, uint8_t index, void *ram, uint32_t ram_size);

#pragma pack(pop)
