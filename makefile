all: client server monitor


client: packet
	gcc -o client client.cpp packet.o -lm

monitor:
	gcc -o monitor monitor.cpp -lm

server: packet
	gcc -o server server.cpp packet.o -lm

packet: 
	gcc -c -o packet.o packet.cpp -lm

clean:
	rm client monitor server *.o