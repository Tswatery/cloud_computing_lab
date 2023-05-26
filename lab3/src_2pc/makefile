CXX= g++
CXXFLAGS= -std=c++17 -Wall -Wextra
SRCS= main.cpp Participant.cpp Coordinator.cpp MessageProcessor.cpp
OBJS=  $(SRCS:.cpp=.o)
TARGET= kvstore2pcsystem
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@
	# $<表示依赖 $@表示目标文件

clean:
	rm -f $(OBJS) $(TARGET)