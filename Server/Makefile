# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2019-20

SOURCES = Server.c fs.c sync.c
SOURCES+= lib/bst.c
OBJS_NOSYNC = $(SOURCES:%.c=%.o)
OBJS_MUTEX  = $(SOURCES:%.c=%-mutex.o)
OBJS_RWLOCK = $(SOURCES:%.c=%-rwlock.o)
OBJS = $(OBJS_NOSYNC) $(OBJS_MUTEX) $(OBJS_RWLOCK)
CC   = gcc #gcc/lldb
LD   = gcc #gcc/lldb
CFLAGS =-Wall -std=gnu99 -I../ -g # -g/-gdwarf-2
LDFLAGS=-lm -pthread
TARGETS = tecnicofs-rwlock

.PHONY: all clean

all: $(TARGETS)

$(TARGETS):
	$(LD) $(CFLAGS) $^ -o $@ $(LDFLAGS)


### NO SYNC ###
lib/bst.o: lib/bst.c lib/bst.h
fs.o: fs.c fs.h lib/bst.h
sync.o: sync.c sync.h constants.h
server.o: server.c fs.h lib/bst.h constants.h lib/timer.h sync.h
tecnicofs-nosync: lib/bst.o fs.o sync.o server.o

### MUTEX ###
lib/bst-mutex.o: CFLAGS+=-DMUTEX
lib/bst-mutex.o: lib/bst.c lib/bst.h

fs-mutex.o: CFLAGS+=-DMUTEX
fs-mutex.o: fs.c fs.h lib/bst.h

sync-mutex.o: CFLAGS+=-DMUTEX
sync-mutex.o: sync.c sync.h constants.h

server-mutex.o: CFLAGS+=-DMUTEX
server-mutex.o: server.c fs.h lib/bst.h constants.h lib/timer.h sync.h
tecnicofs-mutex: lib/bst-mutex.o fs-mutex.o sync-mutex.o server-mutex.o

### RWLOCK ###
lib/bst-rwlock.o: CFLAGS+=-DRWLOCK
lib/bst-rwlock.o: lib/bst.c lib/bst.h

fs-rwlock.o: CFLAGS+=-DRWLOCK
fs-rwlock.o: fs.c fs.h lib/bst.h

sync-rwlock.o: CFLAGS+=-DRWLOCK
sync-rwlock.o: sync.c sync.h constants.h

server-rwlock.o: CFLAGS+=-DRWLOCK
server-rwlock.o: Server.c fs.h lib/bst.h constants.h lib/timer.h sync.h
tecnicofs-rwlock: lib/bst-rwlock.o fs-rwlock.o sync-rwlock.o server-rwlock.o


%.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo Cleaning...
	rm -f $(OBJS) $(TARGETS)
