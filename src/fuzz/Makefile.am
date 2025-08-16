noinst_PROGRAMS = wpdfuzzer

AM_CXXFLAGS = -I$(top_srcdir)/inc \
	$(REVENGE_GENERATORS_CFLAGS) \
	$(REVENGE_CFLAGS) \
	$(REVENGE_STREAM_CFLAGS) \
	$(DEBUG_CXXFLAGS)

wpdfuzzer_LDADD = \
	$(top_builddir)/src/lib/libwpd-@WPD_MAJOR_VERSION@.@WPD_MINOR_VERSION@.la \
	$(REVENGE_GENERATORS_LIBS) \
	$(REVENGE_LIBS) \
	$(REVENGE_STREAM_LIBS) \
	-lFuzzingEngine

wpdfuzzer_SOURCES = \
	wpdfuzzer.cpp
