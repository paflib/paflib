#!/bin/sh

# Generate configuration files not available in git repository

set -e
mkdir -p m4
autoreconf -f -i
