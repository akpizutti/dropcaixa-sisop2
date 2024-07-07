all:
	gcc -o client client.c
	gcc -o monitor monitor.c
	gcc -o server server.c

clean:
	rm client monitor server