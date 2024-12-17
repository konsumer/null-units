UNITS:=docs/copy.wasm docs/sampler.wasm docs/lpf.wasm

.PHONY: all clean server

all: ${UNITS}

clean:
	rm -f ${UNITS}

server:
	npx -y live-server docs

docs/%.wasm: units/%.c
	clang -O3 --target=wasm32 --no-standard-libraries -Wl,--no-entry -o $@ $^
