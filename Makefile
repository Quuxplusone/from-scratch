check:
	./dependency-graph/generate-dependency-graph.py -I ./include > /dev/null

graph:
	./dependency-graph/generate-dependency-graph.py --dot -I ./include | dot -Tpdf > graph.pdf


.PHONY: check graph
