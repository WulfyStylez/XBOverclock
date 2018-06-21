#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <time.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <errno.h>

#include "utils.h"

//#define BETA_BASECLK // pre-DVT4 hardware uses a 13.5MHz base clock
//#define NV2A_A1   // Early A1 revision of NV2A, DVT2 (and maybe DVT1/EVT?) use this

#ifndef BETA_BASECLK
const float base_clock = 16.66; // base clock on DVT4 and newer
#else
const float base_clock = 13.5;
#endif

char *bios_data = NULL;
int bios_size = 0;

// TODO: rewrite this, we should be able to do /1, /2, /4, /8
void calc_clock_params(int clk, int *n, int *m)
{
    int work1, work2, work4, divisor;
    
    work1 = clk / base_clock;
    
    work2 = (clk*2) / base_clock;
    
    work4 = (clk*4) / base_clock;
    
    if(work2 * 2 != work4) {
        *n = work4;
        *m = 4;
    }
    else if(work1 * 2 != work2) {
        *n = work2;
        *m = 2;
    }
    else {
        *n = work1;
        *m = 1;
    }
}

// find and patch FSB dividers
void patch_fsb(int fsb_target)
{
    printf("Patching FSB to %dMHz...\n", fsb_target);
    
    int n, m;
    calc_clock_params(fsb_target, &n, &m);
    float clk = base_clock * n / m;
    printf("Actual FSB clock: %.01fMHz\n", clk);
    printf("CPU clock (stock CPU): %.01fMHz\n", clk * 5.5);
    
    // get MCP init table offset so we can tweak FSB multipliers
    int table_offset = getle32(&bios_data[4]) & 0x00FFFFFF;
    
    // n and m for each fsb mode
    for(int i = 0; i < 4; i++)
    {
        bios_data[table_offset+0x08+i] = m;
        bios_data[table_offset+0x0C+i] = n;
    }
    // "ROM" mode clock
    bios_data[table_offset+0x10] = m;
    bios_data[table_offset+0x11] = n;
    
    // 0x00230801, default MPLL coefficient (MEM_PDIV = 2, FSB_PDIV = 3, NDIV = 8, MDIV = 1)
    // 0x01230801, "override" version of above, patch this too
    // look for 0x8000036C (register write offset), then the shared 3 bottom bytes of the above words
    u32 mpll_data[2];
    putle32(&mpll_data[0], 0x8000036C);
    putle32(&mpll_data[1], 0x00230801);
    int found = 0;
    for(int i = 0; i < bios_size - sizeof(mpll_data) && found < 2; i++)
    {
        // don't compare the top byte, this heuristic is enough to catch both things we need to patch
        if(!memcmp(&bios_data[i], mpll_data, 7))
        {
            printf("Patching MPLL coefficient pattern at offset 0x%X\n", i);
            found++;
            
            bios_data[i+4] = m;
            bios_data[i+5] = n;
        }
    }
    if(found < 2)
        printf("WARNING: Failed to find 2 instances of the MPLL coefficient pattern as expected.\n");
}

// find and patch NVCLK divider
void patch_nvclk(int nvclk_target)
{
    printf("Patching NVCLK to %dMHz...\n", nvclk_target);
    
    // NVCLK is packed in the following way:
    // 0x000PNNMM, where clock in MHz = (N * (base_clock) / M) >> P;
    // we always use 1 for P
    
    int n, m;
    calc_clock_params(nvclk_target*2, &n, &m);
    printf("Actual NVCLK: %.01fMHz\n", (base_clock * n / m)/2);
    
    /*
     * find a pattern of 32-bit {pll coefficient register address, register setting}
     *
     * bytecode sets one clock profile for >= A2 revision NV2A, and another for A1 revision.
     * NOTE: NV2A A1 is only in DVT2 (and lower?) kits (?!), differing DVT3 clocks (early vs late w/ fansink)
     *   seem to have been handled with completely separate bootloaders.
     */
    u32 nvclk_data[2];
    bool found = false;
    putle32(&nvclk_data[0], 0x0F680500);
#ifndef NV2A_A1
    putle32(&nvclk_data[1], 0x00011C01);  // NV2A A2+ (DVT4+ default: ~266MHz at 16.66MHz base)
#else
    putle32(&nvclk_data[1], 0x00011701);  // NV2A A1 (155MHz at 13.5MHz base)
#endif
    for(int i = 0; i < bios_size - sizeof(nvclk_data); i++)
    {
        if(!memcmp(&bios_data[i], &nvclk_data, sizeof(nvclk_data)))
        {
            printf("Patching NVCLK setting pattern at offset 0x%X\n", i);
            
            bios_data[i+4] = m;
            bios_data[i+5] = n;
            //bios_data[i+6] = p;
            found = true;
            break;
        }
    }
    if(found == false)
        printf("WARNING: Failed to find an instance of the NVCLK coefficient pattern as expected.\n");
}

int main(int argc, char* argv[])
{
    u32 i = 0, j = 0;
    int ret = 0;
    
    // disable printf buffering for now
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    
    printf("XBOverclock v1.0\n"
           "By WulfyStylez 2k18\n"
           "Built %s, %s\n\n", __DATE__, __TIME__);
    
    // parse and prepare input
    if(argc < 4)
    {
        printf("Usage: XBOverclock [args] [xboxbios.rom]\n"
               "Arguments:\n"
               "  -nv  [num]        set NVCLK (NV2A GPU frequency) in MHz (default: 233)\n"
               "  -fsb [num]        set front-side bus frequency in MHz (default: 133)\n"
               "  -o   [filename]   write to a separate output file\n");
        return -1;
    }
    
    char* fn_bios = argv[argc-1];
    char* fn_out = fn_bios;
    
    // parse middle args
    int nvclk_target = 0;
    int fsb_target = 0;
    for(i = 1; i < argc-1; i++)
    {
        if(!strcmp("-nv", argv[i])) {
            i++;
            nvclk_target = strtoul(argv[i], NULL, 0);
        }
        else if(!strcmp("-fsb", argv[i])) {
            i++;
            fsb_target = strtoul(argv[i], NULL, 0);
        }
        else if(!strcmp("-o", argv[i])) {
            i++;
            fn_out = argv[i];
        }
        else {
            printf("Unrecognized argument %s !\n", argv[i]);
            return -1;
        }
    }
    
    // read the BIOS
    FILE* f_bios = fopen(fn_bios, "rb");
    if(!f_bios)
    {
        printf("Failed to open %s for reading!\n", fn_bios);
        return -1;
    }
    
    if(fseek(f_bios, 0, SEEK_END))
    {
        printf("Failed to check size of BIOS file!\n");
        return -1;
    }
    
    bios_size = ftell(f_bios);
    rewind(f_bios);
    
    bios_data = malloc(bios_size);
    if(!bios_data || fread(bios_data, 1, bios_size, f_bios) != bios_size)
    {
        printf("Failed to read BIOS!\n");
        return -1;
    }
    fclose(f_bios);
    
    if(nvclk_target != 0)
        patch_nvclk(nvclk_target);
    
    if(fsb_target != 0)
        patch_fsb(fsb_target);
    
    
    FILE* f_out = fopen(fn_out, "wb");
    if(!f_out)
    {
        printf("Failed to create %s!\n", fn_out);
        return -1;
    }
        
    if(fwrite(bios_data, 1, bios_size, f_out) != bios_size)
        printf("Failed to write data to %s...\n", fn_out);
    
    fclose(f_out);
    free(bios_data);
	return 0;
}
