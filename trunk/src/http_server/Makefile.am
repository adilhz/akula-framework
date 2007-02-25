INCLUDES = -I$(top_srcdir)/src $(all_includes)
METASOURCES = AUTO

akulahttpdir = $(bindir)

akulahttp_PROGRAMS = akulahttp

EXTRA_DIST=akulahttp.config

akulahttp_SOURCES = \
main.cpp \
server.cpp \
configuration.cpp \
$(top_srcdir)/src/akula/dbg/dbg.cpp

akulahttp_LDADD = $(top_builddir)/src/http_server/http/libhttp.a

noinst_HEADERS = \
connection.h \
server.h \
configuration.h

SUBDIRS = http
