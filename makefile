include Makefile

ifeq ($(ACLOCAL),)

all: Makefile
	$(MAKE) -f Makefile $(MAKECMDGOALS)

.FORCED: all

Makefile: configure
	./configure --enable-maintainer-mode

configure: Makefile.in configure.in config.h.in
	autoconf

Makefile.in: configure.in Makefile.am config.h.in
	automake --foreign --add-missing --copy

config.h.in: configure.in aclocal.m4
	autoheader

aclocal.m4: configure.in
	aclocal

endif
