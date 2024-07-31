CXX = g++
CXXFLAGS = -O0
TARGET = multiplexServer
SOURCES = main.cpp Server/SelectServer/select.cpp Server/PollServer/poll.cpp Server/EpollServer/epoll.cpp Server/utils/socket.cpp Server/utils/utils.cpp
OBJDIR = bin
OBJECTS = $(addprefix $(OBJDIR)/, $(SOURCES:.cpp=.o))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)/$(dir $<)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)
