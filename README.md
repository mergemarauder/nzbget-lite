# NZBGet Lite

NZBGet Lite is a Linux-only, API-only fork of
[NZBGet](https://github.com/nzbgetcom/nzbget). It keeps the downloader,
JSON-RPC/XML-RPC APIs, command-line client, and native Linux
daemon while removing the bundled web UI, static-file server, Docker image,
non-Linux ports, and platform packaging infrastructure.

There is intentionally no browser interface. Non-RPC HTTP routes return
`404 Not Found`.

## Requirements

- Linux
- CMake 3.13 or newer
- A C++20 compiler
- libxml2, OpenSSL, ncurses, zlib, and Boost development packages
- Git and network access during the first build for pinned `par2-turbo` and
  `rapidyenc` dependencies

On Ubuntu or Debian:

```sh
sudo apt-get update
sudo apt-get install -y build-essential cmake git libboost-all-dev \
  libncurses-dev libssl-dev libxml2-dev zlib1g-dev
```

## Build and test

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=ON
cmake --build build -j4
ctest --test-dir build --output-on-failure
```

Install with `sudo cmake --install build`.

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

NZBGet Lite retains NZBGet's GNU General Public License v2 terms. See
[COPYING](COPYING).
