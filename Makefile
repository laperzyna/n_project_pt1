OUT_FILE = client
C_FILES = client.c


#the "|| exit 1" is extra that is only because I have an extra second line that runs the program right after
#it is made. With "|| exit 1", the program does not try to run when the build

client:
	gcc -o $(OUT_FILE) $(C_FILES) || exit 1
	./$(OUT_FILE)
	