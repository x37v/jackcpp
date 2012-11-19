include config.mk

VERSION = 0.3.1
SRCDIR = ./src
DISTDIR = jackcpp.${VERSION}
SUBDIRS = ${SRCDIR} include test doc
INCLUDE_FILES = `ls include/*`
LIBNAME = "libjackcpp.a"
GITHUBNAME = 'jackcpp'
CFLAGS += "-fPIC"

SRC = ${SRCDIR}/jackaudioio.cpp \
		${SRCDIR}/jackmidiport.cpp \
		${SRCDIR}/jackblockingaudioio.cpp

OBJ = ${SRC:.cpp=.o}

.cpp.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} -o $*.o $<

${LIBNAME}: ${OBJ}
	@echo AR $@
	@${AR} $@ ${OBJ}
	@${RANLIB} $@

.PHONY: test doc

doc:
	@cd doc && doxygen Doxyfile

ruby: ${LIBNAME} clean-swig
	@echo "building ruby interface via swig"
	@cd swig/ && ruby ./extconf.rb && make

lua: ${LIBNAME} clean-swig
	@echo "building lua interface via swig"
	@cd swig/ && make -f Makefile-lua

python: ${LIBNAME} clean-swig
	@echo "building python interface via swig"
	@cd swig/ && python setup.py build_ext --inplace

test: ${LIBNAME}
	@cd test && make all

dist: clean doc
	mkdir -p ${DISTDIR}
	mkdir -p ${DISTDIR}/swig
	cp -R COPYING Makefile NEWS config.mk ${SUBDIRS} ${DISTDIR}
	cp swig/* ${DISTDIR}/swig
	echo "jackcpp version ${VERSION}" > ${DISTDIR}/README
	cat README >> ${DISTDIR}/README
	tar -czf ${DISTDIR}.tar.gz --exclude=".svn" ${DISTDIR}
	rm -rf ${DISTDIR}

doc-post: doc
	mkdir tmp/ && cd tmp/ && \
		git clone git@github.com:x37v/${GITHUBNAME}.git && \
		cd ${GITHUBNAME} && git co gh-pages
	cp -r doc/html/* tmp/${GITHUBNAME}
	cd tmp/${GITHUBNAME} && git add . && \
		git ci -a -m 'auto updated docs' && \
		git push origin gh-pages
	rm -rf tmp

#
#dist-post: dist doc-post
#	scp ${DISTDIR}.tar.gz alex@x37v.info:x37v.info/jack_cpp/code

install: ${LIBNAME}
	mkdir -p ${DESTDIR}${PREFIX}/lib/
	mkdir -p ${DESTDIR}${PREFIX}/include/
	cp ${LIBNAME} ${DESTDIR}${PREFIX}/lib/
	cp include/* ${DESTDIR}${PREFIX}/include/

uninstall:
	rm -f ${DESTDIR}${PREFIX}/lib/${LIBNAME}
	for i in ${INCLUDE_FILES}; do \
		rm -f ${DESTDIR}${PREFIX}/$$i; \
	done

clean-swig:
	cd swig && rm -rf build *.so *.cxx Makefile jackaudio.py *.pyc

clean: clean-swig
	rm -f ${SRCDIR}/*.o *.a *.gz 
	rm -rf doc/html doc/latex tmp
	cd test && make clean

all: ${LIBNAME} test
