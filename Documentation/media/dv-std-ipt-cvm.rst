Experimental standard-DV direct-IPT graphics mapping
==================================================

Default and scope
-----------------

The ``amdolby_vision.dv_std_ipt`` runtime parameter remains disabled by
default. Loading the module or starting playback does not enable it. There
is no automatic activation policy. The normal output path remains selected
unless the administrator explicitly enables the experiment.

The experiment applies to SC2, the new STB algorithm, standard Dolby Vision
RGB8 tunnel output, and PQ RGB graphics. It does not select the LLDV, HDR10,
Profile 5, or graphics-priority paths. The separate YCbCr experiment takes
precedence when requested.

Coupled graphics and metadata policy
-----------------------------------

Direct-IPT video and graphics matrices describe the same transmitted colour
basis. The graphics mapper uses a bounded integer implementation of the old
STB 2.4 video-priority policy. Its input is the original frame's public
metadata and the GUI range supplied to the successful native control-path
call, rather than a previously modified output packet.

The mapper selects and interpolates applicable L2 trims, generates four
512-entry tables, and retains the current native PQ/G2L block. The complete
160-byte curve control is the cache key. Curve generation uses preallocated
workspace without floating point, allocation, or sleeping in frame handling.

A private output packet carries the matching source luminance range and L1
scene values. The compatibility limits depend on both source L254 and the
effective sink DM version. Active-area handling also couples positive L2
offset limits to the graphics control. The existing output colour matrices,
CoreELEC active-area policy, and remaining selected extension blocks are
preserved. Original algorithm settings and LUT caches are never overwritten.

Curve and metadata changes request Core2 and Core3 in the same submission.
The source epoch is checked again after generation. Any unsupported input
rejects the entire direct-IPT bundle, including its private metadata. In
particular, the old SDK's eight-byte L3 form must not be confused with the
standard six-byte CM4 L3 form; the latter uses the native path. This is not a
claim of complete CM4 graphics-trim support.

An actual-upload LUT mirror distinguishes private and native payloads from
the native algorithm cache. Leaving the route restores native matrices,
graphics mapping, LUTs, and metadata. A scene-refresh notification covers
paused-picture entry and restoration without modifying native settings.

Runtime controls
----------------

The parameters are intended for explicit diagnostic use::

  echo N > /sys/module/amdolby_vision/parameters/dv_std_ycc
  echo Y > /sys/module/amdolby_vision/parameters/dv_std_ipt
  cat /sys/module/amdolby_vision/parameters/dv_std_ipt_status

Restore the normal path with::

  echo N > /sys/module/amdolby_vision/parameters/dv_std_ipt

``enabled`` reports the request; ``active`` reports the submitted route.
``cvm`` reports whether its graphics mapping is active. The status includes
fallback reasons, generation/upload counts, generation time, and the last
input/output L1 triplets. At idle, ``enabled=1 active=0 reason=idle`` is normal.
An explicit runtime enable does not change the next boot's default.

The Kodi GUI-white slider changes rendered pixels; it is distinct from the
``dolby_vision_graphic_max`` environment supplied to the native algorithm.

Validation and limits
---------------------

Validation includes old-function differential checks for curve controls,
four-table generation, trim interpolation, metadata compatibility branches,
and sink-version decoding, with ASan and UBSan checks. The integrated code
has also been built in the release kernel and checked on an SC2 box for
actual L1 output, paused graphics-range changes, cache stability, native
restoration, and unsupported-format fallback.

``experimental=1 gui_qualified=0`` is intentional. These checks do not certify
optical accuracy, all scenes, or every transparency/blending condition.
