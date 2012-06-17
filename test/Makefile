include config.mk

SRC = \
	testjack.cpp \
	testjackmidi.cpp \
	testjackblocking.cpp \
	testjackringbuffer.cpp

TARGETS = ${SRC:.cpp=}

all: ${TARGETS}

../libjackcpp.a:
	@cd .. && make

%: %.cpp ../libjackcpp.a
	@echo CC $<
	@${CC} ${CFLAGS} -o $@ $@.cpp ${LDFLAGS}

clean:
	@rm -f *.o ${TARGETS}
