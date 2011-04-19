# Customize to fit your system

# paths
PREFIX = /usr/local
CONFPREFIX = ${PREFIX}/etc
MANPREFIX = ${PREFIX}/share/man

# includes and libs
LIBS = -L${PREFIX}/lib -L/usr/lib

# Linux/BSD
CFLAGS = -Wall -g -I. -I../include/ -I${PREFIX}/include -I/usr/include
#LDFLAGS = ${LIBS} ../libjackcpp.a `pkg-config --libs jack`
LDFLAGS = ${LIBS} ../libjackcpp.a -ljack -lpthread -lrt

AR = ar cr
CC = g++
RANLIB = ranlib
