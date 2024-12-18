# set this to wherever you have clang command
# I am making it easier on myself to just use wasi-sdk's pre-configured clang
# but you should be able to use the standard version
CLANG:=/opt/wasi-sdk/bin/clang

# build a unit
docs/%.wasm: units/%.c
	${CLANG} -O3 --target=wasm32 --no-standard-libraries -Wl,--no-entry -o $@ $^
