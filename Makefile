# This file is part of Mongoose project, http://code.google.com/p/mongoose
# $Id: Makefile 473 2009-09-02 11:20:06Z valenok $

PROG=	vizsla

# Possible COPT values: (in brackets are rough numbers for 'gcc -O2' on i386)
# -DHAVE_MD5		- use system md5 library (-2kb)
# -DNDEBUG		- strip off all debug code (-5kb)
# -DDEBUG		- build debug version (very noisy) (+7kb)
# -DNO_CGI		- disable CGI support (-5kb)
# -DNO_SSL		- disable SSL functionality (-2kb)
# -DCONFIG_FILE=\"file\" - use `file' as the default config file
# -DHAVE_STRTOUI64	- use system strtoui64() function for strtoull()
# -DSSL_LIB=\"libssl.so.<version>\" - use system versioned SSL shared object
# -DCRYPTO_LIB=\"libcrypto.so.<version>\" - use system versioned CRYPTO so


##########################################################################
###                 UNIX build: linux, bsd, mac, rtems
##########################################################################

##CFLAGS = -W -Wall -O2 $(COPT)
CFLAGS = -W -O2 $(COPT)
MAC_SHARED = -flat_namespace -bundle -undefined suppress
LINFLAGS = -ldl -pthread $(CFLAGS)
CC = g++

# Make sure that the compiler flags come last in the compilation string.
# If not so, this can break some on some Linux distros which use
# "-Wl,--as-needed" turned on by default  in cc command.
# Also, this is turned in many other distros in static linkage builds.
all:
	$(CC) vizsla.c main.c -o $(PROG) $(LINFLAGS)


##########################################################################
###            Manuals, cleanup, test, release
##########################################################################

man:
	groff -man -T ascii vizsla.1 | col -b > vizsla.txt
	groff -man -T ascii vizsla.1 | less

# "TEST=unit make test" - perform unit test only
# "TEST=embedded" - test embedded API by building and testing test/embed.c
# "TEST=basic_tests" - perform basic tests only (no CGI, SSI..)
test: do_test
do_test:
	perl test/test.pl $(TEST)

release: clean
	F=mongoose-`perl -lne '/define\s+MONGOOSE_VERSION\s+"(\S+)"/ and print $$1' mongoose.c`.tgz ; cd .. && tar --exclude \*.hg --exclude \*.svn --exclude \*.swp --exclude \*.nfs\* -czf x mongoose && mv x mongoose/$$F

clean:
	rm -rf *.o *.core $(PROG) *.obj *.so $(PROG).txt *.dSYM *.tgz
