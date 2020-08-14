CC=gcc
CFLAGS=`pkg-config --cflags gtk+-3.0` -O3
LDLIBS=`pkg-config --libs gtk+-3.0`

prefix=/usr/local
exec_prefix=$(prefix)
bindir=$(exec_prefix)/bin
datarootdir=$(prefix)/share
datadir=$(datarootdir)
mandir=$(datarootdir)/man
man1dir=$(mandir)/man1
desktopdir=$(datadir)/applications

srcdir=src
iconsdir=icons

files=main dialog objects adapter device station wps ap adhoc utilities switch known_network network hidden agent icons
icons=$(iconsdir)/*.svg

headers=$(patsubst %,$(srcdir)/%.h,$(files) iwgtk)
objects=$(patsubst %,%.o,$(files))

.PHONY : clean install uninstall

iwgtk : $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o : $(srcdir)/%.c $(headers)
	$(CC) -c $(CFLAGS) -o $@ $<

$(srcdir)/icons.c : icons.gresource.xml $(icons)
	glib-compile-resources --target=$@ --sourcedir=$(iconsdir) --generate-source $<

$(srcdir)/icons.h : icons.gresource.xml $(icons)
	glib-compile-resources --target=$@ --sourcedir=$(iconsdir) --generate-header $<

install : iwgtk
	install -d $(DESTDIR)$(bindir)
	install iwgtk $(DESTDIR)$(bindir)
	install -d $(DESTDIR)$(desktopdir)
	install iwgtk.desktop $(DESTDIR)$(desktopdir)
	install -d $(DESTDIR)$(man1dir)
	install iwgtk.1.gz $(DESTDIR)$(man1dir)

uninstall :
	rm $(DESTDIR)$(bindir)/iwgtk
	rm $(DESTDIR)$(desktopdir)/iwgtk.desktop
	rm $(DESTDIR)$(man1dir)/iwgtk.1.gz

clean :
	rm -f iwgtk *.o $(srcdir)/icons.c $(srcdir)/icons.h
