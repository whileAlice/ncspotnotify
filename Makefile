# ncspot socket listener + notifier

APPNAME = ncspotnotify
SRCDIR  = src
BINDIR  = bin
BIN = ${BINDIR}/${APPNAME}
SRC = ${wildcard ${SRCDIR}/*.c}
OBJ = ${SRC:.c=.o}

CC = clang
# TODO: verify if any of this is redundant
CFLAGS = -Wall -Wextra -Wconversion -Wdouble-promotion \
         -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion \
         -fsanitize=undefined -fsanitize-trap -std=c23 \
         -D_POSIX_C_SOURCE=200809L

${BIN}: ${OBJ}
	${CC} -o ${BIN} ${OBJ}

run: ${BIN}
	${BIN} --debug

runopt: CFLAGS += -O3
runopt: run

clean:
	rm -f ${BIN} ${OBJ}

fresh: clean ${BIN}

.PHONY: clean fresh run runopt
