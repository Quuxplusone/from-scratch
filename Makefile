check:
	./dependency-graph/generate-dependency-graph.py -I ./include --root include > /dev/null

graph:
	./dependency-graph/generate-dependency-graph.py --dot -I ./include --root include | dot -Tpdf > graph.pdf


.PHONY: check graph
