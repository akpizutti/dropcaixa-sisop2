all:
	gcc -o client client.cpp
	gcc -o monitor monitor.cpp
	gcc -o server server.cpp

clean:
	rm client monitor server