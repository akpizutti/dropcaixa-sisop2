all: client server monitor users


client: packet users utils
	g++ -o client client.cpp packet.o users.o utils.o -lm

monitor:
	g++ -o monitor monitor.cpp -lm

server: packet users utils
	g++ -o server server.cpp packet.o users.o utils.o -lm

packet: utils
	g++ -c -o packet.o packet.cpp utils.o -lm

users: 
	g++ -c -o users.o users.cpp -lm

utils:
	g++ -c -o utils.o utils.cpp -lm

test: users utils
	g++ -o test test.cpp users.o utils.o -lm

clean:
	rm client monitor server test *.o