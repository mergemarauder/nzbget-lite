#!/bin/sh
# Modified by NZBGet Lite contributors, 2026-08-28; see MODIFICATIONS.md.
set -eu

config=/config/nzbget.conf
if [ ! -e "$config" ]; then
    cp /usr/share/nzbget/nzbget.conf "$config"
    sed -i \
        -e 's|^MainDir=.*|MainDir=/downloads|' \
        -e 's|^ControlIP=.*|ControlIP=0.0.0.0|' \
        -e 's|^CertStore=.*|CertStore=/etc/ssl/certs/ca-certificates.crt|' \
        "$config"
fi

exec "$@"
