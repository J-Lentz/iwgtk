CC=gcc
CFLAGS=`pkg-config --cflags gtk+-3.0`
LDLIBS=`pkg-config --libs gtk+-3.0`

prefix=/usr/local
exec_prefix=$(prefix)
bindir=$(exec_prefix)/bin
datarootdir=$(prefix)/share
datadir=$(datarootdir)
mandir=$(datarootdir)/man
man1dir=$(mandir)/man1
desktopdir=$(datadir)/applications
svg_icon_dir=$(datadir)/icons/hicolor/scalable

srcdir=src

files=sni main window indicator dialog adapter device station wps ap adhoc utilities switch known_network network hidden agent icons
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
	install -d $(DESTDIR)$(svg_icon_dir)/apps
	install icons/iwgtk.svg $(DESTDIR)$(svg_icon_dir)/apps
	install -d $(DESTDIR)$(svg_icon_dir)/actions
	install icons/unknown.svg $(DESTDIR)$(svg_icon_dir)/actions/com.github.j_lentz.iwgtk.station-down.svg
	install icons/connecting.svg $(DESTDIR)$(svg_icon_dir)/actions/com.github.j_lentz.iwgtk.station-connecting.svg
	install icons/connected.svg $(DESTDIR)$(svg_icon_dir)/actions/com.github.j_lentz.iwgtk.station-up.svg
	install icons/ap-down.svg $(DESTDIR)$(svg_icon_dir)/actions/com.github.j_lentz.iwgtk.ap-down.svg
	install icons/ap-up.svg $(DESTDIR)$(svg_icon_dir)/actions/com.github.j_lentz.iwgtk.ap-up.svg

uninstall :
	rm $(DESTDIR)$(bindir)/iwgtk
	rm $(DESTDIR)$(desktopdir)/iwgtk.desktop
	rm $(DESTDIR)$(man1dir)/iwgtk.1.gz
	rm $(DESTDIR)$(svg_icon_dir)/apps/iwgtk.svg
	rm $(DESTDIR)$(svg_icon_dir)/actions/com.github.j_lentz.iwgtk.station-down.svg
	rm $(DESTDIR)$(svg_icon_dir)/actions/com.github.j_lentz.iwgtk.station-connecting.svg
	rm $(DESTDIR)$(svg_icon_dir)/actions/com.github.j_lentz.iwgtk.station-up.svg
	rm $(DESTDIR)$(svg_icon_dir)/actions/icom.github.j_lentz.wgtk-ap-down.svg
	rm $(DESTDIR)$(svg_icon_dir)/actions/com.github.j_lentz.iwgtk.ap-up.svg

clean :
	rm -f iwgtk *.o $(srcdir)/icons.c $(srcdir)/icons.h iwgtk.1.gz
