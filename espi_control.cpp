
#include <unistd.h>

#include <stdplus/fd/create.hpp>
#include <stdplus/fd/intf.hpp>
#include <stdplus/fd/managed.hpp>
#include <stdplus/fd/mmap.hpp>

#include <cassert>
#include <cstdint>
#include <cstdio>

/* Base address for Nuvoton's global control register space. */
#define NPCM730_GLOBAL_CTRL_BASE_ADDR 0xF0800000
/* Offset of the PDID register and expected PDID value. */
#define PDID_OFFSET 0x00
#define NPCM730_PDID 0x04A92750

/* Register width in bytes. */
#define REGISTER_WIDTH 4

/* Base address for Nuvoton's eSPI register space. */
#define NPCM730_ESPI_BASE_ADDR 0xF009F000
/*
 * Offset of the eSPI config (ESPICFG) register, along with host channel enable
 * mask and core channel enable mask.
 */
#define ESPICFG_OFFSET 0x4
#define ESPICFG_HOST_CHANNEL_ENABLE_MASK 0xF0
#define ESPICFG_CORE_CHANNEL_ENABLE_MASK 0x0F
/*
 * Offset of the host independence (ESPIHINDP) register and automatic ready bit
 * mask.
 */
#define ESPIHINDP_OFFSET 0x80
#define ESPI_AUTO_READY_MASK 0xF

namespace
{

using stdplus::fd::MMapAccess;
using stdplus::fd::MMapFlags;
using stdplus::fd::OpenAccess;
using stdplus::fd::OpenFlag;
using stdplus::fd::OpenFlags;
using stdplus::fd::ProtFlag;
using stdplus::fd::ProtFlags;

static void usage(char* name)
{
    std::fprintf(stderr, "Usage: %s [-d]\n", name);
    std::fprintf(stderr, "Enable or disable eSPI bus on NPCM730 BMC\n");
    std::fprintf(stderr, "This program will enable eSPI by default, unless "
                         "the -d option is used.\n");
    std::fprintf(stderr, "  -d   Disable eSPI\n");
}

/*
 * Get a pointer to the register at the given offset, within the provided
 * memory-mapped I/O space.
 */
static inline volatile uint32_t* getReg(stdplus::fd::MMap& map,
                                        size_t regOffset)
{
    uintptr_t regPtr = (uintptr_t)map.get().data() + regOffset;
    /* Make sure the register pointer is properly aligned. */
    assert((regPtr & ~(REGISTER_WIDTH - 1)) == regPtr);

    return (volatile uint32_t*)regPtr;
}

} // namespace

int main(int argc, char** argv)
{
    int opt;
    bool disable = false;
    while ((opt = getopt(argc, argv, "d")) != -1)
    {
        switch (opt)
        {
            case 'd':
                disable = true;
                break;
            default:
                usage(argv[0]);
                return -1;
        }
    }

    /*
     * We need to make sure this is running on a Nuvoton BMC. To do that, we'll
     * read the product identification (PDID) register.
     */

    /* Find the page that includes the Product ID register. */
    size_t pageSize = sysconf(_SC_PAGE_SIZE);
    size_t pageBase = NPCM730_GLOBAL_CTRL_BASE_ADDR / pageSize * pageSize;
    size_t pageOffset = NPCM730_GLOBAL_CTRL_BASE_ADDR - pageBase;
    size_t mapLength = pageOffset + PDID_OFFSET + REGISTER_WIDTH;

    stdplus::fd::ManagedFd&& fd = stdplus::fd::open(
        "/dev/mem", OpenFlags(OpenAccess::ReadWrite).set(OpenFlag::Sync));
    stdplus::fd::MMap pdidMap(fd, mapLength, ProtFlags().set(ProtFlag::Read),
                              MMapFlags(MMapAccess::Shared), pageBase);

    volatile uint32_t* const pdidReg = getReg(pdidMap,
                                              pageOffset + PDID_OFFSET);

    /* Read the PDID register to make sure we're running on a Nuvoton BMC. */
    if (*pdidReg != NPCM730_PDID)
    {
        std::fprintf(stderr, "Unexpected product ID %#x != %#x\n", *pdidReg,
                     NPCM730_PDID);
        return -1;
    }

    /*
     * Find the start of the page that includes the start of the eSPI register
     * space.
     */
    pageBase = NPCM730_ESPI_BASE_ADDR / pageSize * pageSize;
    pageOffset = NPCM730_ESPI_BASE_ADDR - pageBase;

    mapLength = pageOffset + ESPIHINDP_OFFSET + REGISTER_WIDTH;

    stdplus::fd::MMap espiMap(
        fd, mapLength, ProtFlags().set(ProtFlag::Read).set(ProtFlag::Write),
        MMapFlags(MMapAccess::Shared), pageBase);

    /* Read the ESPICFG register. */
    volatile uint32_t* const espicfgReg = getReg(espiMap,
                                                 pageOffset + ESPICFG_OFFSET);
    uint32_t espicfgValue = *espicfgReg;

    if (disable)
    {
        /*
         * Check if the automatic ready bits are set in the eSPI host
         * independence register (ESPIHINDP).
         */
        volatile uint32_t* const espihindpReg =
            getReg(espiMap, pageOffset + ESPIHINDP_OFFSET);
        uint32_t espihindpValue = *espihindpReg;
        if (espihindpValue & ESPI_AUTO_READY_MASK)
        {
            /*
             * If any of the automatic ready bits are set, we need to disable
             * them, using several steps:
             *   - Make sure the host channel enable and core channel bits are
             *     consistent, in the ESPICFG register, i.e. copy the host
             *     channel enable bits to the core channel enable bits.
             *   - Clear the automatic ready bits in ESPIHINDP.
             */
            uint32_t hostChannelEnableBits = espicfgValue &
                                             ESPICFG_HOST_CHANNEL_ENABLE_MASK;
            espicfgValue |= (hostChannelEnableBits >> 4);
            *espicfgReg = espicfgValue;

            espihindpValue &= ~ESPI_AUTO_READY_MASK;
            *espihindpReg = espihindpValue;
        }

        /* Now disable the core channel enable bits in ESPICFG. */
        espicfgValue &= ~ESPICFG_CORE_CHANNEL_ENABLE_MASK;
        *espicfgReg = espicfgValue;

        fprintf(stderr, "Disabled eSPI bus\n");
    }
    else
    {
        /* Enable eSPI by setting the core channel enable bits in ESPICFG. */
        espicfgValue |= ESPICFG_CORE_CHANNEL_ENABLE_MASK;
        *espicfgReg = espicfgValue;

        fprintf(stderr, "Enabled eSPI bus\n");
    }

    return 0;
}
