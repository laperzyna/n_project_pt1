PROG = client server
all : $(PROGS)

client: client.c 
	gcc -g -o client client.c
server: server.c 
	gcc -g -o server server.c

client2: client_v2.c 
	gcc -g -o client2 client_v2.c

client3: client3.c 
	gcc -g -o client3 client3.c
server3: server3.c 
	gcc -g -o server3 server3.c


clean:
	rm -rf $(PROGS)
	
