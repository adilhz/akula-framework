INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/unit_tests $(all_includes)
METASOURCES = AUTO

noinst_PROGRAMS = unit_tests

unit_tests_SOURCES = \
main.cpp \
test_dbg.cpp \
test_http_request.cpp \
test_string_utils.cpp \
test_thread.cpp

unit_tests_LDADD = $(top_builddir)/src/akula/libakula.la
