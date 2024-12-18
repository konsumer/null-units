WASI_SDK_PATH:=/opt/wasi-sdk
CLANG:=${WASI_SDK_PATH}/bin/clang

# build a unit
docs/%.wasm: units/%.c
	${CLANG} -O3 --sysroot=${WASI_SDK_PATH}/share/wasi-sysroot -Wl,--no-entry -o $@ $^
