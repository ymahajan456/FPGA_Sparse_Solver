CXX = g++
CXXFLAGS = -std=c++11 -O3

MATLAB = ~/Softwares/Matlab/R2017a/bin/matlab
PYTHON3 = python3


# CXXFLAGS = -std=c++0x -g

.PHONY: clean cleanRes cleanall cleanPreprocess nerdStats all run

all: main

run:
	./main

# File dependency tree
CCS.o: CCS.h stdafx.h CCS.cpp
	$(CXX) $(CXXFLAGS) -c CCS.cpp

Graph.o: Graph.h stdafx.h Graph.cpp
	$(CXX) $(CXXFLAGS) -c Graph.cpp

sparseLU.o: stdafx.h CCS.h Graph.h sparseLU.h sparseLU.cpp
	$(CXX) $(CXXFLAGS) -c sparseLU.cpp

Hardware.o : stdafx.h Hardware.h Hardware.cpp
	$(CXX) $(CXXFLAGS) -c Hardware.cpp

Schedule.o: stdafx.h Graph.h Schedule.h Schedule.cpp Hardware.h
	$(CXX) $(CXXFLAGS) -c Schedule.cpp

# Hungarian.o: Hungarian.h Hungarian.cpp
# 	$(CXX) $(CXXFLAGS) -c Hungarian.cpp

main.o: stdafx.h CCS.h Graph.h sparseLU.h Schedule.h Hardware.h main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

main: CCS.o Graph.o sparseLU.o Schedule.o Hardware.o main.o
	$(CXX) $(CXXFLAGS) -o $@ $^

mainMultipleTests.o: stdafx.h CCS.h Graph.h sparseLU.h Schedule.h Hardware.h mainMultipleTests.cpp
	$(CXX) $(CXXFLAGS) -c mainMultipleTests.cpp

mainMultipleTests: CCS.o Graph.o sparseLU.o Schedule.o Hardware.o mainMultipleTests.o
	$(CXX) $(CXXFLAGS) -o $@ $^
	./$@

clean: 
	rm -f main main.o sparseLU.o Graph.o CCS.o

cleanRes:
	rm -f *.dot *.png *.txt *.csv
	rm -rf ../outputFiles

buildPreprocessTests: processTests.o CCS.o
	$(CXX) $(CXXFLAGS) -o preprocessTest $^

preprocessTests:
	$(MATLAB) -nodisplay -nosplash -nodesktop -r "run('../matlabScripts/processMsparseMatrices.m'); exit;"
	$(PYTHON3) ../pyFiles/preprocess.py

processTests.o: stdafx.h CCS.h processTests.cpp
	$(CXX) $(CXXFLAGS) -c processTests.cpp

cleanPreprocess:
	rm -f preprocessTest processTests.o

cleanall:
	rm -f *.o *.txt *.csv *.png *.dot main preprocessTest mainMultipleTests

# test: stdafx.h
# 	$(CXX) $(CXXFLAGS) -c test.cpp 
# 	$(CXX) $(CXXFLAGS) -o test test.o
# 	./test
# 	rm -f test test.o

nerdStats: 
	$(PYTHON3) ../pyFiles/nerdStats.py
