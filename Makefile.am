# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.

AUTOMAKE_OPTIONS 	= subdir-objects
AM_CFLAGS 		= $(gtk_CFLAGS) $(glib_CFLAGS) $(cairo_CFLAGS) $(rsvg_CFLAGS) $(libusb_CFLAGS) -std=gnu99
bin_PROGRAMS 		= inklingreader
inklingreader_SOURCES 	= src/main.c src/gui/mainwindow.c src/gui/mainwindow.h \
			  src/converters/svg.c src/converters/svg.h \
			  src/converters/png.c src/converters/png.h \
			  src/converters/json.c src/converters/json.h \
			  src/converters/pdf.c src/converters/pdf.h \
			  src/converters/csv.c src/converters/csv.h \
			  src/parsers/wpi.c src/parsers/wpi.h \
			  src/high/conversion.c src/high/conversion.h \
			  src/datatypes/configuration.c src/datatypes/configuration.h \
			  src/optimizers/point-reduction.h src/optimizers/point-reduction.c \
			  src/usb/online-mode.h src/usb/online-mode.c \
			  src/datatypes/coordinate.h src/datatypes/clock.h \
			  src/datatypes/element.h src/datatypes/metadata.h \
			  src/datatypes/pressure.h src/datatypes/stroke.h src/datatypes/tilt.h \
              src/datatypes/inklingOSC.h src/osc_broadcaster/osc_broadcaster.c \
              src/libraries/tinyosc/tinyosc.c src/constants.h

inklingreader_LDADD 	= $(gtk_LIBS) $(glib_LIBS) $(cairo_LIBS) $(rsvg_LIBS) $(libusb_LIBS)

if OS_LINUX
inklingreader_LDADD    += -ldl -lpthread
else
inklingreader_LDADD    += -lshell32
inklingreader_LDFLAGS   = -mwindows
endif

###############################
#Code for compiling HIDAPI
ACLOCAL_AMFLAGS = -I src/libraries/hidapi/m4

SUBDIRS=

#if OS_LINUX
#SUBDIRS += linux libusb
#endif

if OS_DARWIN
SUBDIRS += src/libraries/hidapi/mac
AM_CFLAGS += -framework CoreFoundation -framework IOKit
inklingreader_SOURCES += src/libraries/hidapi/hidapi/hidapi.h src/libraries/hidapi/mac/hid.c
endif

EXTRA_DIST = udev doxygen

dist_doc_DATA = \
 src/libraries/hidapi/README.txt \
 src/libraries/hidapi/AUTHORS.txt \
 src/libraries/hidapi/LICENSE-bsd.txt \
 src/libraries/hidapi/LICENSE-gpl3.txt \
 src/libraries/hidapi/LICENSE-orig.txt \
 src/libraries/hidapi/LICENSE.txt

SCMCLEAN_TARGETS= \
 src/libraries/hidapi/aclocal.m4 \
 src/libraries/hidapi/config.guess \
 src/libraries/hidapi/config.sub \
 src/libraries/hidapi/configure \
 src/libraries/hidapi/config.h.in \
 src/libraries/hidapi/depcomp \
 src/libraries/hidapi/install-sh \
 src/libraries/hidapi/ltmain.sh \
 src/libraries/hidapi/missing \
 src/libraries/hidapi/mac/Makefile.in \
 src/libraries/hidapi/testgui/Makefile.in \
 src/libraries/hidapi/libusb/Makefile.in \
 src/libraries/hidapi/Makefile.in \
 src/libraries/hidapi/linux/Makefile.in \
 src/libraries/hidapi/windows/Makefile.in \
 src/libraries/hidapi/m4/libtool.m4 \
 src/libraries/hidapi/m4/lt~obsolete.m4 \
 src/libraries/hidapi/m4/ltoptions.m4 \
 src/libraries/hidapi/m4/ltsugar.m4 \
 src/libraries/hidapi/m4/ltversion.m4

SCMCLEAN_DIR_TARGETS = \
 autom4te.cache

scm-clean: distclean
	rm -f $(SCMCLEAN_TARGETS)
	rm -Rf $(SCMCLEAN_DIR_TARGETS)
###############################



if ENABLE_GCOV_OPTION
AM_CFLAGS	       += -fprofile-arcs -ftest-coverage
endif

docs:
	@cd doc/; texi2pdf inklingreader.texi 

docs-clean:
	@cd doc/; ${RM} -rf *.aux *.ky *.pg *.tp *.cp *.log *.vr *.fn *.toc

docs-doxygen:
	@cd doc/; doxygen Doxyfile

gcov-clean:
	@rm -rf src/*.gcno src/*.gcda src/*/*.gcno src/*/*.gcda

.PHONY: docs docs-clean docs-doxygen gcov-clean
