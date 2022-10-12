OUT_FILEC = client
C_FILESC = client.c
OUT_FILES = server
C_FILES = server.c


#the "|| exit 1" is extra that is only because I have an extra second line that runs the program right after
#it is made. With "|| exit 1", the program does not try to run when the build

client:
	gcc -o $(OUT_FILEC) $(C_FILESC) || exit 1
	./$(OUT_FILEC)
	
server:
	gcc -o $(OUT_FILES) $(C_FILES) || exit 1
	./$(OUT_FILES)
	