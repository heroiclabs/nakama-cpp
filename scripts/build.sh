#!/bin/sh

exec docker buildx build --platform linux/amd64 -f $(dirname $0)/Dockerfile --progress=tty $(dirname $0)/.. --output ${1:?pass output directory as first argument}
