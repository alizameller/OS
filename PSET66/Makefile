fifotest.exe: fifotest.o fifo.o cv.o spinlock.o tas.o
	gcc -std=gnu99 -o fifotest.exe fifotest.o fifo.o cv.o spinlock.o tas.o 

fifotest.o: fifotest.c fifo.h
	gcc -std=gnu99 -c fifotest.c -g

fifo.o: fifo.c fifo.h
	gcc -std=gnu99 -c fifo.c

cv.o: cv.c cv.h
	gcc -std=gnu99 -c cv.c

spintest.o:
	gcc -std=gnu99 spintest.c spinlock.c tas.S -o spintest.o

spinlock.o: spinlock.c spinlock.h
	gcc -std=gnu99 -c spinlock.c

tas.o: tas.S tas.h
	gcc -std=gnu99 -c tas.S

debug:
	gcc -std=gnu99 -g -o fifoDebug.exe fifotest.c fifo.c cv.c spinlock.c tas.S

clean:
	rm -f *.exe *.o *.stackdump *~

backup:
	test -d backups || mkdir backups
	cp *.cpp backups
	cp *.h backups
	cp Makefile backups