COMPILER = g++-11



all: client server users


client: packet users utils
	$(COMPILER) -o Client/client Client/client.cpp Common/packet.o Common/users.o Common/utils.o -lm

server: packet users utils
	$(COMPILER) -o Server/server Server/server.cpp Common/packet.o Common/users.o Common/utils.o -lm

packet: utils
	$(COMPILER) -c -o Common/packet.o Common/packet.cpp Common/utils.o -lm

users: 
	$(COMPILER) -c -o Common/users.o Common/users.cpp -lm

utils:
	$(COMPILER) -c -o Common/utils.o Common/utils.cpp -lm

monitor:
	$(COMPILER) -o Misc/monitor Misc/monitor.cpp -lm

test: users utils
	$(COMPILER) -o Misc/test Misc/test.cpp Common/users.o Common/utils.o -lm

clean:
	rm -f Common/*.o Client/*.o Server/*.o Misc/*.o
	rm -f Client/client Monitor/monitor Server/server Misc/test 
	