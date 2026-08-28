# NZBGet Lite modification notice

NZBGet Lite is a modified version of
[NZBGet](https://github.com/nzbgetcom/nzbget). The fork began from upstream
commit `0832faf0b741453bb27440c1eab1e035c4376520` and was substantially modified
on 28 August 2026.

The principal changes are:

- removal of the web user interface and static-file server;
- removal of extension loading, execution, APIs, and documentation;
- removal of non-Linux source, build, packaging, and CI infrastructure;
- removal of configuration-management RPC endpoints;
- removal of credentials embedded in URL paths;
- replacement of upstream container packaging with a minimal, non-root,
  multi-architecture container; and
- reduction of the documentation and CI to the supported API-only Linux scope.

The complete history and exact changes are available in this repository. This
fork is not an official NZBGet release and is not endorsed by the upstream
NZBGet project.

NZBGet Lite is distributed under the GNU General Public License, version 2 or
(at your option) any later version. See [COPYING](COPYING). It comes with no
warranty, to the extent permitted by law.
