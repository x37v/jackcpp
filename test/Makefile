include config.mk

SRC = \
	testjack.cpp \
	testjackblocking.cpp \
	testjackringbuffer.cpp

TARGETS = ${SRC:.cpp=}

all: ${TARGETS}

%: %.cpp ../libjackcpp.a
	@echo CC $<
	@${CC} ${CFLAGS} -o $@ $@.cpp ${LDFLAGS}

clean:
	@rm -f *.o ${TARGETS}
