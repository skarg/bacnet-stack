#Makefile to build test case
CC      = gcc
BASEDIR = .
#CFLAGS  = -Wall -I.
# -g for debugging with gdb
#CFLAGS  = -Wall -I. -g
CFLAGS  = -Wall -I. -Itest -DTEST -DTEST_ATOMIC_WRITE_FILE -DBACDL_BIP=1 -g

SRCS = bacdcode.c \
       bigend.c \
       awf.c \
       test/ctest.c

OBJS = ${SRCS:.c=.o}

TARGET = atomicwritefile

all: ${TARGET}
 
${TARGET}: ${OBJS}
	${CC} -o $@ ${OBJS} 

.c.o:
	${CC} -c ${CFLAGS} $*.c -o $@
	
depend:
	rm -f .depend
	${CC} -MM ${CFLAGS} *.c >> .depend
	
clean:
	rm -rf core ${TARGET} $(OBJS) *.bak *.1 *.ini

include: .depend
