INCLUDES = $(all_includes)
METASOURCES = AUTO

libakula_ladir=$(libdir)


#        EXAMPLE_RELEASE=2:1:3
#        EXAMPLE_LIBRARY_VERSION=3:0:0
#                             | | |
#                      +------+ | +---+
#                      |        |     |
#                    current:revision:age
#                      |        |     |
#                      |        |     +- increment if interfaces have been added
#                      |        |        set to zero if interfaces have been removed
#                      |        |        or changed
#                      |        +- increment if source code has changed
#                      |           set to zero if current is incremented
#                      +- increment if interfaces have been added, removed or changed
AKULA_RELEASE=0:0:0
AKULA_LIBRARY_VERSION=0:0:0
libakula_la_LDFLAGS= -version-number $(AKULA_LIBRARY_VERSION) -no-undefined


libakula_la_LTLIBRARIES = libakula.la

libakula_la_SOURCES = dbg http net reactor utils

#Link all our sub-libraries and libpthread into one
#We expect to have -lpthread set into $(LIBS); done during configuration, see configure.in
libakula_la_LIBADD = \
    $(top_builddir)/src/akula/dbg/libdbg.la \
    $(top_builddir)/src/akula/net/libnet.la \
    $(top_builddir)/src/akula/http/libhttp.la \
    $(top_builddir)/src/akula/reactor/libreactor.la \
    $(LIBS)

SUBDIRS = dbg utils http net reactor
