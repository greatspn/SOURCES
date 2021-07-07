#!/bin/bash

# contributed by millergarym

ROOT="$(cd "$( dirname "${BASH_SOURCE[0]}" )/../.." && pwd )"

docker build -t gspn $ROOT -f $ROOT/docker/fedora/fedora.dockerfile
