CLIENT_OBJS := client.o
TSCLIENT_OBJS := tsclient.o
SERVER_OBJS := server.o

CXXFLAGS += -std=c++11 -g -W -Wall -Weffc++ -Wextra -pedantic -O0

LDLIBS +=

.PHONY: all clean

all: client tsclient server

client: $(CLIENT_OBJS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

tsclient: $(TSCLIENT_OBJS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

server: $(SERVER_OBJS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -rf *.o client tsclient server
