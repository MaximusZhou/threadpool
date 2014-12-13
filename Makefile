CFLAGS = -D_REENTRANT -Wall -pedantic -Isrc
LDLIBS = -lpthread

ifdef DEBUG
CFLAGS += -g
LDFLAGS += -g
endif

TARGETS = test/test_tp_main

all: $(TARGETS)

test/test_tp_main: test/test_tp_main.o src/threadpool.o src/listed_blocking_queue.o
test/test_tp_main.o: test/test_tp_main.c src/threadpool.h
src/threadpool.o: src/threadpool.c src/threadpool.h src/listed_blocking_queue.h
src/listed_blocking_queue.o: src/listed_blocking_queue.c src/listed_blocking_queue.h

clean:
	rm -f $(TARGETS) */*.o
