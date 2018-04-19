CXX = ../../llvm/build/bin/clang++
CXXFLAGS ?= -std=c++17 -O3
BENCHMARK_FLAGS ?= -I/usr/local/include -L/usr/local/lib -lbenchmark

benchmark-comparable: benchmark-comparable.cc *.h
	$(CXX) $(CXXFLAGS) $(BENCHMARK_FLAGS) benchmark-comparable.cc -o $@
	./benchmark-comparable --benchmark_repetitions=15 --benchmark_report_aggregates_only=true

benchmark-relocatable: benchmark-relocatable.cc *.h
	$(CXX) $(CXXFLAGS) $(BENCHMARK_FLAGS) benchmark-relocatable.cc -o $@
	./benchmark-relocatable --benchmark_repetitions=15 --benchmark_report_aggregates_only=true

benchmark-tombstone: benchmark-tombstone.cc *.h
	$(CXX) $(CXXFLAGS) $(BENCHMARK_FLAGS) benchmark-tombstone.cc -o $@
	./benchmark-tombstone --benchmark_repetitions=15 --benchmark_report_aggregates_only=true

clean:
	rm -f benchmark-comparable benchmark-relocatable benchmark-tombstone

.PHONY: clean benchmark-comparable benchmark-relocatable benchmark-tombstone
