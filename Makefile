UNITS:=docs/copy.wasm docs/sampler.wasm docs/lpf.wasm

# set this to wherver you have clang command
# I am making it easier on myself to just use wasi-sdk's pre-configured clang
# but you should be able to use the standard version
CLANG:=/opt/wasi-sdk/bin/clang

.PHONY: all clean server

all: ${UNITS}

clean:
	rm -f ${UNITS}

server:
	npx -y live-server docs

docs/%.wasm: units/%.c
	${CLANG} -O3 --target=wasm32 --no-standard-libraries -Wl,--no-entry -o $@ $^
