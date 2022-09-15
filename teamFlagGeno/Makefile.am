lib_LTLIBRARIES = teamFlagGeno.la

teamFlagGeno_la_SOURCES = teamFlagGeno.cpp
teamFlagGeno_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
teamFlagGeno_la_LDFLAGS = -module -avoid-version -shared
teamFlagGeno_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.teamFlagGeno.txt \
	teamFlagGeno.sln \
	teamFlagGeno.vcxproj	\
	teamFlagGeno.vcxproj.filters

MAINTAINERCLEANFILES =	\
	Makefile.in
