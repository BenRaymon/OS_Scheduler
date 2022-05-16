all: os
os: objs.o queues.o main.c reqs.h
	gcc -o os main.c objs.o queues.o


objs.o: objs.c objs.h
	gcc -c objs.c

queues.o: queues.c queues.h
	gcc -c queues.c

clean:
	rm *.o os