.. SPDX-License-Identifier: GPL-2.0

Optional controller privacy for LE reconnection
==============================================

This backport adds controller resolving-list management and enhanced LE
connection completion to the 4.9 host stack. It uses the upstream experimental
MGMT UUID ``15c0a148-c273-11ea-b3de-0242ac130004`` and the standard per-device
Device Privacy flag. Selection does not depend on peripheral names, vendor
names, address prefixes or a particular Bluetooth controller model.

The feature is disabled by default. A controller must advertise LL Privacy,
the required commands and a nonzero resolving-list capacity. Applications
should check the experimental features before enabling it. BlueZ 5.78 already
implements the necessary interfaces; no BlueZ source modification is required.
The corresponding BlueZ General settings are::

    Privacy = device
    KernelExperimental = 15c0a148-c273-11ea-b3de-0242ac130004

Preserve a stable host identity in native BlueZ storage. Normal SMP pairing
must distribute that same identity to the peer. Generating a new local IRK
does not repair an old bond whose peripheral knows a different host identity.
Never silently replace an existing identity or assume that possession of a
peer IRK proves that the peer received the host IRK.

Resolving-list entries are cached only after successful commands. Changed
IRKs replace old entries even when identity addresses are unchanged. If the
pending set exceeds the controller capacity, background scanning uses the
existing software path without a partially populated address filter.

Enhanced completion uses identity addresses for connection management and
actual link RPAs for SMP. The public/static-random host identity determines
the appropriate controller-generated RPA address type. Discovery and existing
connections continue to use the host stack's normal request ownership.

Some existing bonds require identity-address initiation. In Device Privacy
compatibility mode, a genuine establishment failure can alternate the next
attempt between private and identity initiation. Successful links retain the
observed choice for that connection-parameter record. This is a runtime hint,
not new persistent key material: a reboot may require probing an old bond
again. Rapid cancellations and authentication/key errors do not trigger it;
strict Network Privacy Mode and peripheral-role initiation are unchanged.

For identity initiation to a peer observed advertising an identity address,
controller address resolution is disabled during the attempt. If the peer
actually used an RPA, resolution stays enabled so its advertisements remain
resolvable. Neither case modifies peer keys or selects devices by name.

Local legacy LE advertising is supported while the feature is enabled.
Background list updates serialize: stop scanning, pause advertising, disable
address resolution, update resolving/accept lists, enable resolution, resume
advertising, start scanning. Each stage waits for controller completion without
holding the device lock. Independent restore requests also run after failure;
scanning then uses unfiltered software resolution rather than a partial filter.
Connections and discovery that acquire the radio between stages take priority.

Advertising retains software-generated RPAs from the persistent host IRK, so
it needs no dummy peer in the resolving list. Existing advertising instance
rotation/lifetimes are retained across short pauses. Active discovery keeps
its existing exclusive-radio policy and restores advertising afterwards.
Concurrent advertising with established LE connections follows the controller's
LE Supported States for central/peripheral roles; unsupported combinations
are deferred, not forced. Advertising is resumed after connection completion.
The legacy experimental UUID is retained for BlueZ 5.78 compatibility, not as
an advertising prohibition. This does not add extended advertising to 4.9.
Controller privacy is separate from firmware-specific wake-from-suspend.

The implementation follows Linux commits e1d572357599, 0eee35bdfa3b,
b31bc00bfe3a, d03c759e3919, 5c49bcce5c12 and cbbdfa6f3319, with the
enable-state correction from 1fb17dfc258f and the optional-index MGMT
dispatch dependency. Local adaptations include capacity preflight, replacing
changed keys, actual SMP address handling and legacy-bond initiation hints.

Advertising coordination follows ad383c2c65a5, adapted to 4.9 requests rather
than importing the modern hci_sync framework. The API remains opt-in; modern
per-peer Address Resolution flags require additional userspace integration.
