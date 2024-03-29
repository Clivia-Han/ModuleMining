MSG = update
V = aes_core.hierarchy.v
TABLE = aes_core.hierarchy.v.table

clean:
	rm -fr build && rm -fr bin

debug-build: clean
	mkdir -p bin && mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug && make -j

debug: debug-build
	cd bin && gdb main

build: clean
	mkdir -p bin && mkdir build && cd build && cmake .. && make -j

test-run:
	cd bin && ./test_run

pre:
	cd bin && ./pre $(TABLE) $(V)

run:
	cd bin && ./main

git-push:
	git add . && git commit -m "$(MSG)" && git push origin main