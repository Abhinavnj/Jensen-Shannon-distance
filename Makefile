compare: compare.o queue.o linkedlist.o
	gcc -g -std=c99 -Wvla -Wall -fsanitize=address,undefined -o compare compare.o queue.o linkedlist.o

compare.o: compare.c queue.h linkedlist.h
	gcc -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined compare.c

queue.o: queue.c queue.h
	gcc -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined queue.c

linkedlist.o: linkedlist.c linkedlist.h
	gcc -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined linkedlist.c

clean:
	rm -f *.o $(OUTPUT)