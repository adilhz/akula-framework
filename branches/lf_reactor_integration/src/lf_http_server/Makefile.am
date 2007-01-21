INCLUDES = -I$(top_srcdir)/src $(all_includes)
METASOURCES = AUTO

bin_PROGRAMS = akulahttp

akulahttp_SOURCES = \
main.cpp \
server.cpp \
configuration.cpp

akulahttp_LDADD = $(top_builddir)/src/akula/libakula.la

noinst_HEADERS = \
connection.h \
server.h \
configuration.h

