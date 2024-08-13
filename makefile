all: client server monitor users


client: packet users utils
	g++ -o client client.cpp packet.o users.o utils.o -lm

monitor:
	g++ -o monitor monitor.cpp -lm

server: packet users utils
	g++ -o server server.cpp packet.o users.o utils.o -lm

packet: 
	g++ -c -o packet.o packet.cpp -lm

users: 
	g++ -c -o users.o users.cpp -lm

utils:
	g++ -c -o utils.o utils.cpp -lm

test: users
	g++ -o test test.cpp users.o -lm

clean:
	rm client monitor server test *.o