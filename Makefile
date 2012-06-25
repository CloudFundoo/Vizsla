# This file is taken from Mongoose project, http://code.google.com/p/mongoose

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
###                 UNIX build: linux
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
	$(CC) vizsla_http.c vizsla.c -o $(PROG) $(LINFLAGS)
	$(MAKE) -C ./polarssl


clean:
	rm -rf *.o vizsla
	$(MAKE) -C ./polarssl clean
