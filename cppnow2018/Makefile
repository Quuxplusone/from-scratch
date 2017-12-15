CXXFLAGS ?= -std=c++1z -O2
BENCHMARK_FLAGS ?= -I/usr/local/include -L/usr/local/lib -lbenchmark

benchmark-relocatable: benchmark-relocatable.cc *.h
	$(CXX) $(CXXFLAGS) $(BENCHMARK_FLAGS) benchmark-relocatable.cc -o $@

benchmark-comparable: benchmark-comparable.cc *.h
	$(CXX) $(CXXFLAGS) $(BENCHMARK_FLAGS) benchmark-comparable.cc -o $@

benchmark-tombstone: benchmark-tombstone.cc *.h
	$(CXX) $(CXXFLAGS) $(BENCHMARK_FLAGS) benchmark-tombstone.cc -o $@ -DALSO_TEST_BOOST

run: benchmark-relocatable benchmark-comparable benchmark-tombstone
	./benchmark-relocatable
	./benchmark-comparable
	./benchmark-tombstone

clean:
	rm -f benchmark-relocatable benchmark-comparable benchmark-tombstone
