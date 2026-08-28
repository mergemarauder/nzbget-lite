# syntax=docker/dockerfile:1.7
# SPDX-License-Identifier: GPL-2.0-or-later
# Added by NZBGet Lite contributors, 2026-08-28.
FROM alpine:3.22 AS build

RUN apk add --no-cache \
    boost-dev cmake g++ git libxml2-dev make openssl-dev zlib-dev

WORKDIR /src
COPY . .
RUN cmake -S . -B build \
      -DCMAKE_BUILD_TYPE=Release \
      -DENABLE_TESTS=OFF && \
    cmake --build build --parallel 2 && \
    strip build/nzbget

FROM alpine:3.22

ARG VCS_REF="unknown"
LABEL org.opencontainers.image.source="https://github.com/mergemarauder/nzbget-lite" \
      org.opencontainers.image.url="https://github.com/mergemarauder/nzbget-lite" \
      org.opencontainers.image.description="Minimal Linux API-only NZBGet" \
      org.opencontainers.image.licenses="GPL-2.0-or-later" \
      org.opencontainers.image.revision="${VCS_REF}"

RUN apk add --no-cache \
      7zip boost1.84-json ca-certificates libgcc libstdc++ libxml2 openssl zlib && \
    mkdir -p /config /downloads && \
    chgrp -R 0 /config /downloads && \
    chmod -R g=u /config /downloads

COPY --from=build /src/build/nzbget /usr/local/bin/nzbget
COPY --from=build /src/build/nzbget.conf /usr/share/nzbget/nzbget.conf
COPY COPYING /usr/share/licenses/nzbget-lite/COPYING
COPY COPYING.LESSER /usr/share/licenses/nzbget-lite/COPYING.LESSER
COPY THIRD_PARTY_NOTICES.md /usr/share/licenses/nzbget-lite/THIRD_PARTY_NOTICES.md
COPY --chmod=0555 docker/entrypoint.sh /usr/local/bin/container-entrypoint

ENV HOME=/config
USER 65532:0
WORKDIR /config
EXPOSE 6789
ENTRYPOINT ["container-entrypoint"]
CMD ["nzbget", "-s", "-c", "/config/nzbget.conf"]
