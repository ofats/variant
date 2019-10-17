cd `dirname ${BASH_SOURCE[0]}`

BUILD_DIR=build/Debug
CC=gcc
CXX=g++
CXX_FLAGS="-g -fsanitize=address,leak,undefined -fno-sanitize-recover=undefined -Wall -Wextra -Werror"
LINKER_FLAGS="-lasan -lubsan -fuse-ld=gold"

mkdir -p ${BUILD_DIR} && cd ${BUILD_DIR}
cmake ../.. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=14 -DCMAKE_CXX_FLAGS="${CXX_FLAGS}" -DCMAKE_EXE_LINKER_FLAGS="${LINKER_FLAGS}" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build . -j
