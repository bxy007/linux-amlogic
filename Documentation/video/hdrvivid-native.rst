Native HDR Vivid receiver output
===============================

HDR Vivid receiver adaptation uses the original decoded PQ/HLG BT.2020
video with per-frame VS-EMDS metadata. The receiver performs adaptation.
This differs from monitor adaptation, which requires transmitter-side
tone mapping and a monitor-mode VSIF. This implementation selects receiver adaptation
when EDID advertises rx_mode_support, including when monitor mode or HDR10+
is also advertised. It does not implement monitor-side tone mapping.

The implemented metadata syntax is system_start_code 1. The VS-EMDS
version is 1, following T/UWA 005.2-1-2022 sections 11.3.1 and 11.3.3.
The parser was also checked against T/UWA 005.1-2024 V1.3 metadata
syntax; this does not imply support for every later system_start_code.
The three-packet dataset preserves statistics, the first tone-mapping
parameter set, both splines of that set and saturation parameters.
The source transfer function and mastering luminance populate their
respective transport fields. Malformed/truncated CUVA metadata is rejected
before partial data can be submitted as a new dataset.

The HDMI driver allocates two coherent, 32-bit-addressable DMA buffers
during probe. The video callback fills the alternate buffer and publishes
its address before enabling three EMP packets. Allocation failure leaves
native output unavailable. No sleeping allocation or per-frame logging
is performed in the callback. Playback transitions disable the old EMP
transport through the existing packet lifecycle.

Releasing native metadata does not select the next desktop format. A null
metadata callback stops EMP and keeps the static declaration until an
explicit output-format callback arrives. Dolby Vision, HDR10 and HLG take
over through their existing callbacks. Only an explicit BT.709/SDR target
starts the SDR handoff: zero DRM is sent continuously before the existing
HDR worker retires it. Repeated reset/SDR callbacks are idempotent, avoiding
a disabled-DRM interval followed by a delayed re-enable. This does not force
the desktop to SDR independently of the configured output mode.

Receiver mode sends VS-EMDS only. A CUVA VSIF with monitor_mode_enable=0
was tried during diagnosis and could trigger a receiver's Vivid logo, but
this is not the receiver-mode transport specified by the system integration
guide. That experiment is not part of this implementation.

EMP also needs the ESM clock domain when HDCP is unused. The driver holds
a prepared/enabled framework reference for this clock during its lifetime,
so unused-clock cleanup cannot gate the DMA interface. It uses the named
hdcp22_tx_esm consumer clock where available. Legacy SC2 device trees omit
that consumer: the driver obtains it from the documented SC2 clock provider
binding, without a board-specific DTB replacement. Probe deferral is
propagated; an actual clock acquisition/enable failure disables native
capability. Earlier HDMI20 SoCs without this consumer retain their existing
register-based clock initialization. Resource cleanup disables the clock
before releasing its reference.

The hdr_cap sysfs output includes ``CUVA native RX: 1`` only when both a
usable transmitter and a compatible receiver capability are present.
Kodi uses this signal to prefer native Vivid over its optional statistics
to Dolby Vision conversion and VS10 mapping. With older kernels or a
receiver without this capability, the previous conversion policy remains.
No display model, resolution or media filename is used in that decision.

``/sys/class/amhdmitx/amhdmitx0/cuva_emds`` is a read-only diagnostic:
active flag, submission count and the last three 32-byte DMA packets.
This describes the driver's submitted payload, not an HDMI wire capture
or proof that the receiver rendered every field correctly.
