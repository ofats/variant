cd `dirname ${BASH_SOURCE[0]}`
[ ! -d "./build/Debug" ] && echo "Error: build project first using ./build_debug.sh first" && exit
cd build/Debug
ctest $*
