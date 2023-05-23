CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
SRCS = main.cpp MessageProcessor.cpp Node.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = kvstoreraftsystem
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
