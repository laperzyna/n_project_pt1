PROG = client server
all : $(PROGS)

client: client.c JsonParse.h
	gcc -g -o client client.c
server: server.c JsonParse.h
	gcc -g -o server server.c


clean:
	rm -rf $(PROGS)
	
