cd `dirname ${BASH_SOURCE[0]}`
[ ! -d "./build/Release" ] && echo "Error: build project first using ./build_release.sh" && exit
./build/Release/bench/evaluator/evaluator_bench
