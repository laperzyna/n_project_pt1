
client: client.c 
	gcc -g -o client client.c
server: server.c 
	gcc -g -o server server.c

client2: client_v2.c 
	gcc -g -o client2 client_v2.c


clean:
	rm -rf $(PROGS)
	