DV-as-HDR10 output encoding
==========================

R10's DV-as-HDR10 modes use the STB low-latency Dolby algorithm with a
VSVDB-defined RGB output basis, while advertising BT.2020 HDR10 over HDMI.
The output matrix correction converts that basis to BT.2020/D65. It is
on by default, and applies only to LL-as-HDR10 Core3 submissions. Ordinary
LLDV, standard DV, native HDR10 and SDR bypass do not opt into this path.
It does not select an HDMI format, alter VSVDB, or replace HDR metadata.

For each changed effective VSVDB, the driver computes::

    A = inverse(RGB_to_XYZ_BT2020_D65) * RGB_to_XYZ_VSVDB_D65
    B2A_output = A * B2A_algorithm

The effective block is the one submitted to the algorithm, including any
injection or runtime target adjustments; it is not the Kodi settings string.
V0, both V1 lengths, and V2 (including extended V2 blocks) are decoded with
length/OUI/version checks. V1 short RY uses all three low bits, matching the
STB target parser. The STB LL output basis uses D65. Matrix construction uses
Q12 chromaticities, Q20 coefficients and bounded signed 64-bit arithmetic;
there is no kernel floating point or per-pixel CPU processing. Recalculation
occurs only when the block/length changes, including after invalid input.

The original algorithm structures remain intact. Only a local Core3 register
copy is transformed, so repeated submissions do not compound the correction.
B2A is submitted on each update even if the original algorithm matrix is
unchanged. Disabling the correction restores the original matrix at the next
submission. Existing mute handling takes precedence.

Unknown/malformed blocks, non-physical or nearly collinear primaries, a D65
white outside the RGB triangle, unsupported B2A scaling, and coefficient
overflow preserve the original complete matrix. A rejected new block never
reuses the preceding target's matrix. There are no display-model constants.

Runtime controls
----------------

Under /sys/module/amdolby_vision/parameters/:

``dv_hdr10_encoding``
    Root-writable Y/N switch, default Y. Takes effect at a Core3 update.
    The value is not persisted in Kodi; reboot returns to the compiled default.

``dv_hdr10_encoding_config``
    Read-only cached effective VSVDB (32 bytes in hex) and nine signed Q20
    coefficients. No userspace provisioning is needed or accepted.

``dv_hdr10_encoding_vsvdb``
    Read-only VSVDB from the most recent Core3 submission.

``dv_hdr10_encoding_status``
    Result of the last Core3 submission: 0 bypassed, 1 applied, -1 unsupported
    or invalid target, -2 coefficient/scale overflow. This is not a playback
    activity indicator; when Core3 is not being updated it may retain the
    previous submission's result. Check playback/output state as well.

The existing RGB stimulus and mastering-primary diagnostics remain off by
default. They are not needed for normal correction or target rebinding.

Validation and limits
---------------------

Run the production helpers under host UBSan with an independent NumPy
floating-point reference::

    python3 tools/testing/selftests/amlogic/dv_hdr10_encoding_test.py

Tests cover VSVDB decoding, bounded arithmetic, malformed input, target/cache
transitions, disabled/non-HDR10 scope and preservation of original matrices.

Production device checks verified actual 4K60 YUV422/12-bit HDR10 and
1080p60 YUV444/12-bit HDR10, automatic display/BT.709/BT.2020 target changes,
exact register restoration across off/on, and ordinary LLDV bypass. Read-back
B2A matched independent integer composition. These format/register checks
are separate from the preceding optical measurements.

The preceding candidate showed the expected chromaticity direction on mixed
color patches and was accepted by the user in a real-film comparison. This
is not a claim of full Dolby colorimetric conformance: independent HDR10
references retained a roughly six-percent luminance difference, also present
in the uncorrected path. Effective bit depth was not established.

Combining matrices does not insert a native-RGB clipping stage between them:
A*clip(RGB) and clip(A*RGB) are not generally equal. Artificial negative-channel
stimuli favored the latter chromaticity in some cases, but did not establish
that real mapped film pixels reach the same out-of-gamut values. Target-gamut
clipping preservation remains unverified. Keep the switch available for
comparison and regression isolation; do not describe the correction as proof
that every aspect of the original target mapping is preserved.
