compare: compare.o queueU.o queueB.o linkedlist.o filenode.o
	gcc -g -std=c99 -Wvla -Wall -fsanitize=address,undefined -o compare compare.o queueU.o queueB.o linkedlist.o filenode.o -lm

compare.o: compare.c queueU.h queueB.h linkedlist.h
	gcc -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined compare.c

queueU.o: queueU.c queueU.h
	gcc -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined queueU.c

queueB.o: queueB.c queueB.h
	gcc -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined queueB.c

filenode.o: filenode.c filenode.h
	gcc -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined filenode.c

linkedlist.o: linkedlist.c linkedlist.h
	gcc -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined linkedlist.c

clean:
	rm -f *.o $(OUTPUT)