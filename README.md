# NZBGet Lite

NZBGet Lite is a Linux-only, API-only fork of
[NZBGet](https://github.com/nzbgetcom/nzbget). It keeps the downloader,
JSON-RPC/XML-RPC APIs, command-line client, and Linux daemon while removing the
bundled web UI, extension runtime, static-file server, non-Linux ports, and
platform packaging infrastructure. It is intended to be built, distributed,
and run exclusively as a container.

This is a substantially modified, unofficial fork and is not endorsed by the
upstream NZBGet project. See [MODIFICATIONS.md](MODIFICATIONS.md) for the base
revision, modification date, and a summary of the changes.

There is intentionally no browser interface. Non-RPC HTTP routes return
`404 Not Found`.

## Run the container

The published image is `ghcr.io/mergemarauder/nzbget-lite:latest`. It supports
64-bit x86 and 64-bit ARM Linux hosts, including servers, NAS devices, and
64-bit Raspberry Pi systems. Use Docker, Podman, or a Kubernetes-compatible
container runtime.

Docker:

```sh
docker run -d --name nzbget-lite \
  --restart unless-stopped \
  -p 6789:6789 \
  -v nzbget-config:/config \
  -v nzbget-downloads:/downloads \
  ghcr.io/mergemarauder/nzbget-lite:latest
```

Podman uses the same arguments:

```sh
podman run -d --name nzbget-lite \
  -p 6789:6789 \
  -v nzbget-config:/config \
  -v nzbget-downloads:/downloads \
  ghcr.io/mergemarauder/nzbget-lite:latest
```

The process runs as numeric UID `65532` in group `0`, requires no Linux
capabilities, and writes only to `/config` and `/downloads`. For bind mounts,
make both directories writable by that identity. Kubernetes may assign a
different UID; set an appropriate pod `securityContext` and `fsGroup` for the
two persistent volumes.

On first start, the container writes the default configuration to
`/config/nzbget.conf`. Edit that persistent file to configure news servers and
change the default API password, then restart the container. Configuration is
file-based because the configuration-management API was deliberately removed.

The image includes CA certificates and 7-Zip but deliberately omits the
proprietary `unrar` binary. Extend the image or provide a compatible executable
separately if RAR extraction is required. A scratch image is not used because
TLS certificate verification, archive extraction, and first-run configuration
require a small set of runtime files.

## Build the image locally

Native installation is not a supported deployment method. To build the
container from this source tree:

```sh
docker build --build-arg VCS_REF="$(git rev-parse HEAD)" -t nzbget-lite .
```

Use `podman build` with the same arguments if preferred. The repository's CI
performs native compilation and tests solely to verify the source used by the
container; it does not publish native packages.

## API

The daemon exposes the existing JSON-RPC and XML-RPC endpoints on
`ControlPort` (6789 by default). Configure `ControlIP`, `ControlUsername`, and
`ControlPassword` in `nzbget.conf`, then use the API documented in
[docs/api/API.md](docs/api/API.md).

```sh
curl --user nzbget:password http://127.0.0.1:6789/jsonrpc/version
```

Do not expose the control port directly to the public internet. Use a firewall
and, when remote access is required, a secured reverse proxy or private
network.

## License

NZBGet Lite retains NZBGet's GNU General Public License, version 2 or (at your
option) any later version. Copyright remains with the authors identified in
the source files and Git history. See [COPYING](COPYING) and
[MODIFICATIONS.md](MODIFICATIONS.md). The software is provided without
warranty, to the extent permitted by law.

Component-specific copyright and licensing information is recorded in
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). The LGPL 2.1 text required
by the retained GNU regex fallback is included as [COPYING.LESSER](COPYING.LESSER).

The published container includes the license at
`/usr/share/licenses/nzbget-lite/COPYING`. Its OCI metadata links to the public
corresponding source and records the exact Git revision used to build it.
