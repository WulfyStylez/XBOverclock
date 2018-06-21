XBOverclock v1.0
by WulfyStylez, 2018

This is a tool for adjusting clocks on original XBOX hardware. It modifies system initialization fields and bytecode ("X-Codes") using heuristics to allow for reclocking of a few parts of the XBOX system.
Your BIOS must not check the hash of the "X-Codes" or you will FRAG upon boot. Other than this, XBOverclock should work with any and all XBOX BIOSes. (Support for prototype EVT/DVT1-3 boards is even implemented! This requires rebuilding with defines adjusted.)

Currently, NVCLK (NV2A GPU core) and FSB (system front-side bus, used by CPU, SB, ...) clocks can be adjusted.

Author's suggestions regarding overclocking: 
- FSB overclocks require quite a lot of hardware to run out of spec, and are limited by this. Bad FSB overclocks regularly result in hangs with audio looping.
- NVCLK overclocks are naturally less restricted. Theoretically different chips should reach different peak clocks, but this is currently speculation.
- Clocks are rounded down to the nearest clock divider value. Try adding 1-2MHz if actual clocks differ significantly from input.
- This was only tested on a single 1.0 console (MFD 12/2001). Stable clocks achieved were 259MHz NVCLK, 141MHz FSB.
- If these numbers turn out to be consistent between many consoles, additional boot parameters may need to be tweaked in order to push clocks farther. Let me know if this turns out to be the case!
- Have fun, and please be responsible!!
