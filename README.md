# NZBGet Lite

NZBGet Lite is a Linux-only, API-only fork of
[NZBGet](https://github.com/nzbgetcom/nzbget). It keeps the downloader,
JSON-RPC/XML-RPC APIs, command-line client, and native Linux
daemon while removing the bundled web UI, extension runtime, static-file
server, non-Linux ports, and platform packaging infrastructure.

This is a substantially modified, unofficial fork and is not endorsed by the
upstream NZBGet project. See [MODIFICATIONS.md](MODIFICATIONS.md) for the base
revision, modification date, and a summary of the changes.

There is intentionally no browser interface. Non-RPC HTTP routes return
`404 Not Found`.

## Requirements

- Linux
- CMake 3.13 or newer
- A C++20 compiler
- libxml2, OpenSSL, zlib, and Boost development packages
- Git and network access during the first build for pinned `par2-turbo` and
  `rapidyenc` dependencies

On Ubuntu or Debian:

```sh
sudo apt-get update
sudo apt-get install -y build-essential cmake git libboost-all-dev \
  libssl-dev libxml2-dev zlib1g-dev
```

## Build and test

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=ON
cmake --build build -j4
ctest --test-dir build --output-on-failure
```

Install with `sudo cmake --install build`.

## Container

The published image is `ghcr.io/mergemarauder/nzbget-lite:latest`. It supports
64-bit x86 and 64-bit ARM hosts, including common NAS and Raspberry Pi systems.

```sh
docker run -d --name nzbget-lite \
  -p 6789:6789 \
  -v nzbget-config:/config \
  -v nzbget-downloads:/downloads \
  ghcr.io/mergemarauder/nzbget-lite:latest
```

The same image works with Podman and Kubernetes. The process runs as numeric
UID `65532` in group `0`, does not require Linux capabilities, and only writes
to `/config` and `/downloads`. Kubernetes may replace the UID; ensure the two
mounted volumes are writable by the selected security context or `fsGroup`.
The image includes CA certificates and 7-Zip, but deliberately omits the
proprietary `unrar` binary; mount or extend the image with one if required.

On first start the container copies the default configuration into `/config`.
Change its default API password before exposing the service. A scratch image
is not used because TLS certificate verification, archive extraction, and a
writable first-run configuration require runtime files beyond the executable.

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
