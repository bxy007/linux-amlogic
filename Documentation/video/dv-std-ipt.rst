Standard Dolby Vision direct target IPT experiment
=================================================

``dv_std_ipt`` is a default-off SC2 standard-DV experiment. It selects the
direct-output-IPT recipe previously tested as mark14. It is separate from
``dv_std_ycc``; an explicitly requested YCC experiment takes precedence.

Control and scope
-----------------

The parameters are in ``/sys/module/amdolby_vision/parameters/``::

  echo Y > dv_std_ipt
  cat dv_std_ipt_status
  echo N > dv_std_ipt

Only SC2 with the new STB algorithm, a Dolby source described by valid YCC/PQ
source DM, standard RGB8-tunnel output and PQ graphics can enter this route.
LLDV, DV-as-HDR10, source IPT/P5, non-PQ graphics, debug overrides and unsupported
matrices retain the original route. No LLDV/HDR10 optimization is implied.

``enabled`` is the request, ``active`` is the last submitted route, and
``request`` / ``handled`` track asynchronous requests. The status also gives a
fallback reason, error and source epoch/serial. A submission is not an
independent hardware-VSYNC acknowledgement. ``gui_qualified=0`` deliberately
remains set: geometric rebase is not full graphics, alpha or optical approval.

Parameter construction
----------------------

The builder consumes original algorithm DM1/DM2/DM3 plus the corresponding
source and output ETSI DM. It computes, without kernel floating point::

  video A2B = row_normalize(inverse(output M2) * source M2), Q15
  target C2D = inverse(output M1), Q12
  graphics A2B = video A2B * inverse(native video A2B) * native graphics A2B

Rows are quantized with constrained nearest row sums. Video row normalization
is an explicit white-axis approximation, limited to 1/1024 relative gain; it is
not an exact equivalence for arbitrary RPU white points. Graphics keeps its own
derived white gain instead of normalizing it. Matrices must have supported
conditioning, positive linear input mixing and representable signed16 results.
Intermediate arithmetic is bounded/checked signed64. Invalid data and overflow
reject the complete bundle rather than using an earlier source's matrix.

The original video and graphics post-PQ bases must be equal. Both planes use
the target C2D. Core1 CSC remains enabled; video and graphics CVMs are bypassed.
Core3 uses mode1 with scale4096 and offsets0/2048/2048. Composer, source CSC,
input normalization, transfer selection, LUTs, output DM and creative metadata
are retained. There is no CPU pixel conversion or decoded-frame copy.

The original algorithm software caches remain original. A basis transition
forces all three cores together, including restoration. Retained-picture/OSD
updates use the matching source epoch; seek, stop, engine-off and mode changes
invalidate it. YCC selection from the same submission is also considered when
choosing IPT, so a concurrent YCC-disable cannot combine YCC metadata with IPT
matrices. Requests blocked by the existing core-setting debug flag are retried.

Known experimental limits
-------------------------

This is not a drop-in equivalence to native graphics CVM policy: its luminance
and saturation mapping are bypassed as part of changing the shared blend basis.
The renderer's PQ colours and graphics peak controls need their own checks.
Premultiplied texture alpha handed to nonlinear colour processing is an
independent issue also observed in the original standard-DV route. Do not call
zero-alpha/no-foreground measurements successful graphics validation.

Graphics A2B allows a derived row gain up to1025/1024. Linear RGB components
at most1024/1025 remain within its unit input domain; near10000-nit HDR graphics,
negative RGB, superwhite, arbitrary source matrices, spatial edges and all
receiver behaviour have not acquired full-domain qualification. The switch
must remain experimental/default-off until those boundaries are addressed.

Host arithmetic checks
----------------------

From the kernel tree::

  cc -std=gnu11 -O1 -g -Wall -Wextra -Werror \
     -fsanitize=address,undefined tools/testing/dv_std_ipt_test.c \
     -o /tmp/dv_std_ipt_test
  ASAN_OPTIONS=detect_leaks=0 /tmp/dv_std_ipt_test

The host test checks constrained rational quantization, real matrix fixtures,
failure atomicity, malformed metadata and bounded randomized inputs. It does
not simulate hardware PQ interpolation, blending, resampling or HDMI output.
