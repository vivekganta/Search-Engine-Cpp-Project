CXX     = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
TARGET  = search_engine
SRCS    = main.cpp engine.cpp

$(TARGET): $(SRCS) engine.h
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
