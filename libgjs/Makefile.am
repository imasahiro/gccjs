## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libgjs.la

AM_CPPFLAGS = -I$(top_srcdir)/include
AM_YFLAGS = -d --verbose

library_includedir=$(includedir)/gjs
library_include_HEADERS = 
library_include_HEADERS += include/gjs/js.h
library_include_HEADERS += include/gjs/array.h

libgjs_la_CFLAGS = -DGJS -DSHARED
libgjs_la_LDFLAGS = -release 0.0.1 -lm
libgjs_la_SOURCES = runtime/js-runtime.c
