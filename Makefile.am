## Process this file with automake to generate Makefile.in
#SUBDIRS = src tests

bin_PROGRAMS = microdef
microdef_SOURCES = md_gd.c md_gtk.c md_ps.c md_gfx.c mdkernel.c mdunxio.c microdef.h 
microdef_CPPFLAGS = @GTK_CFLAGS@ -DPOSIX -DGTKGUI -DPSGUI -D_NANONOTE_ -D_OMAKO_
microdef_LDADD = @GTK_LIBS@

CLEANFILES = *~
