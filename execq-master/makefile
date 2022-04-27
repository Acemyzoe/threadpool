# g++ -pthread ./test.cpp -I include/ -L build/ -l execq

CXX = g++
CXXFLAGS = -pthread  -I include/  -L build/  -l execq

source = test

demo: $(source)
	$(CXX) test.cpp $(CXXFLAGS) -o $(source)

.PHONY: clean
clean:
	rm -f $(source)

help:
	@echo "variables:"