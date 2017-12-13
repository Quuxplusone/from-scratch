CXXFLAGS ?= -std=c++1z -O2
BENCHMARK_FLAGS ?= -I/usr/local/include -L/usr/local/lib -lbenchmark

benchmark-relocatable: benchmark-relocatable.cc *.h
	$(CXX) $(CXXFLAGS) $(BENCHMARK_FLAGS) benchmark-relocatable.cc -o $@

benchmark-comparable: benchmark-comparable.cc *.h
	$(CXX) $(CXXFLAGS) $(BENCHMARK_FLAGS) benchmark-comparable.cc -o $@

run: benchmark-relocatable benchmark-comparable
	./benchmark-relocatable
	./benchmark-comparable

clean:
	rm -f benchmark-relocatable benchmark-comparable
