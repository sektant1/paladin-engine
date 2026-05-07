#!/bin/bash

set -e # Exit early if any commands fail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Default build type is debug
BUILD_TYPE="debug"
VULKAN_PREVIEW=OFF

while [ $# -gt 0 ]; do
    case "$1" in
        --release|-r)
            BUILD_TYPE="release"
            shift
            ;;
        --vulkan|--vulkan-preview)
            VULKAN_PREVIEW=ON
            shift
            ;;
        *)
            break
            ;;
    esac
done
# Ensure compile steps are run within the repository directory

cd "$SCRIPT_DIR"

# Create and build in the appropriate directory
BUILD_DIR="bin/$BUILD_TYPE"
echo "Building in $BUILD_DIR..."

# Configure and build
cmake -B "$BUILD_DIR" -S . -G Ninja -DCMAKE_BUILD_TYPE="${BUILD_TYPE^}" -DMONAD_VULKAN_PREVIEW="$VULKAN_PREVIEW"

if [ "$VULKAN_PREVIEW" = "ON" ]; then
    if ! cmake --build "$BUILD_DIR" --target vulkan_preview -j"$(nproc)"; then
        echo "Error: Vulkan preview target was not built."
        echo "Install Vulkan headers/dev libraries, then rerun: ./compile.sh --vulkan"
        exit 1
    fi
    EXECUTABLE="$BUILD_DIR/vulkan_preview"
    if [ ! -f "$EXECUTABLE" ]; then
        echo "Error: Vulkan preview target was not built."
        echo "Install Vulkan headers/dev libraries, then rerun: ./compile.sh --vulkan"
        exit 1
    fi
    echo "Running: $EXECUTABLE $@"
    exec "$EXECUTABLE" "$@"
fi

cmake --build "$BUILD_DIR" -j"$(nproc)"

# Gets the binary name that cmake exports to
source "$BUILD_DIR/build_config.sh"

# Execute the binary (which should have the same name as the project)
EXECUTABLE="$BUILD_DIR/$PROJECT_EXECUTABLE_NAME"

if [ ! -f "$EXECUTABLE" ]; then
    echo "Error: The executable '$PROJECT_EXECUTABLE_NAME' was not found in $BUILD_DIR."
    echo "Check if the PROJECT_NAME at the script matches with the one at add_executable in CMakeLists.txt"
    exit 1
fi

echo "Running: $EXECUTABLE $@"
exec "$EXECUTABLE" "$@"
