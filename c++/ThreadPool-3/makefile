
CXX = g++
CXXFLAGS = -std=c++11 -Wall -pthread
HEADERS = ThreadPool.h

ifdef d
	CXXFLAGS += -DDEBUG
endif

source = example1 example2 example3 example4 example5

demo: $(source)

$(for var in $(source);do\
	$(var): $(var) $(HEADERS);\
	$(CXX) -o $@ $(var).cpp $(CXXFLAGS);\
done)

.PHONY: clean
clean:
	rm -f $(source)

help:
	@echo "variables:"
	@echo "  d=ANY   debug or not"

