#!/usr/bin/env bash
# Simple helper script for building the project under MSYS2 / MinGW.
#
# Usage:
#   ./build.sh          – incremental C++ build (fastest)
#   ./build.sh -c       – full clean build (re-generates everything)
#   ./build.sh -s       – compile shaders only (does *not* touch C++)
#
# The script keeps CMake invocation logic in one place so developers don’t have
# to remember the exact flags.  It also avoids rebuilding 3rd-party submodules
# unless a clean build is requested.

set -euo pipefail

# ------------------------------ Argument parsing -----------------------------
CLEAN=false
SHADERS_ONLY=false

for arg in "$@"; do
    case "$arg" in
        -c|--clean)
            CLEAN=true
            ;;
        -s|--shaders)
            SHADERS_ONLY=true
            ;;
        *)
            echo "Unknown argument: $arg" >&2
            echo "Valid options are: -c/--clean  -s/--shaders" >&2
            exit 1
            ;;
    esac
done

# Prevent conflicting flags (clean implies full rebuild, not shader-only)
if $CLEAN && $SHADERS_ONLY; then
    echo "[build.sh] Warning: --clean and --shaders both specified – performing clean *and* full build." >&2
    SHADERS_ONLY=false
fi

# ------------------------------ Configuration -------------------------------
BUILD_DIR="build"
GENERATOR_FLAGS=(
    -DCMAKE_BUILD_TYPE=Debug
    -DCMAKE_TOOLCHAIN_FILE=../mingw-gcc-toolchain.cmake
    -DCMAKE_VERBOSE_MAKEFILE=ON
)

# Helper – run CMake configure in the build directory
configure() {
    local with_submodules=$1  # ON / OFF
    cmake "${GENERATOR_FLAGS[@]}" -DBUILD_SUBMODULES="${with_submodules}" ..
}

# Number of parallel jobs (fallback to 4)
JOBS=$( (command -v nproc >/dev/null 2>&1 && nproc) || echo 4 )

# ------------------------------ Clean build ---------------------------------
if $CLEAN; then
    echo "[build.sh] Performing clean build – removing ${BUILD_DIR}/"
    rm -rf "${BUILD_DIR}"
fi

# Ensure build directory exists & CMake configured
if [ ! -d "${BUILD_DIR}" ]; then
    echo "[build.sh] Creating ${BUILD_DIR}/ and running initial CMake configure (with submodules) …"
    mkdir -p "${BUILD_DIR}"
    pushd "${BUILD_DIR}" >/dev/null
    configure ON
    popd >/dev/null
else
    # Decide whether we can skip submodules. If essential third-party static
    # libraries are missing (e.g. first incremental build was interrupted), we
    # must rebuild them, otherwise CMake will fail to locate them.

    VKB_BOOTSTRAP_LIB="${BUILD_DIR}/src/renderer/3rdparty/vk-bootstrap/libvk-bootstrap.a"

    if [ -f "${VKB_BOOTSTRAP_LIB}" ]; then
        echo "[build.sh] Re-configuring (submodules OFF – libraries already present) …"
        pushd "${BUILD_DIR}" >/dev/null
        configure OFF
        popd >/dev/null
    else
        echo "[build.sh] Third-party libraries missing, enabling submodule build …"
        pushd "${BUILD_DIR}" >/dev/null
        configure ON
        popd >/dev/null
    fi
fi

# ------------------------------ Build phase ---------------------------------
if $SHADERS_ONLY; then
    echo "[build.sh] Building *shaders only* (target: compile_shaders) …"
    cmake --build "${BUILD_DIR}" --target compile_shaders -- -j"${JOBS}"
else
    echo "[build.sh] Building C++ targets …"
    cmake --build "${BUILD_DIR}" -- -j"${JOBS}" VERBOSE=1

    # On a clean build we also want the shaders compiled so that the runtime has
    # everything it needs right away.
    if $CLEAN; then
        echo "[build.sh] Clean build – compiling shaders as part of the full rebuild …"
        cmake --build "${BUILD_DIR}" --target compile_shaders -- -j"${JOBS}"
    fi
fi

echo "[build.sh] Build complete."