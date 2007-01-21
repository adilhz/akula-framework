INCLUDES = -I$(top_srcdir)/src $(all_includes)
METASOURCES = AUTO

reactor_includedir = $(includedir)/akula/reactor

reactor_include_HEADERS = \
reactor.h \
app_event_demultiplexer_array.h \
app_event_demultiplexer.h \
app_event_demultiplexer_map.h \
os_event_demultiplexer.h \
os_event_demultiplexer_poll.h \
os_event_demultiplexer_select.h \
reactor_type.h \
reactor_utils.h

noinst_LTLIBRARIES = libreactor.la

libreactor_la_SOURCES = \
os_event_demultiplexer_poll.cpp \
os_event_demultiplexer_select.cpp \
reactor_utils.cpp \
app_event_demultiplexer_array.h \
app_event_demultiplexer.h \
app_event_demultiplexer_map.h \
os_event_demultiplexer.h \
os_event_demultiplexer_poll.h \
os_event_demultiplexer_select.h \
reactor_type.h \
reactor.h \
reactor_utils.h
