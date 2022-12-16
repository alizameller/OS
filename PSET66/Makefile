fifotest:
	gcc fifotest.c fifo.c cv.c spinlock.c tas.S -g3 -o fifotest
spintest:
	gcc spintest.c spinlock.c tas.S -o spintest
clean:
	rm fifotest