build: main.o policies.o
	gcc main.o policies.o -o build

rebuild:
	make build
	
main.o: main.c
	gcc -c main.c

policies.o: policies.c policies.o
	gcc -c policies.c

runFIFO: build
	./build processes.dat FIFO

runSJF: build
	./build processes.dat SJF

runSTCF: build
	./build processes.dat STCF

runRR: build
	./build processes.dat RR

clean:
	rm *.o build
