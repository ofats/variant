cd `dirname ${BASH_SOURCE[0]}`
[ ! -d "./build/Release" ] && echo "Error: build project first using ./build_release.sh first" && exit
cd build/Release
ctest $*
