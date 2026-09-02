FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential git python3 python3-pip pkg-config \
    && pip3 install --no-cache-dir --upgrade pip \
    && pip3 install --no-cache-dir conan cmake ninja \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . /src

RUN conan profile detect --force \
    && conan install . -u --build=missing -of=build \
    && cmake -S . -B build/cmake \
        -DCMAKE_TOOLCHAIN_FILE=build/generators/conan_toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DVIDRE_BUILD_APP=OFF \
    && cmake --build build/cmake --target vidre_camera_service vidre_storage_service -j"$(nproc)"

FROM ubuntu:22.04

RUN apt-get update && apt-get install -y --no-install-recommends libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /src/build/cmake/bin/vidre_camera_service /usr/local/bin/
COPY --from=builder /src/build/cmake/bin/vidre_storage_service /usr/local/bin/

CMD ["/usr/local/bin/vidre_camera_service", "--help"]
