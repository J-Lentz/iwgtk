CC?=gcc
CCINCS=`pkg-config --cflags gtk+-3.0`
LDLIBS=`pkg-config --libs gtk+-3.0`

prefix=$(PREFIX)
prefix?=/usr/local
exec_prefix=$(prefix)
bindir=$(exec_prefix)/bin
datarootdir=$(prefix)/share
datadir=$(datarootdir)
mandir=$(datarootdir)/man
man1dir=$(mandir)/man1
desktopdir=$(datadir)/applications
svg_icon_dir=$(datadir)/icons/hicolor/scalable

srcdir=src

files=sni main window indicator dialog adapter device station wps diagnostic ap adhoc utilities switch known_network network hidden agent
icons=icons/*.svg

headers=$(patsubst %,$(srcdir)/%.h,$(files) iwgtk)
objects=$(patsubst %,%.o,$(files))

.PHONY : clean install uninstall

iwgtk : $(objects)
	$(CC) $(CCINCS) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o : $(srcdir)/%.c $(headers)
	$(CC) -c $(CCINCS) $(CFLAGS) -o $@ $<

iwgtk.1.gz : iwgtk.1
	gzip -k $<

install : iwgtk iwgtk.1.gz
	install -d $(DESTDIR)$(bindir)
	install iwgtk $(DESTDIR)$(bindir)
	install -d $(DESTDIR)$(desktopdir)
	install iwgtk.desktop $(DESTDIR)$(desktopdir)
	install -d $(DESTDIR)$(man1dir)
	install iwgtk.1.gz $(DESTDIR)$(man1dir)
	install -d $(DESTDIR)$(svg_icon_dir)/apps
	install icons/iwgtk.svg $(DESTDIR)$(svg_icon_dir)/apps

uninstall :
	rm $(DESTDIR)$(bindir)/iwgtk
	rm $(DESTDIR)$(desktopdir)/iwgtk.desktop
	rm $(DESTDIR)$(man1dir)/iwgtk.1.gz
	rm $(DESTDIR)$(svg_icon_dir)/apps/iwgtk.svg

clean :
	rm -f iwgtk *.o iwgtk.1.gz
