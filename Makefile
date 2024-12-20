WASI_SDK_PATH:=/opt/wasi-sdk
CLANG:=${WASI_SDK_PATH}/bin/clang

# build a unit
docs/units/%.wasm: units/%.c
	${CLANG} -Wl,--import-memory -O3 --sysroot=${WASI_SDK_PATH}/share/wasi-sysroot -o $@ $^
