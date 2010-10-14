CC=gcc
#CC=/usr/nekoware/gcc-4.3/bin/gcc
#DEBUG=-O0 -Wall -pedantic -ansi -g #-DDEVEL_VERBOSE
DEBUG=-O3 -Wall -pedantic -ansi

# desktop system (Gtk+):
#CFLAGS=$(DEBUG) -DFEM_NEW_FILE_DLG -DPOSIX -DGTKGUI -DPSGUI `pkg-config --cflags gtk+-2.0`  -DGDGUI -I/usr/include/gd -I/usr/freeware/include/ #-I/sw/include
CFLAGS=$(DEBUG) -DPOSIX -DGTKGUI -DPSGUI `pkg-config --cflags gtk+-2.0`  -DGDGUI -I/usr/include/gd -I/usr/freeware/include/ #-I/sw/include
#CFLAGS=$(DEBUG) -D_OMAKO_ -DPOSIX -DGTKGUI -DPSGUI `pkg-config --cflags gtk+-2.0`  -DGDGUI -I/usr/include/gd #-I/usr/freeware/include/ #-I/sw/include
#CFLAGS=$(DEBUG) -DPOSIX -DGTKGUI -DPSGUI `pkg-config --cflags gtk+-2.0`  -DGDGUI -I/usr/include/gd #-I/usr/freeware/include/ #-I/sw/include
LIBS=`pkg-config --libs gtk+-2.0` -lm -L/usr/freeware/lib32 -lgd #-lefence

# old Hildon (IT2006, Maemo 2):
#CFLAGS=$(DEBUG) -DUSE_HILDON -DPOSIX -DGTKGUI -DPSGUI `pkg-config --cflags hildon-libs gtk+-2.0` # -DGDGUI -I/usr/include/gd #-I/usr/freeware/include/ #-I/sw/include
#LIBS=`pkg-config --libs hildon-libs gtk+-2.0`

# new Hildon (IT2008, Maemo 4):
#CFLAGS=$(DEBUG) -DUSE_HILDON -D_OMAKO_ -DPOSIX -DGTKGUI `pkg-config --cflags hildon-1 gtk+-2.0`
#LIBS=`pkg-config --libs hildon-1 gtk+-2.0` -lm #-L/usr/freeware/lib32 -lgd #-lefence

MMLIBS=-lm

OBJECTS=mdunxio.o mdkernel.o md_gfx.o md_gtk.o md_gd.o md_ps.o

all: microdef mmint

microdef: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) -o $(@)


mdunxio.o: mdunxio.c microdef.h
	$(CC) -c $(CFLAGS) mdunxio.c

mdkernel.o: mdkernel.c microdef.h
	$(CC) -c $(CFLAGS) mdkernel.c

md_gfx.o: md_gfx.c microdef.h
	$(CC) -c $(CFLAGS) md_gfx.c

md_gtk.o: md_gtk.c microdef.h
	$(CC) -c $(CFLAGS) md_gtk.c

md_gd.o: md_gd.c microdef.h
	$(CC) -c $(CFLAGS) md_gd.c

md_ps.o: md_ps.c microdef.h
	$(CC) -c $(CFLAGS) md_ps.c

mmint: mmint.c
	$(CC) -o $(@)  $(CFLAGS) mmint.c $(MMLIBS)


clean:
	rm -f *.o *.log *.dvi *.ps *-mm.out *.log *-report.aux *-report.txt *-report.html *-report.tex microdef core mmint
