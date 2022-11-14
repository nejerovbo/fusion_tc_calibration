# Clean build directory
rm -rf build
# Install conan repos to build
conan install . -if build
# Configure cmake to build from build
cmake -B build/
# Build with cmake
cmake --build build
