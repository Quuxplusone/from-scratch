CXXFLAGS ?= -std=c++1z -O2
SEED ?= 0
BENCHMARK_FLAGS ?= -I/usr/local/include -L/usr/local/lib -lbenchmark

local:
	./generate-harness.py --seed=$(SEED)
	$(CXX) $(CXXFLAGS) harness.gen.cc dynamicast.cc things.gen.cc -o fuzz
	echo 'Success!'

gcc:
	./generate-harness.py --seed=$(SEED)
	CXXFLAGS=-DFREE_USE_OF_CXX17 ../dependency-graph/unity-dump.py things.gen.cc dynamicast.cc harness.gen.cc --g++
	echo 'Success!'

clang:
	./generate-harness.py --seed=$(SEED)
	CXXFLAGS=-DFREE_USE_OF_CXX17 ../dependency-graph/unity-dump.py things.gen.cc dynamicast.cc harness.gen.cc --clang
	echo 'Success!'

benchmark:
	./generate-harness.py --seed=$(SEED) --benchmark
	$(CXX) $(CXXFLAGS) $(BENCHMARK_FLAGS) harness.gen.cc dynamicast.cc things.gen.cc -o bench
	./bench

clean:
	rm -f bench fuzz harness.gen.cc things.gen.cc things.gen.h
