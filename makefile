FLAGS=-g
CXX=g++
target=server\
	   client

all: $(target)


server: server.o
	$(CXX) $(FLAGS) -o $@ $^

client: client.o
	$(CXX) $(FLAGS) -o $@ $^

$.o:%.cpp
	$(CXX) $(FLAGS) -c $^ -o $@ 

clean:
	rm  $(target) *.o
