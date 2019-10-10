cd `dirname ${BASH_SOURCE[0]}`
mkdir -p build/Debug
cd build/Debug
cmake ../.. -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=14 -DCMAKE_CXX_FLAGS="-g -fsanitize=address,leak,undefined -fno-sanitize-recover=undefined -Wall -Wextra -Werror"
make -j
