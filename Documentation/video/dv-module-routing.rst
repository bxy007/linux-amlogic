Dolby STB module refresh-rate routing
====================================

The default dv_new_blob_max_hz is 60 (previously 50). The existing
routing conditions remain unchanged: Dolby input, FORMAT_DOVI algorithm
output, and xbmc_dv_vp == 0. The threshold uses the display refresh rate
integer quotient, not the source frame rate. Thus 59.94 and 60 Hz qualify.
Missing-module fallback and the writable threshold parameter are unchanged.
The DV-as-HDR10 path also qualifies when it uses the Dolby LL algorithm
before changing the final output encoding and HDMI signalling.

On 2026-11-03 the user requested the higher default after A/B playback on
an S905X4-j: ten minutes per module of 59.94 fps MEL, then a complete
74.6-second 60 fps FEL clip per module. The FEL stable interval produced
59.985 fps with the new module and 59.999 fps with the old module, with no
additional drops or BL/EL decoder errors. All 4475 RPUs in that clip were
CMv2.9; these tests do not establish universal CMv4.0 performance or explain
why the original downstream implementation chose 50 Hz.

For diagnosis, writing 50 to the existing parameter restores the previous
routing threshold. This is a runtime override and resets on reboot unless
the system administrator explicitly persists it.
