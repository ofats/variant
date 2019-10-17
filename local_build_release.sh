cd `dirname ${BASH_SOURCE[0]}`

BUILD_DIR=build/Release
CC=gcc
CXX=g++
CXX_FLAGS="-O3 -Wall -Wextra -Werror -g"
LINKER_FLAGS="-fuse-ld=gold"

mkdir -p ${BUILD_DIR} && cd ${BUILD_DIR}
cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=14 -DCMAKE_CXX_FLAGS="${CXX_FLAGS}" -DCMAKE_EXE_LINKER_FLAGS="${LINKER_FLAGS}"
cmake --build . -j
