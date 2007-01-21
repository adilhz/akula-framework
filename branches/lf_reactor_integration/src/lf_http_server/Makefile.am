INCLUDES = -I$(top_srcdir)/src $(all_includes)
METASOURCES = AUTO

bin_PROGRAMS = akulahttp

akulahttp_SOURCES = \
connections_table.cpp \
connection.cpp \
lf_thread_pool.cpp \
main.cpp \
server.cpp \
event_handlers.cpp \
aim_connections_table.h \
connection.h \
lf_thread_pool.h \
server.h \
event_handlers.h \
configuration.h \
configuration.cpp

akulahttp_LDADD = $(top_builddir)/src/akula/libakula.la

noinst_HEADERS = \
connections_table.h \
connection.h \
lf_thread_pool.h \
server.h \
event_handlers.h\
configuration.h

