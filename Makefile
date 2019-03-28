all: buNeDu

buNeDu: main.o
	gcc main.o -o buNeDu

131044019_main.o: main.c
	gcc -c main.c

clean:
	rm *.o buNeDu
