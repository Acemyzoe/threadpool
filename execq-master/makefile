# g++ -pthread ./test.cpp -I include/ -L build/ -l execq


# CXX = g++
CXXFLAGS = -pthread  -I include/  -L build/  -l execq

# source = test

demo: $(source)

	g++ test.cpp $(CXXFLAGS) -o test

.PHONY: clean
clean:
	rm -f $(source)

help:
	@echo "variables:"
	@echo "  d=ANY   debug or not"