all: client server monitor


client: packet
	gcc -o client client.cpp packet.o

monitor:
	gcc -o monitor monitor.cpp

server: packet
	gcc -o server server.cpp packet.o

packet: 
	gcc -c -o packet.o packet.cpp

clean:
	rm client monitor server *.o