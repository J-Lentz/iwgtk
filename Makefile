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
app_svg_icon_dir=$(datadir)/icons/hicolor/scalable/apps

srcdir=src

files=main dialog objects adapter device station wps ap adhoc utilities switch known_network network hidden agent icons
icons=icons/*.svg

headers=$(patsubst %,$(srcdir)/%.h,$(files) iwgtk)
objects=$(patsubst %,%.o,$(files))

.PHONY : clean install uninstall

iwgtk : $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o : $(srcdir)/%.c $(headers)
	$(CC) -c $(CFLAGS) -o $@ $<

$(srcdir)/icons.c : icons.gresource.xml $(icons)
	glib-compile-resources --target=$@ --sourcedir=icons --generate-source $<

$(srcdir)/icons.h : icons.gresource.xml $(icons)
	glib-compile-resources --target=$@ --sourcedir=icons --generate-header $<

iwgtk.1.gz : iwgtk.1
	gzip -k $<

install : iwgtk iwgtk.1.gz
	install -d $(DESTDIR)$(bindir)
	install iwgtk $(DESTDIR)$(bindir)
	install -d $(DESTDIR)$(desktopdir)
	install iwgtk.desktop $(DESTDIR)$(desktopdir)
	install -d $(DESTDIR)$(man1dir)
	install iwgtk.1.gz $(DESTDIR)$(man1dir)
	install -d $(DESTDIR)$(app_svg_icon_dir)
	install icons/iwgtk.svg $(DESTDIR)$(app_svg_icon_dir)

uninstall :
	rm $(DESTDIR)$(bindir)/iwgtk
	rm $(DESTDIR)$(desktopdir)/iwgtk.desktop
	rm $(DESTDIR)$(man1dir)/iwgtk.1.gz
	rm $(DESTDIR)$(app_svg_icon_dir)/iwgtk.svg

clean :
	rm -f iwgtk *.o $(srcdir)/icons.c $(srcdir)/icons.h iwgtk.1.gz
