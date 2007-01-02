INCLUDES = -I$(top_srcdir)/src $(all_includes)
METASOURCES = AUTO


utils_includedir = $(includedir)/akula/utils

utils_include_HEADERS = \
utils.h \
condition_object.h \
queue.h \
string_utils.h \
timeval.h \
guard.h \
singleton.h \
thread.h \
thread_mutex.h \
thread_safe_queue.h





noinst_LTLIBRARIES = libutils.la

libutils_la_SOURCES = condition_object.cpp condition_object.h queue.h \
	string_utils.h timeval.h timeval.cpp guard.h singleton.h thread.h thread_mutex.cpp \
	thread_mutex.h thread_safe_queue.h
