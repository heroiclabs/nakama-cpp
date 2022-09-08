#!/bin/sh



exec docker build -f $(dirname $0)/Dockerfile --progress=plain $(dirname $0)/.. --output ${1:?pass output directory as first argument}
