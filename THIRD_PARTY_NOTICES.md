<!-- Added by NZBGet Lite contributors, 2026-08-28. -->
# Third-party notices

This file records licensing information for code retained in NZBGet Lite or
linked into its executable. The corresponding source is available from this
repository and the pinned sources identified below.

## GNU regex fallback

`lib/regex/regex.c` is from the GNU C Library and is licensed under the GNU
Lesser General Public License, version 2.1 or (at your option) any later
version. Its copyright and license notice remains intact in that file. See
[COPYING.LESSER](COPYING.LESSER). Linux builds use the system regex
implementation; the fallback remains in the source tree.

## par2cmdline-turbo

NZBGet Lite statically links
[nzbgetcom/par2cmdline-turbo](https://github.com/nzbgetcom/par2cmdline-turbo/tree/v1.4.0-20260803)
at tag `v1.4.0-20260803`. It is distributed under the GNU General Public
License, version 2 or (at your option) any later version. See [COPYING](COPYING).

## rapidyenc

NZBGet Lite statically links
[nzbgetcom/rapidyenc](https://github.com/nzbgetcom/rapidyenc/tree/v1.1.1-20260821)
at tag `v1.1.1-20260821`. rapidyenc is dedicated to the public domain or
offered under CC0 where public-domain dedication is not recognized.

The optional Apache-2.0 crcutil implementation is disabled in NZBGet Lite to
avoid incorporating it into the executable. rapidyenc retains CRC folding
code derived from zlib-ng under the zlib license, with copyright held by
Jean-loup Gailly, Mark Adler, Intel Corporation, and the contributors named in
that source. The complete zlib notice and corresponding derived source remain
in the pinned rapidyenc source linked above.
