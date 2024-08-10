all: client server monitor


client: packet
	g++ -o client client.cpp packet.o -lm

monitor:
	g++ -o monitor monitor.cpp -lm

server: packet
	g++ -o server server.cpp packet.o -lm

packet: 
	g++ -c -o packet.o packet.cpp -lm

users:
	g++ -c -o users.o users.cpp -lm

test: users
	g++ -o test test.cpp users.o -lm

clean:
	rm client monitor server test *.o