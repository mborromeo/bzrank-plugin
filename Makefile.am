lib_LTLIBRARIES = bzrank.la

bzrank_la_SOURCES = bzrank.cpp
bzrank_la_CXXFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
bzrank_la_LDFLAGS = -module -avoid-version -shared 
bzrank_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.txt \
	bzrank.def \
	bzrank.sln \
	bzrank.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
