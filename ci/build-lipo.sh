#!/usr/bin/env bash
set -ue

# Builds Apple lipo tool on Linux so that we don't need to pay for expensive MacOS Github action runners
git_fetch() {
  git init cctools
  (
    cd cctools;
    git fetch --depth=1 https://github.com/tpoechtrager/cctools-port.git ${1:?pass commit id}:refs/heads/main
    git checkout main
  )
}

git_fetch 04663295d0425abfac90a42440a7ec02d7155fea
cd cctools/cctools
./configure
( cd libmacho; make; )
( cd libstuff; make; )
( cd misc; make lipo; )


