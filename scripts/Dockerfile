# syntax=docker/dockerfile:1.4
ARG UBUNTU_BASE=ubuntu:18.04
FROM $UBUNTU_BASE

RUN rm -f /etc/apt/apt.conf.d/docker-clean; echo 'Binary::apt::APT::Keep-Downloaded-Packages "true";' > /etc/apt/apt.conf.d/keep-cache
RUN --mount=type=cache,target=/var/cache/apt --mount=type=cache,target=/var/lib/apt \
  apt update && apt-get install --no-install-recommends -y libc-dev curl zip unzip ca-certificates busybox git-core pkg-config software-properties-common

RUN --mount=type=cache,target=/var/cache/apt --mount=type=cache,target=/var/lib/apt <<EOT bash -ue
 mkdir -p /opt/cmake /opt/ninja
 curl -L https://github.com/Kitware/CMake/releases/download/v3.23.2/cmake-3.23.2-linux-x86_64.tar.gz \
 | tar -C /opt/cmake --strip-components=1 -xzf -
 ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
 ( cd /usr/local/bin; curl -o /tmp/ninja.zip -L  https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-linux.zip; unzip /tmp/ninja.zip ; )
 add-apt-repository --yes ppa:ubuntu-toolchain-r/test
 apt install -y g++-11 python3-distutils
EOT

ARG CMAKE_ARGS=""

ENV CC=/usr/bin/gcc-11 CXX=/usr/bin/g++-11

RUN --mount=type=bind,target=/src,rw --mount=type=cache,target=/tmp/build <<EOT bash -ue
  export VCPKG_DOWNLOADS=/tmp/build/vcpkg/downloads
  export VCPKG_FORCE_SYSTEM_BINARIES=1
  export VCPKG_DEFAULT_BINARY_CACHE=/tmp/build/vcpkg/archives
  mkdir -p \${VCPKG_DOWNLOADS}
  mkdir -p \${VCPKG_DEFAULT_BINARY_CACHE}
  cd /src
  rm -rf /tmp/build/src
  cmake --preset linux-amd64 -B /tmp/build/cmake-build ${CMAKE_ARGS}
  cmake --build /tmp/build/cmake-build --config MinSizeRel --target nakama-sdk
  cmake --install /tmp/build/cmake-build --prefix /export/
EOT

FROM scratch AS export
COPY --from=0 /export .
