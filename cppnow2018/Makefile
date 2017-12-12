CXXFLAGS ?= -std=c++1z -O2
BENCHMARK_FLAGS ?= -I/usr/local/include -L/usr/local/lib -lbenchmark

benchmark-relocatable: benchmark-relocatable.cc *.h
	$(CXX) $(CXXFLAGS) $(BENCHMARK_FLAGS) benchmark-relocatable.cc -o $@

run: benchmark-relocatable
	./benchmark-relocatable

clean:
	rm -f benchmark-relocatable
